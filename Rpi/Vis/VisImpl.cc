#include <Fw/Logger/Logger.hpp>
#include "VisImpl.h"

namespace Rpi
{

    VisImpl::VisImpl(const char* compName)
    : VisComponentBase(compName),
      m_listener(VisListener::NONE),
      m_is_file_recording(false),
      cmd_waiting(false),
      waiting_opCode(0),
      waiting_cmdSeq(0),
      m_recording(nullptr),
      m_pipeline(nullptr),
      m_pipeline_last(nullptr),
      tlm_frame_in(0),
      tlm_frame_failed(0),
      tlm_frame_processed(0)
    {
    }

    void VisImpl::init(NATIVE_INT_TYPE queue_depth, NATIVE_INT_TYPE instance)
    {
        VisComponentBase::init(queue_depth, instance);
    }

    void VisImpl::frame_handler(NATIVE_INT_TYPE portNum, CamFrame* frame)
    {
        FW_ASSERT(frame->in_use());

        // We need to extract this atomically because recording requests
        // may be cancelled asynchronously by commanding
        m_state_mutex.lock();
        VisListener listener = m_listener;
        Fw::String target_file = m_recording_file;
        bool recording_is_file = m_is_file_recording;
        auto pipe_start = m_pipeline;           // hold a reference while we are using it
        auto recording = m_recording;
        m_state_mutex.unLock();

        // If we don't have a listener we need to drop the frame
        // It is up to us to return the frame to the camera
        if (listener == VisListener::NONE)
        {
            frame->decref();
            ready_out(0, 0);
            return;
        }

        // If we don't have a pipeline, Vis is just a passthrough
        if (!pipe_start)
        {
            // Tell the frame pipe we are ready for another frame
            frame->decref();
            ready_out(0, 0);
            return;
        }

        tlm_frame_in++;
        tlmWrite_FramesIn(tlm_frame_in);

        // Build an OpenCV Matrix with the userland memory mapped pointer
        // This memory is MemMapped to the DMA buffer
        cv::Mat image((I32)frame->info.height,
                      (I32)frame->info.width,
                      CV_8U,
                      frame->span.data(),
                      frame->info.stride);

        // Pass the frame through the transformation pipeline
        bool pipe_status = pipe_start->run(frame, image, recording.get());
        if (!pipe_status)
        {
            tlm_frame_failed++;
            tlmWrite_FramesFailed(tlm_frame_failed);

            // Nobody should receive a failed
            frame->decref();

            ready_out(0, 0);
            return;
        }

        m_state_mutex.lock();
        VisRecord* recording_new = m_recording.get();

        if (recording_new == recording.get())
        {
            // The request has not been cancelled
            // It is still running
            if (recording.get() && recording->done())
            {
                if (recording_is_file)
                {
                    bool write_good = recording->write(target_file.toChar());

                    Fw::LogStringArg arg = target_file;
                    if (write_good)
                    {
                        log_ACTIVITY_HI_VisRecordWritten(arg);
                    }
                    else
                    {
                        log_WARNING_HI_VisRecordWriteFailed(arg);
                    }
                }

                clear_recording();
            }
        }

        m_state_mutex.unLock();

        // The image should now have our pipeline changes written to it
        // Send it to the listener
        frameOut_out(listener.e, frame);

        tlm_frame_processed++;
        tlmWrite_FramesPassed(tlm_frame_processed);

        // Tell the frame pipe we are ready for another frame
        ready_out(0, 0);
    }

    void VisImpl::STREAM_cmdHandler(U32 opCode, U32 cmdSeq, VisListener listener)
    {
        m_state_mutex.lock();
        if (m_listener != VisListener::NONE)
        {
            log_ACTIVITY_LO_VisStopping(m_listener);
        }

        if (listener != VisListener::NONE)
        {
            log_ACTIVITY_LO_VisStreaming(listener);
        }

        m_listener = listener;
        m_state_mutex.unLock();
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void VisImpl::CLEAR_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        m_state_mutex.lock();
        m_pipeline = nullptr;
        m_pipeline_last = nullptr;

        clear_recording();

        m_state_mutex.unLock();
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
            case VisPipe::GRADIENT:
            {
                stage_ptr = new VisGradiant();
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
            m_pipeline = std::shared_ptr<VisPipelineStage>(stage_ptr);
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

    void VisImpl::RECORD_FILE_cmdHandler(U32 opCode, U32 cmdSeq,
                                         VisRecordType record_type,
                                         U32 frame_count,
                                         const Fw::CmdStringArg &filename)
    {
        m_state_mutex.lock();
        if (m_recording)
        {
            log_WARNING_LO_VisAlreadyRecording();
            m_state_mutex.unLock();
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
            return;
        }

        switch(record_type)
        {
            case CALIBRATION_RECORD:
                m_recording = std::make_shared<CalibrationRecord>(frame_count);
                Fw::Logger::logMsg("frame_count: %d\n", frame_count);
                break;
            default:
            case VisRecordType_MAX:
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
        }

        m_recording_file = filename;
        m_is_file_recording = true;
        m_state_mutex.unLock();

        Fw::LogStringArg fn_arg = m_recording_file;
        log_ACTIVITY_LO_VisRecord(fn_arg);

        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void VisImpl::RECORD_WAIT_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        m_state_mutex.lock();
        if (m_recording)
        {
            cmd_waiting = true;
            waiting_opCode = opCode;
            waiting_cmdSeq = cmdSeq;
        }
        else
        {
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
        }
        m_state_mutex.unLock();
    }

    void VisImpl::clear_recording()
    {
        if (cmd_waiting)
        {
            cmdResponse_out(waiting_opCode, waiting_cmdSeq, Fw::COMMAND_OK);
            cmd_waiting = false;
            waiting_opCode = 0;
            waiting_cmdSeq = 0;
        }

        m_recording = nullptr;
        m_is_file_recording = false;
        m_recording_file = "";
    }
}
