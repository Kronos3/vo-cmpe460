#include "NavImpl.h"

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

        cv::Mat image((I32)frame->info.height,
                      (I32)frame->info.width,
                      CV_8U,
                      frame->span.data(),
                      frame->info.stride);

        bool keep_going = m_algorithm->process(frame, image);

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
                m_params.simple.turning = paramGet_SIMPLE_CENTER(valid);
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
