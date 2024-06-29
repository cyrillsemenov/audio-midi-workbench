/**
 * @file workbench_midi.h
 * @brief MIDI system initialization, deinitialization, and processing for
 * Workbench applications.
 *
 * This header file defines functions and macros for managing MIDI input and
 * output using the PortMidi library. It includes type definitions for MIDI
 * callback functions and a comprehensive set of MIDI message constants.
 *
 * The `midi_init` function sets up the MIDI input and output streams, while the
 * `midi_deinit` function closes the streams and terminates the MIDI system. The
 * `__midi_callback` function processes incoming MIDI messages, and the
 * `process_midi` function handles MIDI message parsing and logging.
 *
 * Example usage:
 * @code
 * #include "workbench_midi.h"
 *
 * void my_midi_callback(const void *input_buffer, void *output_buffer, unsigned
 * long block_size, void *user_data) {
 *     // Process MIDI data here
 * }
 *
 * int main() {
 *     midi_init();
 *     // Application code here
 *     midi_deinit();
 *     return 0;
 * }
 * @endcode
 */
#pragma once

#include "portmidi.h"
#include "porttime.h"
#include <stdint.h>

#define MIDI_CODE_MASK                                                         \
  0xf0 /**< @brief Mask to extract the MIDI message type. */
#define MIDI_CHN_MASK                                                          \
  0x0f /**< @brief Mask to extract the MIDI channel number. */

// #define MIDI_REALTIME 0xf8 /**< @brief Realtime MIDI message. */
// #define MIDI_CHAN_MODE 0xfa /**< @brief Channel mode MIDI message. */

#define MIDI_OFF_NOTE 0x80 /**< @brief MIDI Note Off message. */
#define MIDI_ON_NOTE 0x90  /**< @brief MIDI Note On message. */
#define MIDI_POLY_TOUCH                                                        \
  0xa0 /**< @brief MIDI Polyphonic Key Pressure (Aftertouch) message. */
#define MIDI_CTRL 0xb0       /**< @brief MIDI Control Change message. */
#define MIDI_CH_PROGRAM 0xc0 /**< @brief MIDI Program Change message. */
#define MIDI_TOUCH                                                             \
  0xd0 /**< @brief MIDI Channel Pressure (Aftertouch) message. */
#define MIDI_BEND 0xe0 /**< @brief MIDI Pitch Bend Change message. */

#define MIDI_SYSEX 0xf0   /**< @brief MIDI System Exclusive (Sysex) message. */
#define MIDI_Q_FRAME 0xf1 /**< @brief MIDI Time Code Quarter Frame message. */
#define MIDI_SONG_POINTER                                                      \
  0xf2                        /**< @brief MIDI Song Position Pointer message. */
#define MIDI_SONG_SELECT 0xf3 /**< @brief MIDI Song Select message. */
#define MIDI_TUNE_REQ 0xf6    /**< @brief MIDI Tune Request message. */
#define MIDI_EOX 0xf7        /**< @brief MIDI End of Exclusive (EOX) message. */
#define MIDI_TIME_CLOCK 0xf8 /**< @brief MIDI Timing Clock message. */
#define MIDI_START 0xfa      /**< @brief MIDI Start message. */
#define MIDI_CONTINUE 0xfb   /**< @brief MIDI Continue message. */
#define MIDI_STOP 0xfc       /**< @brief MIDI Stop message. */
#define MIDI_ACTIVE_SENSING 0xfe /**< @brief MIDI Active Sensing message. */
#define MIDI_SYS_RESET 0xff      /**< @brief MIDI System Reset message. */

#define MIDI_ALL_SOUND_OFF 0x78 /**< @brief MIDI All Sound Off message. */
#define MIDI_RESET_CONTROLLERS                                                 \
  0x79                     /**< @brief MIDI Reset All Controllers message. */
#define MIDI_LOCAL 0x7a    /**< @brief MIDI Local Control On/Off message. */
#define MIDI_ALL_OFF 0x7b  /**< @brief MIDI All Notes Off message. */
#define MIDI_OMNI_OFF 0x7c /**< @brief MIDI Omni Mode Off message. */
#define MIDI_OMNI_ON 0x7d  /**< @brief MIDI Omni Mode On message. */
#define MIDI_MONO_ON 0x7e  /**< @brief MIDI Mono Mode On message. */
#define MIDI_POLY_ON 0x7f  /**< @brief MIDI Poly Mode On message. */

/**
 * @brief Type definition for MIDI callback functions.
 *
 * A `MidiCallback` function is called to handle incoming and outgoing MIDI
 * messages. It takes the following parameters:
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
 * This function initializes the PortMidi library, sets up the MIDI input and
 * output devices based on the configuration settings, and allocates buffers for
 * MIDI events. It also starts the MIDI input and output streams if they are
 * enabled in the configuration.
 *
 * If an error occurs during initialization, it logs the error message and
 * deinitializes the MIDI system.
 */
void midi_init();

/**
 * @brief Deinitializes the MIDI system.
 *
 * This function stops the MIDI system, frees allocated buffers, and terminates
 * the PortMidi library. It logs any errors that occur during the
 * deinitialization process.
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

void __process_midi(uint8_t status, uint8_t data1, uint8_t data2);