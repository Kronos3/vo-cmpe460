#ifndef VO_CMPE460_EVR_H
#define VO_CMPE460_EVR_H

#include <fw/fw_time.h>
#include <cfg/vo_car.h>
#include <rate_driver/scheduled.h>
#include <circle/circle_string.h>
#include <circle/genericlock.h>

class DpEngine;
class Dp;
class EvrEngine : public Scheduled
{
    u32 m_queue_n;                              //!< Number of EVRs waiting in queue
    u8 m_queue_internal[EVR_QUEUE_MAX_BYTES];   //!< Buffer for EVR queue
    Fw::ExternalSerializeBuffer m_queue;        //!< Actual EVR queue wrapping the internal buffer
    u32 m_evrs_dropped_n;                       //!< Number of EVRs dropped due to buffer filling

    DpEngine* m_dp_engine;                      //!< Data product engine for generating EVR products
    Dp* m_dp;                                   //!< Resultant open EVR data product

//    CGenericLock lock;                          //!< Log mutex

public:
    explicit EvrEngine(DpEngine*);

    void init();

    /**
     * Open an EVR to begin writing a data product to
     * @param evr_file file path to write EVRs to
     */
    void open(const CString& evr_file);

    /**
     * Public interface to send an EVR to telemetry comm device
     * @param evr EVR buffer from the auto-coder
     */
    void log(const Fw::SerializeBufferBase& evr);

    void flush();

    void close();

private:
    /**
     * Flush the internal EVR queue to the data product
     */
    void schedIn() override;
};

#endif //VO_CMPE460_EVR_H
