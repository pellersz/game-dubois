#include "speaker.h"
#include <cstring>

void SampleBuffer::sample(float left, float right)
{
    buffer[count] = left;
    buffer[count + 1] = right;
    count += 2;
    if (count >= 5000)
        count = 0;
}

unsigned short SampleBuffer::copy(float* dest, unsigned short count)
{
    count = 2 * count;
    if (count <= this->count)
    {
        memcpy(dest, buffer, sizeof(float) * count);
        memcpy(buffer, buffer + count, sizeof(float) * (this->count - count));
        this->count -= count;
        return count;
    }
    else 
    {
        int currenctCount = this->count;
        float counter = 0;
        float diff = (float) currenctCount / count;
        for(unsigned short i = 0; i < count / 2; ++i, counter += diff)
        {
            int counterInt = counter;
            dest[2 * i] = buffer[2 * counterInt];
            dest[2 * i + 1] = buffer[2 * counterInt + 1];
        }
        memcpy(buffer, buffer + currenctCount, sizeof(float) * (this->count - currenctCount));
        this->count -= currenctCount;
    }
    return 0;
}

Speaker::Speaker() 
{
    ma_device_config deviceConfig;
 
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate        = 48000;
    deviceConfig.dataCallback      = dataCallback;
    deviceConfig.pUserData         = &sampleBuffer;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS)
    {
        printf("Failed to open playback device.\n");
        return;
    }

    initApuData(&sampleBuffer);

    if (ma_device_start(&device) != MA_SUCCESS)
    {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return;
    }
}

Speaker::~Speaker() { ma_device_uninit(&device); }

ma_result Speaker::apuDataRead
(
    ma_data_source* pDataSource, 
    void* pFramesOut, 
    ma_uint64 frameCount,
    ma_uint64* pFramesRead
)
{
    SampleBuffer* pBuffer = (SampleBuffer*) pDataSource;
    float* pFramesOutF32 = (float*)pFramesOut;
    pBuffer->copy(pFramesOutF32, frameCount);
    return MA_SUCCESS;
}

ma_result Speaker::apuDataSeek(ma_data_source* pDataSource, ma_uint64 frameIndex) { return MA_NOT_IMPLEMENTED; }

ma_result Speaker::getDataFormat
(
    ma_data_source* pDataSource,
    ma_format* pFormat,
    ma_uint32* pChannels,
    ma_uint32* pSampleRate,
    ma_channel* pChannelMap,
    size_t channelMapCap
)
{
    *pFormat = ma_format_f32;
    *pChannels = 2;
    *pSampleRate = 48000;
    return MA_SUCCESS;
}

ma_result Speaker::getDataCursor(ma_data_source* pDataSource, ma_uint64* pCursor) { return MA_NOT_IMPLEMENTED; }

ma_result Speaker::initApuData(SampleBuffer* pBuffer)
{
    ma_result result;
    ma_data_source_config baseConfig;

    baseConfig = ma_data_source_config_init();
    baseConfig.vtable = &DATA_SOURCE_VTABLE;

    result = ma_data_source_init(&baseConfig, &pBuffer->base);
    if (result != MA_SUCCESS)
        return result;

    return MA_SUCCESS;
}

void Speaker::uninitApuData(SampleBuffer* pSampleBuffer) { ma_data_source_uninit(&pSampleBuffer->base); }

void Speaker::dataCallback
(
    ma_device* pDevice,
    void* pOutput,
    const void* pInput,
    ma_uint32 frameCount
)
{ 
    apuDataRead(pDevice->pUserData, pOutput, frameCount, NULL); 
}

