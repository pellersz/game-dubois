#include "speaker.h"
#include <cstring>
#include <iostream>

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
    //std::cout << count << " " << this->count << " " << buffer[0] << " " << buffer[1] << std::endl;
    count = 2 * count;
    if (count <= this->count)
    {
        //TODO: sizeof
        memcpy(dest, buffer, 4 * count);
        memcpy(buffer, buffer + count, 4 * (this->count - count));
        this->count -= count;
        return count;
    }
    else 
    {
        int currenct_count = this->count;
        float counter = 0;
        float diff = (float) currenct_count / count;
        for(unsigned short i = 0; i < count / 2; ++i, counter += diff)
        {
            int conter_int = counter;
            dest[2 * i] = buffer[2 * conter_int];
            dest[2 * i + 1] = buffer[2 * conter_int + 1];
        }
        memcpy(buffer, buffer + currenct_count, 4 * (this->count - currenct_count));
        this->count -= currenct_count;
    }
    return 0;
}

Speaker::Speaker() 
{
    ma_device_config device_config;
 
    device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format   = ma_format_f32;
    device_config.playback.channels = 2;
    device_config.sampleRate        = 48000;
    //device_config.periodSizeInFrames = 1800;
    device_config.dataCallback      = dataCallback;
    device_config.pUserData         = &sampleBuffer;

    if (ma_device_init(NULL, &device_config, &device) != MA_SUCCESS)
    {
        printf("Failed to open playback device.\n");
        return;
    }

    initApuData(48000, 220, &sampleBuffer);

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
    ma_data_source* p_data_source, 
    void* p_frames_out, 
    ma_uint64 frame_count,
    ma_uint64* p_frames_read
)
{
    SampleBuffer* p_buffer = (SampleBuffer*) p_data_source;
    float* p_frames_out_f32 = (float*)p_frames_out;
    p_buffer->copy(p_frames_out_f32, frame_count);
    return MA_SUCCESS;
}

ma_result Speaker::apuDataSeek(ma_data_source* p_data_source, ma_uint64 frame_index) { return MA_NOT_IMPLEMENTED; }

ma_result Speaker::getDataFormat
(
    ma_data_source* p_data_source,
    ma_format* p_format,
    ma_uint32* p_channels,
    ma_uint32* p_sample_rate,
    ma_channel* p_channel_map,
    size_t channel_map_cap
)
{
    *p_format = ma_format_f32;
    *p_channels = 2;
    *p_sample_rate = 48000;
    return MA_SUCCESS;
}

ma_result Speaker::getDataCursor(ma_data_source* p_data_source, ma_uint64* p_cursor) { return MA_NOT_IMPLEMENTED; }

ma_result Speaker::initApuData(double sample_rate, double frequency, SampleBuffer* p_buffer)
{
    ma_result result;
    ma_data_source_config baseConfig;

    baseConfig = ma_data_source_config_init();
    baseConfig.vtable = &DATA_SOURCE_VTABLE;

    result = ma_data_source_init(&baseConfig, &p_buffer->base);
    if (result != MA_SUCCESS)
        return result;

    //p_channels->advance = (1.0 / (sample_rate / frequency));

    return MA_SUCCESS;
}

void Speaker::uninitApuData(SampleBuffer* p_sample_buffer) { ma_data_source_uninit(&p_sample_buffer->base); }

void Speaker::dataCallback
(
    ma_device* p_device,
    void* p_output,
    const void* p_input,
    ma_uint32 frame_count
)
{ apuDataRead(p_device->pUserData, p_output, frame_count, NULL); }

