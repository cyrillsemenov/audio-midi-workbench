/**
 * @file workbench_config.h
 * @brief Configuration settings and utility functions for Workbench audio and
 * MIDI applications.
 *
 * This header file defines the configuration structure and related macros for
 * managing audio and MIDI settings in Workbench applications. It includes
 * default values, flag enumerations, and function prototypes for initializing
 * and managing the configuration.
 */
#pragma once

#include "portaudio.h"
#include "workbench_audio.h"
#include "workbench_midi.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @name Config
 * @{ */
#ifndef MIDI_ENABLED
#define MIDI_ENABLED 1 /**< @brief Enables MIDI functionality. */
#endif
#ifndef MIDI_IN_ENABLED
#define MIDI_IN_ENABLED 1 /**< @brief Enables MIDI input. */
#endif
#ifndef MIDI_OUT_ENABLED
#define MIDI_OUT_ENABLED 1 /**< @brief Enables MIDI output. */
#endif
#ifndef AUDIO_ENABLED
#define AUDIO_ENABLED 1 /**< @brief Enables audio functionality. */
#endif
#ifndef AUDIO_IN_ENABLED
#define AUDIO_IN_ENABLED 1 /**< @brief Enables audio input. */
#endif
#ifndef AUDIO_OUT_ENABLED
#define AUDIO_OUT_ENABLED 1 /**< @brief Enables audio output. */
#endif

#define DEFAULT_SAMPLE_FORMAT                                                  \
  paFloat32 /**< @brief Defines the default audio sample format. */
#define DEFAULT_IN_CHANNELS_COUNT                                              \
  1 /**< @brief Defines the default number of audio input channels. */
#define DEFAULT_OUT_CHANNELS_COUNT                                             \
  2 /**< @brief Defines the default number of audio output channels. */
/** @} */

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
 * @brief Enumerates flag bits.
 *
 * The `ENUMERATE` macro appends `_BIT` to each flag name, creating an
 * enumeration of flag bits. This enumeration assigns a unique index to each
 * flag, which can be used to generate bitwise representations.
 *
 * @param FLAG The macro parameter representing a feature flag.
 */
#define ENUMERATE(FLAG) FLAG##_BIT,
/**
 * @brief Enumeration of flag bits for feature flags.
 */
enum flag_bits { FLAGS(ENUMERATE) };
#undef ENUMERATE

/**
 * @brief Defines actual values for feature flags.
 *
 * The `DEFINE` macro assigns bitwise values to each feature flag by shifting
 * `1` left by the corresponding bit index from the `flag_bits` enumeration.
 *
 * @param FLAG The macro parameter representing a feature flag.
 */
#define DEFINE(FLAG) FLAG = (1 << FLAG##_BIT),
/**
 * @brief Enumeration of feature flags.
 */
enum flags { FLAGS(DEFINE) };
#undef DEFINE

/**
 * @brief Defines configuration fields and their default values using the
 * X-macro technique.
 *
 * The `CONFIG` macro uses the X-macro technique to define a list of
 * configuration fields along with their types and default values. By passing a
 * `FIELD` macro to `CONFIG`, you can generate different pieces of code, such as
 * structure definitions, initializers, or setters, based on the same set of
 * configuration fields.
 *
 * @param FIELD A macro that takes three parameters: type, name, and default
 * value of a field.
 *
 * Example Usage:
 * To define a structure with the configuration fields:
 * @code
 * #define DEFINE_STRUCT(type, name, default) type name;
 * typedef struct {
 *     CONFIG(DEFINE_STRUCT)
 * } Config;
 * #undef DEFINE_STRUCT
 * @endcode
 *
 * To generate setter functions for the configuration fields:
 * @code
 * #define DEFINE_SETTERS(type, name, default) \
 *     void set_##name(type value) { config.name = value; }
 * CONFIG(DEFINE_SETTERS)
 * #undef DEFINE_SETTERS
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
  FIELD(void *, user_data, NULL)                                               \
  FIELD(uint8_t, log_level, 4)

/**
 * @brief Macro to define a field in the configuration structure.
 *
 * The `DEFINE_STRUCT` macro specifies how each field in the configuration
 * structure should be declared.
 *
 * This macro is intended to be passed to the `CONFIG` macro to generate the
 * structure definition.
 *
 * @param type The data type of the field.
 * @param field The name of the field.
 * @param default The default value of the field (not used here).
 */
#define DEFINE_STRUCT(type, field, default) type field;
/*!
 * @brief Defines the structure for configuration settings.
 *
 * The `Config` structure holds all configuration settings for the application,
 * including audio and MIDI parameters. The structure is generated by expanding
 * the `CONFIG` macro with the `DEFINE_STRUCT` macro to declare each field.
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
  CONFIG(DEFINE_STRUCT)
} Config;
#undef DEFINE_STRUCT

/**
 * @brief Macro to define setter function declarations for configuration fields.
 *
 * The `DEFINE_SETTERS` macro generates the declaration of setter functions
 * for each field in the configuration structure. Each generated function allows
 * setting the value of a specific configuration field.
 *
 * This macro is intended to be passed to the `CONFIG` macro to generate
 * the setter function declarations.
 *
 * @param type The data type of the field.
 * @param field The name of the field.
 * @param default The default value of the field (not used in this macro, but
 * included for consistency).
 */
#define DEFINE_SETTERS(type, field, default)                                   \
  void config_set_##field(type field);
CONFIG(DEFINE_SETTERS)
#undef DEFINE_SETTERS

/*!
 * @brief Initializes the configuration with the specified audio and
 * MIDI callbacks.
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
 * @brief Retrieves the current configuration.
 *
 * @return A pointer to the current configuration structure.
 */
Config *get_config();

/*!
 * @brief Retrieves the log level.
 *
 * Log level determines the verbosity of the logging. The higher the value, the
 * more verbose the logging:
 * - 0: No logging
 * - 1: Error logging
 * - 2: Warning and Error logging
 * - 3: Info, Warning, and Error logging
 * - 4: Debug, Info, Warning, and Error logging
 *
 * @return An integer value from zero to four.
 */
uint8_t get_log_level();

/*!
 * @brief Prints the current configuration settings to the standard output.
 */
void print_config();

/*!
 * @brief Reads configuration settings from a file.
 *
 * @param filename The name of the file to read the configuration from.
 * @param config A pointer to the configuration structure to populate.
 * @return An integer indicating success (0) or failure (non-zero).
 */
int read_config_from_file(const char *filename, Config *config);

/**
 * \struct Argument_t
 * \brief Structure to store command-line arguments.
 *
 * This structure holds a command-line argument and its corresponding value.
 */
typedef struct {
  char *arg; /**< Argument name. */
  char *val; /**< Argument value. */
} Argument_t;

/**
 * \brief Parse a value from a string and set the corresponding field in the
 * configuration.
 *
 * \param config Pointer to the Config structure.
 * \param arg The argument name.
 * \param val The argument value as a string.
 */
void parse_val(Config *config, char *arg, char *val);

/**
 * \brief Parse command-line arguments and store them in the arguments array.
 *
 * \param argc The number of command-line arguments.
 * \param argv The array of command-line argument strings.
 * \param arguments The array to store parsed arguments.
 * \param args_parsed Pointer to the number of parsed arguments.
 * \param config_file Pointer to the configuration file path.
 */
void argparse(int argc, char **argv, Argument_t *arguments, int *args_parsed,
              char **config_file);