/**
 * @file workbench.h
 */
#pragma once

// clang-format off
#include "portaudio.h"
#include "portmidi.h"
#include "porttime.h"
#include "workbench_logger.h"
#include "workbench_config.h"
#include "workbench_audio.h"
#include "workbench_midi.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
// clang-format on

/**
 * @defgroup audio Audio
 * @brief Audio system initialization and deinitialization for Workbench
 * applications.
 *
 * This header file defines functions for initializing and deinitializing
 * the audio system using the PortAudio library. It includes the type definition
 * for audio callback functions that are used to process audio data.
 *
 * The `audio_init` function sets up and starts the audio stream based
 * on configuration settings, while the `audio_deinit` function stops and closes
 * the audio stream, and terminates the PortAudio library.
 * @{ */

/**
 * @typedef AudioSample_t
 * @brief Defines `AudioSample_t` as the type for audio samples.
 *
 * This typedef defines `AudioSample_t` to be the type corresponding
 * to `SAMPLE_FORMAT`. For example, if `SAMPLE_FORMAT` is `paFloat32`,
 * `AudioSample_t` will be defined as `float`.
 */
typedef SAMPLE AudioSample_t;

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
 * This function initializes the PortAudio library, sets up the audio input
 * and output devices based on the configuration settings, and starts the audio
 * stream. It also configures the audio stream parameters such as sample rate,
 * block size, and channel counts.
 *
 * If an error occurs during initialization, it logs the error message
 * and deinitializes the audio system.
 */
void audio_init();

/**
 * @brief Deinitializes the audio system.
 *
 * This function stops and closes the audio stream, and then terminates
 * the PortAudio library. It logs any errors that occur during the
 * deinitialization process.
 */
void audio_deinit();

/** @} */

/**
 * @defgroup midi MIDI
 * @brief MIDI system initialization, deinitialization, and processing
 * for Workbench applications.
 *
 * This header file defines functions and macros for managing MIDI input
 * and output using the PortMidi library. It includes type definitions for MIDI
 * callback functions and a comprehensive set of MIDI message constants.
 *
 * The `midi_init` function sets up the MIDI input and output streams, while
 * the `midi_deinit` function closes the streams and terminates the MIDI system.
 * The `__midi_callback` function processes incoming MIDI messages,
 * and the `process_midi` function handles MIDI message parsing and logging.
 * @{ */
/**
 * @brief Type definition for MIDI callback functions.
 *
 * A `MidiCallback` function is called to handle incoming and outgoing
 * MIDI messages. It takes the following parameters:
 * - `input_buffer`: A pointer to the input MIDI buffer.
 * - `output_buffer`: A pointer to the output MIDI buffer.
 * - `block_size`: The number of MIDI events in the buffer.
 * - `user_data`: A pointer to user-defined data passed to the callback
 * function.
 *
 * Example usage:
 * @code
 * int my_midi_callback(const void *input_buffer, void *output_buffer, unsigned
 * long block_size, void *user_data) {
 *   const PmEvent *in_buffer = (const PmEvent*)in;
 *
 *   for (unsigned long i = 0; i < length; ++i) {
 *     PmEvent event = in_buffer[i];
 *     PmMessage message = event.message;
 *     log_i("%i", message);
 *   }
 *   return 0;
 * }
 * @endcode
 */
typedef int (*MidiCallback)(const void *, void *, unsigned long, void *);

/**
 * @brief Initializes the MIDI system.
 *
 * This function initializes the PortMidi library, sets up the MIDI input
 * and output devices based on the configuration settings, and allocates buffers
 * for MIDI events. It also starts the MIDI input and output streams if they
 * are enabled in the configuration.
 *
 * If an error occurs during initialization, it logs the error message
 * and deinitializes the MIDI system.
 */
void midi_init();

/**
 * @brief Deinitializes the MIDI system.
 *
 * This function stops the MIDI system, frees allocated buffers, and terminates
 * the PortMidi library. It logs any errors that occur during
 * the deinitialization process.
 */
void midi_deinit();

/**
 * @brief Processes incoming MIDI messages.
 *
 * This function is called by the MIDI system to process incoming MIDI messages.
 * It invokes the user-defined MIDI callback function to handle the MIDI events.
 * If there are any outgoing MIDI messages generated by the callback, it writes
 * them to the MIDI output stream.
 *
 * @param timestamp The timestamp of the MIDI event.
 * @param userData A pointer to user-defined data passed to the callback
 * function.
 */
void __midi_callback(int32_t timestamp, void *userData);
/** @} */

/**
 * @defgroup config Config
 * @brief Configuration settings and utility functions for Workbench audio
 * and MIDI applications.
 *
 * This header file defines the configuration structure and related macros
 * for managing audio and MIDI settings in Workbench applications. It includes
 * default values, flag enumerations, and function prototypes for initializing
 * and managing the configuration.
 * @{ */
#define DEFAULT_IN_CHANNELS_COUNT                                              \
  1 /**< @brief Defines the default number of audio input channels. */
#define DEFAULT_OUT_CHANNELS_COUNT                                             \
  2 /**< @brief Defines the default number of audio output channels. */

/**
 * @brief Defines a list of feature flags and their bitwise representations.
 *
 * The `FLAGS` macro is used to list various feature flags that can be enabled
 * or disabled in the configuration. Each feature flag corresponds to a specific
 * functionality (e.g., MIDI, audio) and has associated enable/disable options
 * for input and output.
 *
 * @see ENUMERATE
 * @see DEFINE
 */
