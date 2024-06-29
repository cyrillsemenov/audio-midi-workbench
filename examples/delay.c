/**
 * \example delay.c
 * \brief Example implementation of a delay effect with feedback and filtering
 * using WorkBench.
 *
 * This file provides an example of implementing a delay effect with feedback
 * and filtering in an audio processing application. It uses PortAudio for audio
 * input and output, and includes MIDI control for adjusting delay parameters.
 *
 * The delay effect is implemented using a circular buffer, and the pitch
 * can be adjusted by changing the playback speed. Feedback and filtering
 * are also applied to the delayed signal.
 */

#include "workbench.h"
#include <stdlib.h>

#define DELAY_CC 48
#define FEEDBACK_CC 49
#define FILTER_CC 50
/// Maximum delay time in seconds and samples
#define BUFFER_SIZE_SEC 2
#define BUFFER_SIZE_SAMPLES (cfg->sample_rate * BUFFER_SIZE_SEC)

#define FILTER_ORDER 4

#define MIDI2DOUBLE(x) (double)(x) / 128.0

Config *cfg;

typedef struct {
  AudioSample_t *buffer;        /**< Pointer to the buffer array */
  AudioSample_t *filter_buffer; /**< Buffer for the filter */
  size_t write;                 /**< Index of the head (next write position) */
  size_t length;                /**< Maximum number of items in the buffer */
  double delay;                 /**< Current delay in samples */
  double delay_target;          /**< Target delay in samples */
  double feedback;              /**< Feedback amount */
  double filter_coefficient;    /**< Coefficient for the low-pass filter */
} DelayBuffer;

/**
 * \brief Initialize the delay buffer.
 *
 * \param del Pointer to the DelayBuffer structure.
 */
void delay_init(DelayBuffer *del);

/**
 * \brief Free the memory allocated for the delay buffer.
 *
 * \param del Pointer to the DelayBuffer structure.
 */
void free_delay_buffer(DelayBuffer *del);

/**
 * \brief Add an element to the delay buffer.
 *
 * \param del Pointer to the DelayBuffer structure.
 * \param item The item to add.
 */
void delay_put(DelayBuffer *del, AudioSample_t item);

/**
 * \brief Get an element from the delay buffer.
 *
 * \param del Pointer to the DelayBuffer structure.
 * \return The delayed sample.
 */
AudioSample_t delay_get(DelayBuffer *del);

// Audio callback function to handle recording and playback
void audio_cb(const void *input_buffer, void *output_buffer,
              unsigned long block_size, void *user_data);

// MIDI callback function to handle MIDI comntol
int midi_cb(const void *in, void *out, unsigned long length, void *user_data);

int main(int argc, char **argv) {
  DelayBuffer del;
  cfg = config_init(argc, argv, audio_cb, midi_cb, &del);
  delay_init(&del);

  while (true) {
    Pa_Sleep(1000);
  }

  free_delay_buffer(&del);
  return 0;
}

void audio_cb(const void *input_buffer, void *output_buffer,
              unsigned long block_size, void *user_data) {
  DelayBuffer *del = (DelayBuffer *)user_data;
  if (del == NULL) {
    return;
  }
  const AudioSample_t *in = (const float *)input_buffer;
  AudioSample_t *out = (AudioSample_t *)output_buffer;

  for (unsigned long i = 0; i < block_size; ++i) {
    // Read the delayed sample from the buffer
    AudioSample_t out_sample = delay_get(del);

    // Write the input sample to the delay buffer
    delay_put(del, in[i] * (1 - del->feedback) + out_sample * del->feedback);

    // Output the delayed sample to all channels
    int channels = cfg->out_channel_count;
    while (channels--) {
      *out++ = out_sample;
    }
  }
}

int midi_cb(const void *in, void *out, unsigned long length, void *user_data) {
  DelayBuffer *del = (DelayBuffer *)user_data;
  if (del == NULL) {
    return 0;
  }
  const PmEvent *in_buffer = (const PmEvent *)in;

  for (unsigned long i = 0; i < length; ++i) {
    PmEvent event = in_buffer[i];
    PmMessage message = event.message;

    uint8_t command = Pm_MessageStatus(message) & MIDI_CODE_MASK;
    uint8_t data1 = Pm_MessageData1(message);
    uint8_t data2 = Pm_MessageData2(message) + 1;

    if (command == MIDI_CTRL) {
      switch (data1) {
      case DELAY_CC:
        del->delay_target = MIDI2DOUBLE(data2);
        break;
      case FEEDBACK_CC:
        del->feedback = MIDI2DOUBLE(data2);
        break;
      case FILTER_CC:
        del->filter_coefficient = MIDI2DOUBLE(data2);
        break;
      }
    }
  }
  return 0;
}

void delay_init(DelayBuffer *del) {
  del->buffer =
      (AudioSample_t *)calloc(BUFFER_SIZE_SAMPLES, sizeof(AudioSample_t));
  del->filter_buffer =
      (AudioSample_t *)malloc(FILTER_ORDER * sizeof(AudioSample_t));
  del->length = BUFFER_SIZE_SAMPLES;
  del->write = 0;
  del->delay = del->delay_target = 0.1; // Initial delay
  del->feedback = 0;
  del->filter_coefficient = 0.5;
}

void free_delay_buffer(DelayBuffer *del) {
  free(del->buffer);
  free(del->filter_buffer);
}

void delay_put(DelayBuffer *del, AudioSample_t item) {
  del->buffer[del->write] = item;
  del->write = (del->write + 1) % del->length; // Wrap around if end is reached
}

#define lerp(a, b, t) (a + t * (b - a))

AudioSample_t delay_get(DelayBuffer *del) {
  // Interpolate the delay adjustment
  del->delay = lerp(del->delay, del->delay_target, 1 / cfg->sample_rate);

  // Calculate the read index for current delay
  double read_idx = del->write - del->delay * (del->length - cfg->block_size) +
                    cfg->block_size;
  if (read_idx < 0) {
    read_idx += del->length;
  }

  // Get the integer part and fractional part of the read index
  size_t idx0 = (size_t)read_idx;
  size_t idx1 = (idx0 + 1) % del->length;
  double frac = read_idx - idx0;

  // Get the delayed samples from the buffer
  AudioSample_t sample0 = del->buffer[idx0];
  AudioSample_t sample1 = del->buffer[idx1];
  AudioSample_t interpolated_sample = lerp(sample0, sample1, frac);

  // Apply the multi-stage low-pass filter
  AudioSample_t filtered_sample = interpolated_sample;
  for (size_t i = 0; i < FILTER_ORDER; ++i) {
    filtered_sample = del->filter_coefficient * filtered_sample +
                      (1.0 - del->filter_coefficient) * del->filter_buffer[i];
    del->filter_buffer[i] = filtered_sample;
  }

  // Return the interpolated sample
  return filtered_sample;
}