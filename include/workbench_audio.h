/**
 * @file workbench_audio.h
 * @brief Audio system initialization and deinitialization for Workbench
 * applications.
 *
 * This header file defines functions for initializing and deinitializing the
 * audio system using the PortAudio library. It includes the type definition for
 * audio callback functions that are used to process audio data.
 *
 * The `audio_init` function sets up and starts the audio stream based on
 * configuration settings, while the `audio_deinit` function stops and closes
 * the audio stream, and terminates the PortAudio library.
 *
 * Example usage:
 * @code
 * #include "workbench_audio.h"
 *
 * void my_audio_callback(const void *input_buffer, void *output_buffer,
 * unsigned long block_size, void *user_data) {
 *     // Process audio data here
 * }
 *
 * int main() {
 *     audio_init();
 *     // Application code here
 *     audio_deinit();
 *     return 0;
 * }
 * @endcode
 */
#pragma once

#include "portaudio.h"

/// Defines the sample format to be used. Change this to change the type of the
/// audio sample. This macro should be set to one of the following values:
/// `Float32`, `Int32`, `Int24`, `Int16`, or `Int8`.
#ifndef SAMPLE_FORMAT
#define SAMPLE_FORMAT Float32
#endif

#define _SAMPLE_TYPE_NAME(T) _SAMPLE_##T
#define _SAMPLE_TYPE(T) _SAMPLE_TYPE_NAME(T)

#define _SAMPLE_Float32 float
#define _SAMPLE_Int32 uint32_t
#define _SAMPLE_Int24 uint32_t
#define _SAMPLE_Int16 uint16_t
#define _SAMPLE_Int8 uint8_t

/**
 * @brief Defines the type for audio samples based on `SAMPLE_FORMAT`.
 *
 * This macro expands to the type corresponding to `SAMPLE_FORMAT` by using
 * `SAMPLE_TYPE`. For example, if `SAMPLE_FORMAT` is `paFloat32`, this macro
 * will expand to `float`.
 */
#define SAMPLE _SAMPLE_TYPE(SAMPLE_FORMAT)

/**
 * @typedef AudioSample_t
 * @brief Defines `AudioSample_t` as the type for audio samples.
 *
 * This typedef defines `AudioSample_t` to be the type corresponding
 * to `SAMPLE_FORMAT`. For example, if `SAMPLE_FORMAT` is `paFloat32`,
 * `AudioSample_t` will be defined as `float`.
 */
typedef SAMPLE AudioSample_t;

#define _PA_SAMPLE_FORMAT_HELPER(x) pa##x
#define _PA_SAMPLE_FORMAT(x) _PA_SAMPLE_FORMAT_HELPER(x)
/// Sample format used by PortAudio
#define PA_SAMPLE_FORMAT _PA_SAMPLE_FORMAT(SAMPLE_FORMAT)

/**
 * @brief Type definition for audio callback functions.
 *
 * An `AudioCallback` function is called during audio processing to handle input
 * and output audio buffers.
 * It takes the following parameters:
 * - `input_buffer`: A pointer to the input audio buffer.
 * - `output_buffer`: A pointer to the output audio buffer.
 * - `block_size`: The number of frames in the buffer.
 * - `user_data`: A pointer to user-defined data passed to the callback
 * function.
 *
 * Example usage:
 * @code
 * void my_audio_callback(const void *input_buffer, void *output_buffer,
 * unsigned long block_size, void *user_data) {
 *   const float *in = (const float *)input_buffer;
 *   float *out = (float *)output_buffer;
 *
 *   for (unsigned long i = 0; i < block_size; ++i) {
 *     *out++ = in[i];
 *   }
 * }
 * @endcode
 */
typedef void (*AudioCallback)(const void *, void *, unsigned long, void *);

/**
 * @brief Initializes the audio system.
 *
 * This function initializes the PortAudio library, sets up the audio input and
 * output devices based on the configuration settings, and starts the audio
 * stream. It also configures the audio stream parameters such as sample rate,
 * block size, and channel counts.
 *
 * If an error occurs during initialization, it logs the error message and
 * deinitializes the audio system.
 */
void audio_init();

/**
 * @brief Deinitializes the audio system.
 *
 * This function stops and closes the audio stream, and then terminates the
 * PortAudio library. It logs any errors that occur during the deinitialization
 * process.
 */
void audio_deinit();