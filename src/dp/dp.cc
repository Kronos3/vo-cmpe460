#include <dp/dp.h>
#include <fw/fw.h>

#include <circle/new.h>
#include <evrs.h>
#include <kernel/kernel.h>

Dp::Dp()
: m_opened(FALSE),
  m_type(SIMPLE),
  m_file_handle()
{
}

boolean Dp::open(const CString &file_path)
{
    FRESULT result = f_open(&m_file_handle, file_path, FA_WRITE | FA_CREATE_NEW | FA_OPEN_APPEND);
    if (FR_OK != result)
    {
        evr_DpOpenedFailed(m_file_path);
        return FALSE;
    }

    m_file_path = file_path;
    evr_DpOpened(m_file_path);

    m_opened = TRUE;
    return TRUE;
}

void Dp::close()
{
    f_close(&m_file_handle);
    evr_DpClosed(m_file_path);

    m_file_path = "";
    m_opened = FALSE;
}

void Dp::write(const u8* data, u32 n_bytes)
{
    FRESULT result;
    u32 n_bytes_written;
    switch (m_type)
    {
        case SIMPLE:
            result = f_write(&m_file_handle, data, n_bytes, &n_bytes_written);

            if (FR_OK != result)
            {
                evr_DpWriteFailed(n_bytes, m_file_path, result);
            }

            result = f_sync(&m_file_handle);
            if (FR_OK != result)
            {
                evr_DpSyncFailed(m_file_path, result);
            }
            break;
        case BUFFERED:
            // Only flush how many bytes we need to write
            u32 buffered_n = m_buffer.enqueue(data, n_bytes);

            // Keep flushing to disk until there is enough space in the buffer
            // We usually shouldn't need to loop here if our buffer is large enough
            while(buffered_n < n_bytes)
            {
                // There was not enough space in the buffer
                // We need to flush data to the disk immediately
                const u8* sector;
                u32 sector_n;
                u32 mutex;
                m_buffer.deqeue_next_sector(sector, sector_n, mutex);
                result = f_write(&m_file_handle, sector, sector_n, &n_bytes_written);

                if (FR_OK != result)
                {
                    evr_DpWriteFailed(n_bytes, m_file_path, result);
                }

                m_buffer.release(mutex);

                // Make sure we could write all the data that was dequeued
                FW_ASSERT(n_bytes_written == sector_n, n_bytes_written, sector_n);

                // Copy more data into the buffer
                buffered_n += m_buffer.enqueue(data + buffered_n, n_bytes - buffered_n);
            }

            result = f_sync(&m_file_handle);
            if (FR_OK != result)
            {
                evr_DpSyncFailed(m_file_path, result);
            }

            break;
    }
}

void Dp::flush()
{
    if (BUFFERED != m_type)
    {
        return;
    }

    const u8* sector;
    u32 sector_n;
    u32 mutex;
    boolean keep_going = TRUE;

    FRESULT result;
    u32 n_bytes_written;

    while(keep_going)
    {
        kernel::dbgButton.Breakpoint(1);
        keep_going = m_buffer.deqeue_next_sector(sector, sector_n, mutex);
        result = f_write(&m_file_handle, sector, sector_n, &n_bytes_written);
        kernel::dbgButton.Breakpoint(2);

        if (FR_OK != result)
        {
            evr_DpWriteFailed(sector_n, m_file_path, result);
        }
        m_buffer.release(mutex);

        // Make sure we could write all the data that was dequeued
        FW_ASSERT(n_bytes_written == sector_n, n_bytes_written, sector_n);
    }

    result = f_sync(&m_file_handle);
    if (FR_OK != result)
    {
        evr_DpSyncFailed(m_file_path, result);
    }
}

void Dp::reopen()
{
    FRESULT result;
    result = f_close(&m_file_handle);
    FW_ASSERT(FR_OK == result, result);
    result = f_open(&m_file_handle, m_file_path, FA_WRITE | FA_OPEN_APPEND);
    FW_ASSERT(FR_OK == result, result);
}

Dp &DpEngine::create(const CString &file_path)
{
    u32 i;
    for (i = 0; i < DP_MAX_OPEN_FILES; i++)
    {
        if (!m_dps[i].m_opened) break;
    }

    FW_ASSERT(i < DP_MAX_OPEN_FILES && "Too many open DPs", i, DP_MAX_OPEN_FILES);

    boolean open_status = m_dps[i].open(file_path);
    FW_ASSERT(TRUE == open_status);

    return m_dps[i];
}

DpEngine::DpEngine()
: m_bufs{nullptr}
{
}

void DpEngine::schedIn()
{
    flush();
}

void DpEngine::flush()
{
    for (auto& m_dp : m_dps)
    {
        if (m_dp.m_opened)
        {
            m_dp.flush();
        }
    }
}

void DpEngine::init()
{
    for (u32 i = 0; i < DP_MAX_OPEN_FILES; i++)
    {
        m_bufs[i] = static_cast<u8*>(CMemorySystem::HeapAllocate(DP_BUFFERED_BYTES, HEAP_ANY));
        m_dps[i].m_type = Dp::BUFFERED;
        m_dps[i].m_buffer.set(m_bufs[i], DP_BUFFERED_BYTES);
    }
}
