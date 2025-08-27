#ifndef SPEAKER_H 
#define SPEAKER_H

#include <cstdio>
#include <miniaudio.h>

class SampleBuffer 
{
public:
    ma_data_source_base base;
    float buffer[10000] = {0};
    volatile int count = 0;

    unsigned short copy(float*, unsigned short);
    void sample(float, float);
};

class Speaker 
{   
public:
    SampleBuffer sampleBuffer;

    Speaker();
    ~Speaker();

private:
    // needed for lifetime
    ma_device device;

    static ma_result apuDataRead(ma_data_source*, void*, ma_uint64, ma_uint64*);
    
    static ma_result apuDataSeek(ma_data_source*, ma_uint64);

    static ma_result getDataFormat(ma_data_source*, ma_format*, ma_uint32*, ma_uint32*, ma_channel*, size_t);

    static ma_result getDataCursor(ma_data_source*, ma_uint64*);

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
    
    ma_result initApuData(double, double, SampleBuffer*);

    void uninitApuData(SampleBuffer*);

    static void dataCallback(ma_device*, void*, const void*, ma_uint32);
};

#endif
