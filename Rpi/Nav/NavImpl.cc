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
                m_algorithm = new NavSimplePid(this);
                break;
            case NavAlgorithmType::COMPLEX_PID:
                m_algorithm = new NavComplexPid(this);
                break;
            case NavAlgorithmType::DIAG_CONTOUR:
                m_algorithm = new NavDiagContour(this);
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
            case PARAMID_SCAN_ROW:
                m_params.scan.row = paramGet_SCAN_ROW(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_SCAN_STEP:
                m_params.scan.step = paramGet_SCAN_STEP(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_SCAN_DISCONTINUITY_THRESH_1:
                m_params.scan.discontinuity_thresh_1 = paramGet_SCAN_DISCONTINUITY_THRESH_1(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_SCAN_DISCONTINUITY_THRESH_2:
                m_params.scan.discontinuity_thresh_2 = paramGet_SCAN_DISCONTINUITY_THRESH_2(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
//            case PARAMID_SIMPLE_CUTOFF:
//                m_params.simple.cutoff = paramGet_SIMPLE_CUTOFF(valid);
//                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
//                break;
            case PARAMID_PID_P:
                m_params.pid.p = paramGet_PID_P(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_PID_I:
                m_params.pid.i = paramGet_PID_I(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_PID_D:
                m_params.pid.d = paramGet_PID_D(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_PID_T:
                m_params.pid.t = paramGet_PID_T(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_PID_STEERING_TIME:
                m_params.pid.steering_t = paramGet_PID_STEERING_TIME(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_PID_THROTTLE_TIME:
                m_params.pid.throttle_t = paramGet_PID_THROTTLE_TIME(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_CONTOUR_RHO:
                m_params.contour.rho = paramGet_CONTOUR_RHO(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_CONTOUR_THETA:
                m_params.contour.theta = paramGet_CONTOUR_THETA(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_CONTOUR_THRESHOLD:
                m_params.contour.threshold = paramGet_CONTOUR_THRESHOLD(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_CONTOUR_MIN_LINE_LENGTH:
                m_params.contour.min_line_length = paramGet_CONTOUR_MIN_LINE_LENGTH(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            case PARAMID_CONTOUR_MAX_LINE_GAP:
                m_params.contour.max_line_gap = paramGet_CONTOUR_MAX_LINE_GAP(valid);
                FW_ASSERT(valid == Fw::PARAM_DEFAULT || valid == Fw::PARAM_VALID, valid);
                break;
            default:
                FW_ASSERT(0, id);
        }
    }

    void NavImpl::parametersLoaded()
    {
        NavComponentBase::parametersLoaded();

        parameterUpdated(PARAMID_SCAN_ROW);
        parameterUpdated(PARAMID_SCAN_STEP);
        parameterUpdated(PARAMID_SCAN_DISCONTINUITY_THRESH_1);
        parameterUpdated(PARAMID_SCAN_DISCONTINUITY_THRESH_2);
        parameterUpdated(PARAMID_PID_P);
        parameterUpdated(PARAMID_PID_I);
        parameterUpdated(PARAMID_PID_D);
        parameterUpdated(PARAMID_PID_T);
        parameterUpdated(PARAMID_PID_STEERING_TIME);
        parameterUpdated(PARAMID_PID_THROTTLE_TIME);
        parameterUpdated(PARAMID_CONTOUR_RHO);
        parameterUpdated(PARAMID_CONTOUR_THETA);
        parameterUpdated(PARAMID_CONTOUR_THRESHOLD);
        parameterUpdated(PARAMID_CONTOUR_MIN_LINE_LENGTH);
        parameterUpdated(PARAMID_CONTOUR_MAX_LINE_GAP);
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
