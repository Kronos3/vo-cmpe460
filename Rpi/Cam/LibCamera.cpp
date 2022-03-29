#include "LibCamera.h"
#include <sys/mman.h>

namespace Rpi
{
    LibCamera::LibCamera(CamImpl &cam)
    : m_cam(cam)
    {
    }

    I32 LibCamera::init(I32 width, I32 height,
                        libcamera::PixelFormat format,
                        U32 buffer_count,
                        I32 rotation)
    {
        I32 ret;
        m_cm = std::make_unique<libcamera::CameraManager>();
        ret = m_cm->start();
        if (ret)
        {
            m_cam.log_WARNING_HI_CameraManagerStartFailed();
            return ret;
        }

        std::string cameraId = m_cm->cameras()[0]->id();
        m_camera = m_cm->get(cameraId);
        Fw::LogStringArg cam_id_fw = cameraId.c_str();

        if (!m_camera)
        {
            m_cam.log_WARNING_HI_CameraNotFound(cam_id_fw);
            return 1;
        }

        if (m_camera->acquire())
        {
            m_cam.log_WARNING_HI_CameraAcquireFailed(cam_id_fw);
            return 1;
        }

        m_acquired = true;

        std::unique_ptr<libcamera::CameraConfiguration> config;
        config = m_camera->generateConfiguration({libcamera::StreamRole::Viewfinder});
        libcamera::Size size(width, height);
        config->at(0).pixelFormat = format;
        config->at(0).size = size;

        if (buffer_count)
        {
            config->at(0).bufferCount = buffer_count;
        }

        libcamera::Transform transform = libcamera::Transform::Identity;

        bool ok;
        libcamera::Transform rot = libcamera::transformFromRotation(rotation, &ok);
        if (!ok)
        {
            FW_ASSERT(0 && "Illegal rotation, use 0 or 180", rotation);
        }

        transform = rot * transform;
        if (!!(transform & libcamera::Transform::Transpose))
        {
            FW_ASSERT(0 && "Transpose not supported");
        }

        config->transform = transform;

        switch (config->validate())
        {
            case libcamera::CameraConfiguration::Valid:
                m_cam.log_ACTIVITY_LO_CameraConfigValid();
                break;

            case libcamera::CameraConfiguration::Adjusted:
                m_cam.log_ACTIVITY_LO_CameraConfigAdjusted();
                break;

            case libcamera::CameraConfiguration::Invalid:
                m_cam.log_WARNING_HI_CameraConfigInvalid();
                return 1;
        }

        m_config = std::move(config);
        return 0;
    }

    I32 LibCamera::start()
    {
        I32 ret;
        ret = m_camera->configure(m_config.get());
        if (ret < 0)
        {
            m_cam.log_WARNING_HI_CameraConfigFailed();
            return ret;
        }

        m_camera->requestCompleted.connect(this, &LibCamera::request_complete);
        m_allocator = std::make_unique<libcamera::FrameBufferAllocator>(m_camera);

        return start_capture();
    }

    int LibCamera::start_capture()
    {
        I32 ret;
        U32 nbuffers = UINT32_MAX;

        for (libcamera::StreamConfiguration &cfg: *m_config)
        {
            ret = m_allocator->allocate(cfg.stream());
            if (ret < 0)
            {
                m_cam.log_WARNING_HI_CameraAllocateFailed();
                return -ENOMEM;
            }

            U32 allocated = m_allocator->buffers(cfg.stream()).size();
            nbuffers = std::min(nbuffers, allocated);
        }

        for (U32 i = 0; i < nbuffers; i++)
        {
            std::unique_ptr<libcamera::Request> request = m_camera->createRequest();
            if (!request)
            {
                m_cam.log_WARNING_HI_CameraRequestFailed();
                return -ENOMEM;
            }

            for (libcamera::StreamConfiguration &cfg: *m_config)
            {
                libcamera::Stream* stream = cfg.stream();
                const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers =
                        m_allocator->buffers(stream);
                const std::unique_ptr<libcamera::FrameBuffer> &buffer = buffers[i];

                ret = request->addBuffer(stream, buffer.get());
                if (ret < 0)
                {
                    m_cam.log_WARNING_HI_CameraBufferSetFailed();
                    return ret;
                }

                for (const libcamera::FrameBuffer::Plane &plane: buffer->planes())
                {
                    void* memory = mmap(nullptr, plane.length,
                                        PROT_READ, MAP_SHARED,
                                        plane.fd.get(), 0);
                    m_buffers[plane.fd.get()] =
                            std::make_pair(memory, plane.length);
                }
            }

            m_requests.push_back(std::move(request));
        }

        ret = m_camera->start(&m_controls);
        // ret = m_camera->start();
        if (ret)
        {
            m_cam.log_WARNING_HI_CameraCaptureStartFailed();
            return ret;
        }
        m_controls.clear();
        m_started = true;

        for (std::unique_ptr<libcamera::Request> &request: m_requests)
        {
            ret = queue_request(request.get());
            if (ret < 0)
            {
                m_cam.log_WARNING_HI_CameraQueueFailed();
                m_camera->stop();
                return ret;
            }
        }

        return 0;
    }

