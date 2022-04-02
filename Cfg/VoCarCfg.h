#ifndef VO_CMPE460_VO_CAR_H
#define VO_CMPE460_VO_CAR_H

enum
{
    CAMERA_LIBCAMERA_BUFFER_N = 4,           //!< Internal libcamera buffers
//    CAMERA_RAW_WIDTH = 1920,
//    CAMERA_RAW_HEIGHT = 1080,
    CAMERA_RAW_WIDTH = 1640,
    CAMERA_RAW_HEIGHT = 1232,

    VIS_CHESSBOARD_DOWNSCALE = 8,            //!< Downscale factor during rough step of chessboard
};

#define DRIVE               "SD:"
#define FIRMWARE_PATH       DRIVE "/firmware/"
#define CONFIG_FILE         DRIVE "/wpa_supplicant.conf"

#endif //VO_CMPE460_VO_CAR_H
