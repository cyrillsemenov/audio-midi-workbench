/**
 * @file workbench_midi.h
 * @brief MIDI message constants for Workbench applications.
 *
 * This header file defines a comprehensive set of MIDI message constants for
 * managing MIDI input and output in Workbench applications. It includes
 * type definitions and macros for various MIDI message types and control
 * change messages.
 */
#pragma once

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
