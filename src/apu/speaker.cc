#include "speaker.h"

Speaker::Speaker() 
{
    ma_device_config device_config;
 
    device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format   = ma_format_f32;
    device_config.playback.channels = 2;
    device_config.sampleRate        = 48000;
    device_config.dataCallback      = dataCallback;
    device_config.pUserData         = &channel3;

    if (ma_device_init(NULL, &device_config, &device) != MA_SUCCESS)
    {
        printf("Failed to open playback device.\n");
        return;
    }

    initApuData(48000, 220, &channel3);

    if (ma_device_start(&device) != MA_SUCCESS)
    {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return;
    }
}

Speaker::~Speaker() { ma_device_uninit(&device); }

ma_result Speaker::apuDataRead(ma_data_source* p_data_source, void* p_frames_out, ma_uint64 frame_count, ma_uint64* p_frames_read)
{
    Channel3* m_data = (Channel3*) p_data_source;
    float* frames_out_f32 = (float*)p_frames_out;

    for (int i_frame = 0; i_frame < frame_count; i_frame += 1)
    {
        double f = m_data->time - (ma_int64)m_data->time;
        double s;
    
        if (f < 0.5) 
            s = 0.2f;
        else
            s = -0.2f;
    
        m_data->time += m_data->advance;

        for (int iChannel = 0; iChannel < 2; iChannel += 1)
            frames_out_f32[i_frame*2 + iChannel] = s;
    }
    return MA_SUCCESS;
}

ma_result Speaker::apuDataSeek(ma_data_source* p_data_source, ma_uint64 frame_index) { return MA_NOT_IMPLEMENTED; }

ma_result Speaker::getDataFormat(ma_data_source* p_data_source, ma_format* p_format, ma_uint32* p_channels, ma_uint32* p_sample_rate, ma_channel* p_channel_map, size_t channel_map_cap)
{
    *p_format = ma_format_f32;
    *p_channels = 2;
    *p_sample_rate = 48000;
    return MA_SUCCESS;
}

ma_result Speaker::getDataCursor(ma_data_source* p_data_source, ma_uint64* p_cursor) { return MA_NOT_IMPLEMENTED; }

ma_result Speaker::initApuData(double sample_rate, double frequency, Channel3* p_channel3)
{
    ma_result result;
    ma_data_source_config baseConfig;

    baseConfig = ma_data_source_config_init();
    baseConfig.vtable = &DATA_SOURCE_VTABLE;

    result = ma_data_source_init(&baseConfig, &p_channel3->base);
    if (result != MA_SUCCESS)
        return result;

    p_channel3->advance = (1.0 / (sample_rate / frequency));

    return MA_SUCCESS;
}

void Speaker::uninitApuData(Channel3* p_channel3) { ma_data_source_uninit(&p_channel3->base); }

void Speaker::dataCallback(ma_device* p_device, void* p_output, const void* p_input, ma_uint32 frame_count)
{
    Channel3* pSineWave;

    pSineWave = (Channel3*)p_device->pUserData;

    apuDataRead(pSineWave, p_output, frame_count, NULL);

    (void)p_input;   /* Unused. */
}

