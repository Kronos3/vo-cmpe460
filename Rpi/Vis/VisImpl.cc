#include <Fw/Logger/Logger.hpp>
#include "VisImpl.h"

namespace Rpi
{

    VisImpl::VisImpl(const char* compName)
    : VisComponentBase(compName),
      m_listener(VisListener::NONE),
      m_recording(nullptr),
      m_pipeline(nullptr),
      m_pipeline_last(nullptr)
    {
    }

    void VisImpl::init(NATIVE_INT_TYPE queue_depth, NATIVE_INT_TYPE instance)
    {
        VisComponentBase::init(queue_depth, instance);
    }

    void VisImpl::frame_handler(NATIVE_INT_TYPE portNum, CamFrame* frame)
    {
        // If we don't have a listener we need to drop the frame
        // It is up to us to return the frame to the camera
        if (m_listener == VisListener::NONE)
        {
            frameDeallocate_out(0, frame);
            ready_out(0, 0);
            return;
        }

        // If we don't have a pipeline, Vis is just a passthrough
        if (!m_pipeline)
        {
            frameOut_out(m_listener.e, frame);

            // Tell the frame pipe we are ready for another frame
            ready_out(0, 0);
            return;
        }

        // Build an OpenCV Matrix with the userland memory mapped pointer
        // This memory is MemMapped to the DMA buffer
        cv::Mat image((I32)frame->info.height,
                      (I32)frame->info.width,
                      CV_8U,
                      frame->span.data(),
                      frame->info.stride);

        // We need to extract this atomically because recording requests
        // may be cancelled asynchronously by commanding
        m_state_mutex.lock();
        VisPipelineStage* pipe_start = m_pipeline;
        VisRecord* recording = m_recording;
        m_state_mutex.unLock();

        // Pass the frame through the transformation pipeline
        pipe_start->run(image, recording);

        if (recording)
        {
            recording->increment();
            if (recording->is_finished())
            {
                Fw::LogStringArg arg = recording->get_filename();
                log_ACTIVITY_LO_VisRecordFinished(arg);

                Os::File::Status status = recording->write();
                if (status != Os::File::OP_OK)
                {
                    log_WARNING_LO_VisRecordWriteFailed(arg, status);
                }
                else
                {
                    log_ACTIVITY_LO_VisRecordWritten(arg);
                }

                delete recording;
                recording = nullptr;
            }
        }

        m_state_mutex.lock();
        if (!m_recording && recording)
        {
            // The recording was cancelled asynchronously by commanding
            // We can just clear the request
            delete recording;
        }
        m_state_mutex.unLock();

        // The image should now have our pipeline changes written to it
        // Send it to the listener
        frameOut_out(m_listener.e, frame);

        // Tell the frame pipe we are ready for another frame
        ready_out(0, 0);
    }

    void VisImpl::STREAM_cmdHandler(U32 opCode, U32 cmdSeq, VisListener listener)
    {
        if (m_listener != VisListener::NONE)
        {
            log_ACTIVITY_LO_VisStopping(m_listener);
        }

        if (listener != VisListener::NONE)
        {
            log_ACTIVITY_LO_VisStreaming(listener);
        }

        m_listener = listener;
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void VisImpl::CLEAR_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        delete m_pipeline;
        m_pipeline = nullptr;
        m_pipeline_last = nullptr;
        log_ACTIVITY_LO_VisCleared();
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void VisImpl::PIPE_cmdHandler(U32 opCode, U32 cmdSeq, Rpi::VisPipe stage)
    {
        VisPipelineStage* stage_ptr = nullptr;
        Fw::ParamValid valid;

        switch(stage.e)
        {
            case VisPipe::FIND_CHESSBOARD:
            {
                I32 col = paramGet_POSE_CALIB_COL(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                I32 row = paramGet_POSE_CALIB_ROW(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                cv::Size calib_size(row, col);
                stage_ptr = new VisFindChessBoard(calib_size);
            }
                break;
            case VisPipe::POSE_CALCULATION:
            {
                I32 col = paramGet_POSE_CALIB_COL(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                I32 row = paramGet_POSE_CALIB_ROW(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                cv::Size calib_size(row, col);
                stage_ptr = new VisPoseCalculation(calib_size);
            }
                break;
            default:
                FW_ASSERT(0 && "Invalid stage", stage.e);
        }

        FW_ASSERT(stage_ptr);

        m_state_mutex.lock();
        if (!m_pipeline)
        {
            // This is the first item
            // We can add it directly to the member
            FW_ASSERT(!m_pipeline_last);
            m_pipeline = stage_ptr;
        }
        else
        {
            FW_ASSERT(m_pipeline_last);
            m_pipeline_last->chain(stage_ptr);
        }

        log_ACTIVITY_LO_VisPipe(stage);
        m_pipeline_last = stage_ptr;

        m_state_mutex.unLock();
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void VisImpl::RECORD_cmdHandler(U32 opCode, U32 cmdSeq, U32 frame_count, const Fw::CmdStringArg &filename)
    {
        m_state_mutex.lock();
        if (m_recording)
        {
            log_WARNING_LO_VisAlreadyRecording();
            m_state_mutex.unLock();
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
            return;
        }

        Fw::LogStringArg fn_arg = filename;
        log_ACTIVITY_LO_VisRecord(fn_arg);
        m_recording = new VisRecord(frame_count, filename);
        m_state_mutex.unLock();

        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void VisImpl::RECORD_TIMEOUT_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        m_state_mutex.lock();
        bool already_recording = m_recording;
        if (already_recording)
        {
            log_WARNING_LO_VisAlreadyRecording();
        }
        m_state_mutex.unLock();
        cmdResponse_out(opCode, cmdSeq, already_recording ? Fw::COMMAND_EXECUTION_ERROR : Fw::COMMAND_OK);
    }
}
