#include <FpConfig.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Os/File.hpp>
#include <Fw/Types/Assert.hpp>
#include <OsCfg.hpp>
#include <RPIFs.h>

namespace Os
{
    static struct FileImpl
    {
        FIL fp;     //!< FatFS file structure
        bool in_use;
    } file_handles[OS_FILE_MAX_HANDLE];

    File::File() : m_fd(0), m_mode(OPEN_NO_MODE), m_lastError(0)
    {}

    File::~File() = default;

    File::Status File::open(const char* fileName, File::Mode mode)
    {
        FW_ASSERT(fileName);

        BYTE fat_fs_mode = 0;
        switch (mode)
        {
            default:
            case OPEN_NO_MODE:
                FW_ASSERT(0);
                break;
            case OPEN_READ:
                fat_fs_mode = FA_READ;
                break;
            case OPEN_SYNC_WRITE:
            case OPEN_WRITE:
            case OPEN_SYNC_DIRECT_WRITE:
                fat_fs_mode = FA_WRITE | FA_OPEN_ALWAYS;
                break;
            case OPEN_CREATE:
                fat_fs_mode = FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_NEW;
                break;
            case OPEN_APPEND:
                fat_fs_mode = FA_WRITE | FA_OPEN_ALWAYS | FA_OPEN_APPEND;
                break;
        }

        // Find an open file descriptor
        bool found = false;
        for (I32 i = 0; i < OS_FILE_MAX_HANDLE; i++)
        {
            if (!file_handles[i].in_use)
            {
                m_fd = i + 1;
                file_handles[i].in_use = true;
                found = true;
                break;
            }
        }

        if (!found)
        {
            // No more open file handles
            return NO_SPACE;
        }

        char buf[OS_FILENAME_MAX];
        FRESULT result = f_open(&file_handles[m_fd - 1].fp,
                                normalize_path(fileName, buf),
                                fat_fs_mode);
        m_lastError = result;
        m_mode = mode;
        return fatfs_to_file_status(result);
    }

    File::Status File::open(const char* fileName, File::Mode mode, bool include_excl)
    {
        (void) include_excl;
        return open(fileName, mode);
    }

    bool File::isOpen()
    {
        return m_fd > 0;
    }

    File::Status File::seek(NATIVE_INT_TYPE offset, bool absolute)
    {
        if (!isOpen()) return NOT_OPENED;
        FW_ASSERT(m_fd < OS_FILE_MAX_HANDLE + 1);

        FRESULT result;
        if (absolute)
        {
            result = f_lseek(&file_handles[m_fd - 1].fp, offset);
        }
        else
        {
            result = f_lseek(
                    &file_handles[m_fd - 1].fp,
                    f_tell(&file_handles[m_fd - 1].fp) + offset);
        }

        m_lastError = result;

        return fatfs_to_file_status(result);
    }

    File::Status File::read(void* buffer, NATIVE_INT_TYPE &size, bool waitForFull)
    {
        if (!isOpen()) return NOT_OPENED;
        FW_ASSERT(m_fd < OS_FILE_MAX_HANDLE + 1);
        (void) waitForFull;

        UINT actually_read;
        FRESULT result = f_read(&file_handles[m_fd - 1].fp, buffer, size, &actually_read);
        size = static_cast<NATIVE_INT_TYPE>(actually_read);
        m_lastError = result;

        return fatfs_to_file_status(result);
    }

    File::Status File::write(const void* buffer, NATIVE_INT_TYPE &size, bool waitForDone)
    {
        (void) waitForDone;
        if (!isOpen()) return NOT_OPENED;
        FW_ASSERT(m_fd < OS_FILE_MAX_HANDLE + 1);

        UINT actually_write;
        FRESULT result = f_write(&file_handles[m_fd - 1].fp, buffer, size, &actually_write);
        size = static_cast<NATIVE_INT_TYPE>(actually_write);
        m_lastError = result;

        return fatfs_to_file_status(result);
    }

    void File::close()
    {
        if (!isOpen()) return;
        FW_ASSERT(m_fd < OS_FILE_MAX_HANDLE + 1);

        f_close(&file_handles[m_fd - 1].fp);
        file_handles[m_fd - 1].in_use = false;
        m_fd = 0;
    }

    NATIVE_INT_TYPE File::getLastError()
    {
        if (!isOpen()) return 0;
        FW_ASSERT(m_fd < OS_FILE_MAX_HANDLE + 1);
        return m_lastError;
    }

    const char* File::getLastErrorString()
    {
        if (!isOpen()) return "";
        FW_ASSERT(m_fd < OS_FILE_MAX_HANDLE + 1);
        switch (m_lastError)
        {
            case FR_OK:
                return "OK";
            case FR_DISK_ERR:
                return "DISK_ERR";
            case FR_INT_ERR:
                return "INT_ERR";
            case FR_NOT_READY:
                return "NOT_READY";
            case FR_NO_FILE:
                return "NO_FILE";
            case FR_NO_PATH:
                return "NO_PATH";
            case FR_INVALID_NAME:
                return "INVALID_NAME";
            case FR_DENIED:
                return "DENIED";
            case FR_EXIST:
                return "EXIST";
            case FR_INVALID_OBJECT:
                return "INVALID_OBJECT";
            case FR_WRITE_PROTECTED:
                return "WRITE_PROTECTED";
            case FR_INVALID_DRIVE:
                return "INVALID_DRIVE";
            case FR_NOT_ENABLED:
                return "NOT_ENABLED";
            case FR_NO_FILESYSTEM:
                return "NO_FILESYSTEM";
            case FR_MKFS_ABORTED:
                return "MKFS_ABORTED";
            case FR_TIMEOUT:
                return "TIMEOUT";
            case FR_LOCKED:
                return "LOCKED";
            case FR_NOT_ENOUGH_CORE:
                return "NOT_ENOUGH_CORE";
            case FR_TOO_MANY_OPEN_FILES:
                return "TOO_MANY_OPEN_FILES";
            case FR_INVALID_PARAMETER:
                return "INVALID_PARAMETER";
        }

        return "UNKNOWN_ERROR";
    }

}