#define FLAGS(FLAG)                                                            \
  FLAG(DISABLE_MIDI)                                                           \
  FLAG(DISABLE_AUDIO)                                                          \
  FLAG(DISABLE_MIDI_IN)                                                        \
  FLAG(DISABLE_MIDI_OUT)                                                       \
  FLAG(DISABLE_AUDIO_IN)                                                       \
  FLAG(DISABLE_AUDIO_OUT)

/**
 * @brief Enumeration of flag bits for feature flags.
 */
enum flag_bits { FLAGS(WORKBENCH_FLAG_ENUMERATE) };

/**
 * @brief Enumeration of feature flags.
 */
enum flags { FLAGS(WORKBENCH_FLAG_DEFINE) };

/**
 * @brief Defines configuration fields and their default values using
 * the X-macro technique.
 *
 * The `CONFIG` macro uses the X-macro technique to define a list of
 * configuration fields along with their types and default values. By passing
 * a `FIELD` macro to `CONFIG`, you can generate different pieces of code, such
 * as structure definitions, initializers, or setters, based on the same set
 * of configuration fields.
 *
 * @param FIELD A macro that takes three parameters: type, name, and default
 * value of a field.
 *
 * Example Usage:
 * To define a structure with the configuration fields:
 * @code
 * #define WORKBENCH_CONFIG_DEFINE_STRUCT(type, name, default) type name;
 * typedef struct {
 *     CONFIG(WORKBENCH_CONFIG_DEFINE_STRUCT)
 * } Config;
 * #undef WORKBENCH_CONFIG_DEFINE_STRUCT
 * @endcode
 *
 * To generate setter functions for the configuration fields:
 * @code
 * #define WORKBENCH_CONFIG_DEFINE_SETTERS(type, name, default) \
 *     void set_##name(type value) { config.name = value; }
 * CONFIG(WORKBENCH_CONFIG_DEFINE_SETTERS)
 * #undef WORKBENCH_CONFIG_DEFINE_SETTERS
 * @endcode
 */
#define CONFIG(FIELD)                                                          \
  FIELD(char *, midi_input, NULL)                                              \
  FIELD(char *, midi_output, NULL)                                             \
  FIELD(int, midi_output_latecncy, 0)                                          \
  FIELD(int, midi_buffer_size, 1024)                                           \
  FIELD(char *, audio_input, NULL)                                             \
  FIELD(char *, audio_output, NULL)                                            \
  FIELD(double, sample_rate, 44100.0)                                          \
  FIELD(uint32_t, block_size, 512U)                                            \
  FIELD(uint32_t, audio_flags, 0U)                                             \
  FIELD(int, in_channel_count, DEFAULT_IN_CHANNELS_COUNT)                      \
  FIELD(int, out_channel_count, DEFAULT_OUT_CHANNELS_COUNT)                    \
  FIELD(double, suggested_latency, -1.0)                                       \
  FIELD(uint32_t, flags, 0U)                                                   \
  FIELD(uint8_t, log_level, 4)

/*!
 * @brief Defines the structure for configuration settings.
 *
 * The `Config` structure holds all configuration settings for the application,
 * including audio and MIDI parameters. The structure is generated by expanding
 * the `CONFIG` macro with the `WORKBENCH_CONFIG_DEFINE_STRUCT` macro to declare
 * each field.
 *
 * Fields include settings for:
 * - Audio callback function
 * - MIDI callback function
 * - MIDI input and output device names
 * - Audio input and output device names
 * - etc.
 *
 * Example usage:
 * @code
 * Config config;
 * config.sample_rate = 44100.0;
 * config.in_channel_count = 2;
 * // Other configuration settings
 * @endcode
 */
typedef struct {
  AudioCallback audio_callback; /**< Audio callback function pointer */
  MidiCallback midi_callback;   /**< MIDI callback function pointer */
  void *user_data; /**< Pointer to a custom user data passed to callbacks */
  CONFIG(WORKBENCH_CONFIG_DEFINE_STRUCT)
} Config;

/*!
 * @brief Initializes the configuration with the specified audio
 * and MIDI callbacks.
 *
 * @param audio_cb The audio callback function.
 * @param midi_cb The MIDI callback function.
 * @param user_data A pointer to the cistom user data.
 * @return A pointer to the initialized configuration structure.
 */
Config *config_init(int argc, char **argv, AudioCallback audio_cb,
                    MidiCallback midi_cb, void *user_data);

void config_deinit();

/*!
 * @brief Prints the current configuration settings to the standard output.
 */
void config_print();

/*!
 * @brief Retrieves the current configuration.
 *
 * @return A pointer to the current configuration structure.
 */
Config *config_get();

CONFIG(WORKBENCH_CONFIG_DEFINE_SETTERS)

/*!
 * @brief Retrieves the log level.
 *
 * Log level determines the verbosity of the logging. The higher the value,
 * the more verbose the logging:
 * - 0: No logging
 * - 1: Error logging
 * - 2: Warning and Error logging
 * - 3: Info, Warning, and Error logging
 * - 4: Debug, Info, Warning, and Error logging
 *
 * @return An integer value from zero to four.
 */
uint8_t get_log_level();

/** @} */