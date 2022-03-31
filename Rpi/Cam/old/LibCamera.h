#include <queue>
#include <mutex>

#include <libcamera/controls.h>
#include <libcamera/control_ids.h>
#include <libcamera/property_ids.h>
#include <libcamera/libcamera.h>
#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>
#include <libcamera/formats.h>
#include <libcamera/transform.h>
#include <BasicTypes.hpp>

#include "CamImpl.h"

namespace Rpi
{
    typedef struct
    {
        BYTE* data;
        U32 size;
        POINTER_CAST request;
    } Frame;

    class LibCamera
    {
    public:
        explicit LibCamera(CamImpl& cam);
        ~LibCamera();

        I32 init(I32 width, I32 height,
                 libcamera::PixelFormat format,
                 U32 buffer_count,
                 I32 rotation);

        I32 start();

        bool read_frame(Frame& frameData);

        void return_buffer(POINTER_CAST request);

        void set(libcamera::ControlList controls);

        void stop();

        void close();

    private:
        I32 start_capture();

        I32 queue_request(libcamera::Request* request);

        void request_complete(libcamera::Request* request);

        void process_request(libcamera::Request* request);

        CamImpl &m_cam;

        std::unique_ptr<libcamera::CameraManager> m_cm;
        std::shared_ptr<libcamera::Camera> m_camera;

        bool m_acquired = false;
        bool m_started = false;
        std::unique_ptr<libcamera::CameraConfiguration> m_config;
        std::unique_ptr<libcamera::FrameBufferAllocator> m_allocator;
        std::vector<std::unique_ptr<libcamera::Request>> m_requests;
        std::map<I32, std::pair<void*, U32>> m_buffers;

        std::queue<libcamera::Request*> m_request_queue;

        std::mutex m_controls_mutex;
        libcamera::ControlList m_controls;

        std::mutex m_camera_stop_mutex;
        std::mutex m_free_requests_mutex;
    };
}
