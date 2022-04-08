#include "NavImpl.h"
#include "opencv2/imgproc.hpp"

namespace Rpi
{
    NavImpl::NavImpl(const char* compName)
    : NavComponentBase(compName),
    m_type(NavAlgorithmType::NONE),
    m_algorithm(nullptr)
    {
    }

    void NavImpl::steer(F32 steering)
    {
        steer_out(0, steering);
    }

    void NavImpl::throttle(F32 throttle_left, F32 throttle_right)
    {
        throttle_out(0, throttle_left, throttle_right);
    }

    void NavImpl::init(NATIVE_INT_TYPE queue_depth, NATIVE_INT_TYPE instance)
    {
        NavComponentBase::init(queue_depth, instance);
    }

    void NavImpl::frame_handler(NATIVE_INT_TYPE portNum, CamFrame* frame)
    {
        FW_ASSERT(frame);
        FW_ASSERT(frame->in_use());

        if (!m_algorithm)
        {
            // Nothing to run
            // Just pass the frame through
            frameOut_out(0, frame);
            return;
        }

        // This image was upscale during vision to maintain
        // the same image dimensions
        cv::Mat full_image(
                (I32)frame->info.height,
                (I32)frame->info.width,
                CV_8U,
                frame->span.data(),
                frame->info.stride);

        // Nav doesn't care about the dimensions of the image,
        // we can operate on the Vision pipeline's size
//        cv::resize(full_image, m_image,
//                   cv::Size(full_image.cols / VIS_RACE_DOWNSCALE,
//                            full_image.rows / VIS_RACE_DOWNSCALE),
//                   0, 0, cv::INTER_NEAREST // same as Vis
//        );

        bool keep_going = m_algorithm->process(frame, full_image);

        if (!keep_going)
        {
            clear();
        }

        frameOut_out(0, frame);
    }

    void NavImpl::RUN_cmdHandler(U32 opCode, U32 cmdSeq, NavAlgorithmType algorithm)
    {
        // Check if there is an algorithm already running
        // If so cancel it and start a new one
        if (m_type != NavAlgorithmType::NONE)
        {
            FW_ASSERT(m_algorithm != nullptr);
            delete m_algorithm;
            m_algorithm = nullptr;

            // Warn which algorithm we are cancelling
            log_WARNING_LO_NavCanceling(m_type, algorithm);
            m_type = NavAlgorithmType::NONE;
        }
        else
        {
            FW_ASSERT(m_algorithm == nullptr, (POINTER_CAST)m_algorithm);
            // Nothing to cancel
        }

        FW_ASSERT(!m_algorithm);
        switch(algorithm.e)
        {
            case NavAlgorithmType::NONE:
                // Don't start anything
                m_algorithm = nullptr;
                break;
            case NavAlgorithmType::SIMPLE:
                m_algorithm = new NavSimple(this);
                break;
            case NavAlgorithmType::SIMPLE_PID:
            case NavAlgorithmType::COMPLEX_PID:
                // TODO(tumbar) Not implemented
                m_algorithm = nullptr;
                break;
            default:
                FW_ASSERT(0 && "Invalid Nav algorithm", algorithm.e);
        }

        if (m_algorithm)
        {
            m_type = algorithm;
            log_ACTIVITY_HI_NavStarting(m_type);
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
        }
        else if (algorithm == NavAlgorithmType::NONE)
        {
            m_type = NavAlgorithmType::NONE;
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
        }
        else
        {
            log_WARNING_HI_NavStartFailed(algorithm);
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
        }
    }

    void NavImpl::STOP_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        if (m_algorithm)
        {
            clear();
        }
        else
        {
            log_WARNING_LO_NavNoAlgorithmRunning();
        }

        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    const NavParams& NavImpl::params() const
    {
        return m_params;
    }

    void NavImpl::parameterUpdated(FwPrmIdType id)
    {
        NavComponentBase::parameterUpdated(id);

        Fw::ParamValid valid;
        switch(id)
        {
            case PARAMID_SIMPLE_CENTER:
                m_params.simple.center = paramGet_SIMPLE_CENTER(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_SIMPLE_P:
                m_params.simple.turning = paramGet_SIMPLE_P(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_SIMPLE_ROW:
                m_params.simple.row = paramGet_SIMPLE_ROW(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_SIMPLE_THROTTLE:
                m_params.simple.throttle = paramGet_SIMPLE_THROTTLE(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_SIMPLE_CUTOFF:
                m_params.simple.cutoff = paramGet_SIMPLE_CUTOFF(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            default:
                FW_ASSERT(0, id);
        }
    }

    void NavImpl::parametersLoaded()
    {
        NavComponentBase::parametersLoaded();

        parameterUpdated(PARAMID_SIMPLE_CENTER);
        parameterUpdated(PARAMID_SIMPLE_P);
        parameterUpdated(PARAMID_SIMPLE_ROW);
        parameterUpdated(PARAMID_SIMPLE_THROTTLE);
        parameterUpdated(PARAMID_SIMPLE_CUTOFF);
    }

    void NavImpl::clear()
    {
        log_ACTIVITY_HI_NavStopping(m_type);

        throttle(0.0, 0.0);
        steer(0.0);

        delete m_algorithm;
        m_algorithm = nullptr;
        m_type = NavAlgorithmType::NONE;
    }
}