    int LibCamera::queue_request(libcamera::Request* request)
    {
        std::lock_guard<std::mutex> stop_lock(m_camera_stop_mutex);
        if (!m_started)
            return -1;
        {
            std::lock_guard<std::mutex> lock(m_controls_mutex);
            request->controls() = std::move(m_controls);
        }
        return m_camera->queueRequest(request);
    }

    void LibCamera::request_complete(libcamera::Request* request)
    {
        if (request->status() == libcamera::Request::RequestCancelled)
        {
            return;
        }

        process_request(request);
    }

    void LibCamera::process_request(libcamera::Request* request)
    {
        std::lock_guard<std::mutex> lock(m_free_requests_mutex);
        m_request_queue.push(request);
    }

    void LibCamera::return_buffer(POINTER_CAST request)
    {
        FW_ASSERT(request);

        auto* req = (libcamera::Request*) request;
        req->reuse(libcamera::Request::ReuseBuffers);
        queue_request(req);
    }

    bool LibCamera::read_frame(Frame &frameData)
    {
        std::lock_guard<std::mutex> lock(m_free_requests_mutex);
        if (!m_request_queue.empty())
        {
            libcamera::Request* request = m_request_queue.front();

            const libcamera::Request::BufferMap &buffers = request->buffers();
            for (auto it: buffers)
            {
                libcamera::FrameBuffer* buffer = it.second;
                for (U32 i = 0; i < buffer->planes().size(); ++i)
                {
                    const libcamera::FrameBuffer::Plane &plane = buffer->planes()[i];
                    const libcamera::FrameMetadata::Plane &meta = buffer->metadata().planes()[i];

                    void* data = m_buffers[plane.fd.get()].first;
                    U32 length = std::min(meta.bytesused, plane.length);

                    frameData.size = length;
                    frameData.data = (U8*) data;
                }
            }

            m_request_queue.pop();
            frameData.request = (POINTER_CAST) request;
            return true;
        }
        else
        {
            libcamera::Request* request = nullptr;
            frameData.request = (POINTER_CAST) request;
            return false;
        }
    }

    void LibCamera::set(libcamera::ControlList controls)
    {
        m_controls = std::move(controls);
    }

    void LibCamera::stop()
    {
        if (m_camera)
        {
            std::lock_guard<std::mutex> lock(m_camera_stop_mutex);
            if (m_started)
            {
                if (m_camera->stop())
                {
                    m_cam.log_WARNING_HI_CameraStopFailed();
                }

                m_started = false;
            }

            m_camera->requestCompleted.disconnect(this, &LibCamera::request_complete);
        }

        while (!m_request_queue.empty())
        {
            m_request_queue.pop();
        }

        m_requests.clear();
        m_allocator.reset();
        m_controls.clear();
    }

    void LibCamera::close()
    {
        if (m_acquired)
        {
            m_camera->release();
        }

        m_acquired = false;
        m_camera.reset();
        m_cm.reset();
    }

    LibCamera::~LibCamera()
    {
        close();
    }
}
