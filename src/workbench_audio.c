#include "workbench_audio.h"
#include "portaudio.h"
#include "workbench_config.h"
#include "workbench_logger.h"
#include "workbench_midi.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define TRY(x)                                                                 \
  err = (x);                                                                   \
  if (err != paNoError) {                                                      \
    log_e("%s", Pa_GetErrorText(err));                                         \
    audio_deinit();                                                            \
    return;                                                                    \
  }

static Config *cfg = NULL;
static PaStreamParameters input_parameters;
static PaStreamParameters output_parameters;
static PaStream *stream = NULL;
static int __audio_in_id;
static int __audio_out_id;

void stream_configure(PaStreamParameters *stream_parameters, int device_idx,
                      int channel_count, unsigned long sample_format,
                      double suggested_latency) {
  stream_parameters->device = device_idx;
  stream_parameters->channelCount = channel_count;
  stream_parameters->sampleFormat = sample_format;
  stream_parameters->suggestedLatency = suggested_latency;
  stream_parameters->hostApiSpecificStreamInfo = NULL;
  // Pa_GetStreamTime(stream);
}

static int __audio_callback(const void *input_buffer, void *output_buffer,
                            unsigned long block_size,
                            const PaStreamCallbackTimeInfo *time_info,
                            PaStreamCallbackFlags status_flags,
                            void *user_data) {
  (void)time_info;
  (void)status_flags;
  if (cfg->midi_callback)
    __midi_callback((int32_t)(time_info->currentTime / 1000), user_data);
  if (cfg->audio_callback) {
    cfg->audio_callback(input_buffer, output_buffer, block_size, user_data);
  }
  return paContinue;
}

static bool audio_device_find(char *pattern, bool input) {
  bool not_enough_channels = false;
  if (pattern == NULL)
    return false;
  for (int i = 0; i < Pa_GetDeviceCount(); i++) {
    const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
    if (strcmp(info->name, pattern) == 0) {
      if (input) {
        not_enough_channels = info->maxInputChannels < cfg->in_channel_count;
      } else {
        not_enough_channels = info->maxOutputChannels < cfg->out_channel_count;
      }
      if (not_enough_channels) {
        log_w("Not enough channels on device \"%s\"", info->name);
        return false;
      }
      return true;
    }
  }
  return false;
}

void audio_init() {
  log_d("Audio init start");
  PaError err;
  TRY(Pa_Initialize());
  cfg = get_config();

  if (!cfg->audio_input) {
    __audio_in_id = Pa_GetDefaultInputDevice();
  } else if (!audio_device_find(cfg->audio_input, true)) {
    __audio_in_id = Pa_GetDefaultInputDevice();
    log_w("Input audio device \"%s\" was not found. Using default instead.",
          cfg->audio_input);
  }
  const PaDeviceInfo *in_device_info = Pa_GetDeviceInfo(__audio_in_id);

  stream_configure(&input_parameters, __audio_in_id, cfg->in_channel_count,
                   PA_SAMPLE_FORMAT, in_device_info->defaultLowInputLatency);

  if (!cfg->audio_output) {
    __audio_out_id = Pa_GetDefaultOutputDevice();
  } else if (!audio_device_find(cfg->audio_output, false)) {
    __audio_out_id = Pa_GetDefaultOutputDevice();
    log_w("Output audio device \"%s\" was not found. Using default instead.",
          cfg->audio_output);
  }
  const PaDeviceInfo *out_device_info = Pa_GetDeviceInfo(__audio_out_id);

  stream_configure(&output_parameters, __audio_out_id, cfg->out_channel_count,
                   PA_SAMPLE_FORMAT, out_device_info->defaultLowOutputLatency);

  TRY(Pa_OpenStream(&stream, &input_parameters, &output_parameters,
                    cfg->sample_rate, cfg->block_size, cfg->audio_flags,
                    __audio_callback, cfg->user_data));
  TRY(Pa_StartStream(stream));
  log_d("Audio init finish");
}

#define PRINT_ERROR(x)                                                         \
  err = (x);                                                                   \
  if (err != paNoError) {                                                      \
    log_e("%s", Pa_GetErrorText(x));                                           \
  }

void audio_deinit() {
  PaError err;
  PRINT_ERROR(Pa_StopStream(stream));
  PRINT_ERROR(Pa_CloseStream(stream));
  PRINT_ERROR(Pa_Terminate());
}