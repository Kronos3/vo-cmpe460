#ifndef VO_CMPE460_RPIFS_H
#define VO_CMPE460_RPIFS_H

#include <Os/File.hpp>
#include <Os/FileSystem.hpp>
#include <fatfs/ff.h>
#include <OsCfg.hpp>

namespace Os
{
    File::Status fatfs_to_file_status(FRESULT result);
    FileSystem::Status fatfs_to_filesystem_status(FRESULT result);

    const char* normalize_path(const char* path, char buffer[OS_FILENAME_MAX]);


}

#endif //VO_CMPE460_RPIFS_H
