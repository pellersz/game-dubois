#include "speaker.h"

Speaker::Speaker() 
{
    ma_device_config deviceConfig;
    ma_device device;
 
    my_data_source s;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate        = 48000;
    deviceConfig.dataCallback      = dataCallback;
    deviceConfig.pUserData         = &s;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return;
    }

    printf("Device Name: %s\n", device.playback.name);

    //ma_waveform_init(&sineWaveConfig, &sineWave);

    initApuData(48000, 220, &s);

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return;
    }
    
    printf("Press Enter to quit...\n");
    getchar();
    
    ma_device_uninit(&device);

}

Speaker::~Speaker() {}

ma_result Speaker::apuDataRead(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead)
{
    // Read data here. Output in the same format returned by my_data_source_get_data_format().
    ma_uint64 iFrame;
    ma_uint64 iChannel;

    my_data_source* m_data = (my_data_source*) pDataSource;
    float* pFramesOutF32 = (float*)pFramesOut;
    for (iFrame = 0; iFrame < frameCount; iFrame += 1) {
        double f = m_data->time - (ma_int64)m_data->time;
        double s;
    
        if (f < 0.5) {
            s = 0.2f;
        } else {
            s = -0.2f;
        }
    
        m_data->time += m_data->advance;

        for (iChannel = 0; iChannel < 2; iChannel += 1) {
            pFramesOutF32[iFrame*2 + iChannel] = s;
        }
    }
    return MA_SUCCESS;
}

ma_result Speaker::apuDataSeek(ma_data_source* pDataSource, ma_uint64 frameIndex)
{
    // Seek to a specific PCM frame here. Return MA_NOT_IMPLEMENTED if seeking is not supported.
    return MA_NOT_IMPLEMENTED;
}

ma_result Speaker::getDataFormat(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap)
{
    // Return the format of the data here.
    *pFormat = ma_format_f32;
    *pChannels = 2;
    *pSampleRate = 48000;
    return MA_SUCCESS;
}

ma_result Speaker::getDataCursor(ma_data_source* pDataSource, ma_uint64* pCursor)
{
    // Retrieve the current position of the cursor here. Return MA_NOT_IMPLEMENTED and set *pCursor to 0 if there is no notion of a cursor.
    return MA_NOT_IMPLEMENTED;
}

ma_result Speaker::initApuData(double sampleRate, double frequency, my_data_source* pMyDataSource)
{
    ma_result result;
    ma_data_source_config baseConfig;

    baseConfig = ma_data_source_config_init();
    baseConfig.vtable = &DATA_SOURCE_VTABLE;

    result = ma_data_source_init(&baseConfig, &pMyDataSource->base);
    if (result != MA_SUCCESS) {
        return result;
    }

    pMyDataSource->advance = (1.0 / (sampleRate / frequency));

    return MA_SUCCESS;
}

void Speaker::uninitApuData(my_data_source* pMyDataSource)
{
    // ... do the uninitialization of your custom data source here ...

    // You must uninitialize the base data source.
    ma_data_source_uninit(&pMyDataSource->base);
}

void Speaker::dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    my_data_source* pSineWave;

    pSineWave = (my_data_source*)pDevice->pUserData;

    apuDataRead(pSineWave, pOutput, frameCount, NULL);

    (void)pInput;   /* Unused. */
}

