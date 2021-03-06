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
            frameOut_out(listener.e, frame);

            // Tell the frame pipe we are ready for another frame
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
        bool pipe_status;
        cv::Mat& output = pipe_start->run(frame, image, recording.get(), pipe_status);
        if (!pipe_status)
        {
            tlm_frame_failed++;
            tlmWrite_FramesFailed(tlm_frame_failed);

            // Nobody should receive a failed
            frame->decref();

            ready_out(0, 0);
            return;
        }

        cv::resize(output, image,
                   cv::Size(image.cols, image.rows),
                   0, 0, cv::INTER_NEAREST);

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
            case VisPipe::WARP_CALCULATION:
            {
                I32 col = paramGet_POSE_CALIB_COL(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                I32 row = paramGet_POSE_CALIB_ROW(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                cv::Size calib_size(row, col);
                stage_ptr = new VisWarpCalculation(calib_size);
            }
                break;
            case VisPipe::DOWNSCALE:
            {
                I32 downscale = paramGet_DOWNSCALE(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                VisInterpolation interpolation = paramGet_INTERPOLATION(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                stage_ptr = new VisDownscale(
                        downscale,
                        static_cast<cv::InterpolationFlags>(
                                interpolation.e)
                                );
            }
                break;
            case VisPipe::GAUSSIAN:
            {
                F32 sigma = paramGet_GAUSSIAN_SIGMA(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                stage_ptr = new VisGaussian(sigma);
            }
                break;
            case VisPipe::GRADIENT:
                stage_ptr = new VisGradiant();
                break;
            case VisPipe::ERODE:
                stage_ptr = new VisErode();
                break;
            case VisPipe::OTSU_THRESHOLD:
            {
                U32 otsu_frames = paramGet_OTSU_FRAMES(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                stage_ptr = new VisOtsuThreshold(otsu_frames);
            }
                break;
            case VisPipe::WARP:
            {
                Fw::ParamString warp_calibration = paramGet_WARP_CALIBRATION(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                F32 scale_x = paramGet_WARP_SCALE_X(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                F32 scale_y = paramGet_WARP_SCALE_Y(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                F32 translate_x = paramGet_WARP_TRANSLATE_X(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                F32 translate_y = paramGet_WARP_TRANSLATE_Y(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);

                auto* warp = new VisWarp(scale_x, scale_y, translate_x, translate_y);
                Fw::LogStringArg a = warp_calibration;
                if (!warp->read(warp_calibration.toChar()))
                {
                    log_WARNING_HI_VisCalibrationFailed(a);
                    delete warp;
                    stage_ptr = nullptr;
                }
                else
                {
                    log_ACTIVITY_LO_VisCalibrationLoaded(a);
                    stage_ptr = warp;
                }
                break;
            }
            case VisPipe::MASK_QUAD:
                stage_ptr = nullptr;
                break;
            default:
                FW_ASSERT(0 && "Invalid stage", stage.e);
        }

        if (!stage_ptr)
        {
            log_WARNING_HI_VisPipelineFailed(stage);
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
            return;
        }

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
                break;
            case WARP_RECORD:
                m_recording = std::make_shared<WarpRecord>(frame_count);
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

    void VisImpl::MASK_QUAD_cmdHandler(U32 opCode, U32 cmdSeq,
                                       U8 color,
                                       F32 c1x, F32 c1y,
                                       F32 c2x, F32 c2y,
                                       F32 c3x, F32 c3y,
                                       F32 c4x, F32 c4y)
    {
        auto* stage_ptr = new VisMaskQuad(
                cv::Point2f(c1x, c1y),
                cv::Point2f(c2x, c2y),
                cv::Point2f(c3x, c3y),
                cv::Point2f(c4x, c4y),
                color
                );

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

        log_ACTIVITY_LO_VisPipe(VisPipe::MASK_QUAD);
        m_pipeline_last = stage_ptr;

        m_state_mutex.unLock();
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }
}
