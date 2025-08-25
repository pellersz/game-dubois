#ifndef SPEAKER_H 
#define SPEAKER_H

#include <cstdio>
#include <miniaudio.h>
#include "channels.h"

class Speaker 
{   
public:
    Channels channels;

    Speaker();
    ~Speaker();

private:
    // needed for lifetime
    ma_device device;

    static ma_result apuDataRead(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead);
    
    static ma_result apuDataSeek(ma_data_source* pDataSource, ma_uint64 frameIndex);

    static ma_result getDataFormat(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap);

    static ma_result getDataCursor(ma_data_source* pDataSource, ma_uint64* pCursor);

    constexpr static const ma_data_source_vtable DATA_SOURCE_VTABLE =
    {
        apuDataRead,
        apuDataSeek,
        getDataFormat,
        getDataCursor,
        NULL,
        NULL,
        0
    };
    
    ma_result initApuData(double sampleRate, double frequency, Channels* pMyDataSource);

    void uninitApuData(Channels* pMyDataSource);

    static void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
};

#endif
