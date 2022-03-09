#ifndef VO_CMPE460_DP_H
#define VO_CMPE460_DP_H

#include <circle/types.h>
#include <circle/string.h>

#include <rate_driver/scheduled.h>
#include <fw/serializable.h>

#include <cfg/vo_car.h>
#include <fw/circular_buffer.h>
#include <fatfs/ff.h>

class DpEngine;
class Dp
{
    friend DpEngine;
public:
    enum Type
    {
        SIMPLE,         //!< Data product will write in one shot
        BUFFERED,       //!< Data product will be buffered and can be written to multiple times
    };

    Dp();

    /**
     * Open a data product at a path in the filesystem
     * This will open the product in SIMPLE mode
     * @param file_path path the data product should be stored
     * @param type data product type
     * @return TRUE if DP was successfully opened
     */
    boolean open(const CString& file_path);

    /**
     * Write some data to the data product
     * SIMPLE: Data is written immediately to flash via DMA
     * BUFFERED: Data is buffered until we can fill an entire DMA frame or is flushed
     * @param data
     * @param n_bytes
     */
    void write(const u8* data, u32 n_bytes);

    /**
     * Flushes the internal buffers to the SDCard
     * Note: ONLY for BUFFERED, SIMPLE will do nothing
     */
    void flush();

    /**
     * Close the data product
     */
    void close();

    void reopen();

private:
    boolean m_opened;               //!< A data product is open (for buffered, this is just in use)
    Type m_type;                    //!< Data product type
    CString m_file_path;            //!< Path to the data product in the filesystem
    FIL m_file_handle;              //!< Open OS file handle

    Fw::CircularBuffer m_buffer;    //!< Internal buffer for BUFFERED mode
};

class DpEngine : public Scheduled
{
    Dp m_dps[DP_MAX_OPEN_FILES];
    u8* m_bufs[DP_MAX_OPEN_FILES];

public:
    DpEngine();

    void init();

    /**
     * Create and open a buffered data product
     * @param file_path path to buffered data product
     * @return reference to data product stored internally in the engine
     */
    Dp& create(const CString& file_path);

    /**
     * Flush all open buffered files to disk
     */
    void flush();

private:
    /**
     * Flushes all open files to SDCard
     */
    void schedIn() override;
};

#endif //VO_CMPE460_DP_H
