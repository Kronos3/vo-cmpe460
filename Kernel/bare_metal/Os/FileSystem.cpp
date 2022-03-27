#include <Fw/Types/BasicTypes.hpp>
#include <Os/FileSystem.hpp>
#include <Os/File.hpp>
#include <Os/RPIFs.h>

namespace Os
{
    namespace FileSystem
    {
        Status createDirectory(const char* path)
        {
            char buf[OS_FILENAME_MAX];
            return fatfs_to_filesystem_status(f_mkdir(normalize_path(path, buf)));
        } // end createDirectory

        Status removeDirectory(const char* path)
        {
            char buf[OS_FILENAME_MAX];
            return fatfs_to_filesystem_status(f_rmdir(normalize_path(path, buf)));
        } // end removeDirectory

        Status readDirectory(const char* path,
                             const U32 maxNum,
                             Fw::String fileArray[])
        {
            DIR Directory;
            FILINFO FileInfo;

            char buf[OS_FILENAME_MAX];
            U32 out_num = 0;
            for (FRESULT Result = f_findfirst (&Directory, &FileInfo, normalize_path(path, buf), "*");
                 Result == FR_OK && FileInfo.fname[0];
                 Result = f_findnext (&Directory, &FileInfo))
            {
                if (!(FileInfo.fattrib & (AM_HID | AM_SYS)))
                {
                    if (out_num >= maxNum) break;
                    fileArray[out_num++] = FileInfo.fname;
                }
            }

            return OP_OK;
        }

        Status removeFile(const char* path)
        {
            char buf[OS_FILENAME_MAX];
            return fatfs_to_filesystem_status(f_unlink(normalize_path(path, buf)));
        } // end removeFile

        Status moveFile(const char* originPath, const char* destPath)
        {
            char buf1[OS_FILENAME_MAX];
            char buf2[OS_FILENAME_MAX];
            return fatfs_to_filesystem_status(f_rename(
                    normalize_path(originPath, buf1),
                    normalize_path(destPath, buf2)
                    ));
        } // end moveFile

        Status appendFile(const char* originPath, const char* destPath, bool createMissingDest)
        {
            File dest;
            File::Status s;
            s = dest.open(destPath, File::OPEN_APPEND);
            if (s != File::OP_OK) return OTHER_ERROR;

            File origin;
            s = origin.open(originPath, File::OPEN_READ);
            if (s != File::OP_OK) return OTHER_ERROR;

            I32 size = 1024;
            U8 block[1024];
            while(size == 1024)
            {
                s = origin.read(block, size);
                if (s != File::OP_OK) return OTHER_ERROR;
                s = dest.write(block, size);
                if (s != File::OP_OK) return OTHER_ERROR;
            }

            return OP_OK;
        } // end appendFile

        Status handleFileError(File::Status fileStatus)
        {
            return OTHER_ERROR;
        } // end handleFileError

        Status copyFile(const char* originPath, const char* destPath)
        {
            File dest;
            File::Status s;
            s = dest.open(destPath, File::OPEN_CREATE);
            if (s != File::OP_OK) return OTHER_ERROR;

            File origin;
            s = origin.open(originPath, File::OPEN_READ);
            if (s != File::OP_OK) return OTHER_ERROR;

            I32 size = 1024;
            U8 block[1024];
            while(size == 1024)
            {
                s = origin.read(block, size);
                if (s != File::OP_OK) return OTHER_ERROR;
                s = dest.write(block, size);
                if (s != File::OP_OK) return OTHER_ERROR;
            }

            return OP_OK;
        } // end copyFile

        Status getFileSize(const char* path, U64 &size)
        {
            char buf[OS_FILENAME_MAX];
            FILINFO fno;
            FRESULT status = f_stat(normalize_path(path, buf), &fno);
            if (status != FR_OK) return fatfs_to_filesystem_status(status);
            size = fno.fsize;
            return OP_OK;
        } // end getFileSize

        Status changeWorkingDirectory(const char* path)
        {
            // TODO(tumbar)
            return OTHER_ERROR;
        } // end changeWorkingDirectory

        // Public function to get the file count for a given directory.
        Status getFileCount(const char* directory, U32 &fileCount)
        {
            // TODO(tumbar)
            return OTHER_ERROR;
        } //end getFileCount
    } // end FileSystem namespace
} // end Os namespace
