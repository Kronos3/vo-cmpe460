// ======================================================================
// \title  OsCfg.hpp
// \author tumbar
// \brief  hpp file for Raspberry Pi Os configuration
//
// ======================================================================

#ifndef RPI_OSCFG_HPP
#define RPI_OSCFG_HPP

enum OsCfg {
    OS_FILE_MAX_HANDLE = 32,       //!< Maximum number of open handles at one time
    OS_FILENAME_MAX = 80,          //!< Maximum length of path
};

#endif //RPI_OSCFG_HPP
