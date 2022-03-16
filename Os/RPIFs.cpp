#include <Assert.hpp>
#include <Logger.hpp>
#include <cstring>
#include "RPIFs.h"

namespace Os
{

    File::Status fatfs_to_file_status(FRESULT result)
    {
        switch (result)
        {
            case FR_OK:
                return File::OP_OK;
            case FR_NO_FILE:
            case FR_NO_PATH:
                return File::DOESNT_EXIST;
            case FR_EXIST:
                return File::FILE_EXISTS;
            case FR_WRITE_PROTECTED:
            case FR_DENIED:
                return File::NO_PERMISSION;
            case FR_INVALID_PARAMETER:
                return File::BAD_SIZE;
            case FR_INVALID_OBJECT:
            case FR_INVALID_NAME:
            case FR_DISK_ERR:
            case FR_INT_ERR:
            case FR_NOT_READY:
            case FR_INVALID_DRIVE:
            case FR_NOT_ENABLED:
            case FR_NO_FILESYSTEM:
            case FR_MKFS_ABORTED:
            case FR_TIMEOUT:
            case FR_LOCKED:
            case FR_NOT_ENOUGH_CORE:
            case FR_TOO_MANY_OPEN_FILES:
            default:
                return File::OTHER_ERROR;
        }
    }

    FileSystem::Status fatfs_to_filesystem_status(FRESULT result)
    {
        switch(result)
        {
            case FR_OK:
                return FileSystem::OP_OK;
            case FR_NOT_READY:
                return FileSystem::BUSY;
            case FR_NO_FILE:
            case FR_NO_PATH:
            case FR_INVALID_NAME:
                return FileSystem::INVALID_PATH;
            case FR_EXIST:
                return FileSystem::ALREADY_EXISTS;
            case FR_INVALID_OBJECT:
                return FileSystem::INVALID_PATH;
            case FR_DENIED:
            case FR_WRITE_PROTECTED:
                return FileSystem::NO_PERMISSION;
            case FR_TOO_MANY_OPEN_FILES:
                return FileSystem::FILE_LIMIT;
            case FR_DISK_ERR:
            case FR_INT_ERR:
            case FR_INVALID_DRIVE:
            case FR_NOT_ENABLED:
            case FR_NO_FILESYSTEM:
            case FR_MKFS_ABORTED:
            case FR_TIMEOUT:
            case FR_LOCKED:
            case FR_NOT_ENOUGH_CORE:
            case FR_INVALID_PARAMETER:
            default:
                return FileSystem::OTHER_ERROR;
        }
    }

    const char* normalize_path(const char* path, char buffer[OS_FILENAME_MAX])
    {
        if (path[0] != '/')
        {
            Fw::Logger::logMsg("Normalizing non absolute path: %s\r\n", (POINTER_CAST)path);
            strcpy(buffer, "SD:/");
        }
        else
        {
            strcpy(buffer, "SD:");
        }

        strncat(buffer, path, OS_FILENAME_MAX - 5);
        return buffer;
    }
}
