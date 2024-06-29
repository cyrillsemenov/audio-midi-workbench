/** \example midi_monitor.c
 * @brief Example of using the Workbench library to monitor MIDI messages.
 *
 * This dead simple rudimentary example demonstrates how to use the Workbench
 * library to initialize the MIDI system and monitor incoming MIDI messages.
 * It is a simplified version of the MIDI monitor example provided in the
 * PortMidi library
 * (https://github.com/PortMidi/portmidi/blob/master/pm_test/mm.c). The example
 * configures the system to use a custom MIDI callback function that processes
 * and prints various types of MIDI messages such as Note On, Note Off, Control
 * Change, Program Change, and System Exclusive messages.
 *
 * The example includes:
 * - Initialization of the configuration and MIDI system.
 * - A custom MIDI callback function (`midi_cb`) to handle and print MIDI
 * messages.
 * - A main loop that keeps the application running to continuously monitor MIDI
 * messages.
 *
 * The main function initializes the configuration with the custom MIDI callback
 * and enters an infinite loop to keep the application running. This setup
 * allows the application to continuously monitor and print incoming MIDI
 * messages.
 */
#include "workbench.h"
#include <stdio.h>

Config *cfg;

static bool in_sysex = false; /**< Array of pitch names for MIDI note values. */

/**
 * @brief Flag indicating if a System Exclusive message is being processed.
 */
static char *ptos[] = {"C",  "C#", "D",  "D#", "E",  "F",
                       "F#", "G",  "G#", "A",  "A#", "B"};

/**
 * @brief Prints the pitch name and octave for a given MIDI note value.
 *
 * @param p MIDI note value.
 */
static void print_pitch(int p) { printf("%s%d", ptos[p % 12], (p / 12) - 1); }

/**
 * @brief Prints the velocity value for a given MIDI note value.
 *
 * @param p MIDI note value.
 * @param v Velocity value.
 */
static void print_velocity(int p, int v) {
  printf(" %sVel: %d\n", strlen(ptos[p % 12]) == 1 ? " " : "", v);
}

/**
 * @brief MIDI callback function to handle and print incoming MIDI messages.
 *
 * @param in Pointer to the input MIDI buffer.
 * @param out Pointer to the output MIDI buffer (unused).
 * @param length Number of MIDI events in the input buffer.
 * @param user_data Pointer to user-defined data (unused).
 *
 * @return Always returns 0 as there is no outgoing midi messages created.
 */
static int midi_cb(const void *in, void *out, unsigned long length,
                   void *user_data) {
  const PmEvent *in_buffer = (const PmEvent *)in;

  for (unsigned long i = 0; i < length; ++i) {
    PmEvent event = in_buffer[i];
    PmMessage message = event.message;
    int status = Pm_MessageStatus(message);
    int data1 = Pm_MessageData1(message);
    int data2 = Pm_MessageData2(message);

    int command = status & MIDI_CODE_MASK;
    int chan = status & MIDI_CHN_MASK;

    printf("%x\t", message);

    if (in_sysex || status == MIDI_SYSEX) {
      int i;
      PmMessage data_copy = Pm_Message(status, data1, data2);
      in_sysex = true;
      for (i = 0; (i < 4) && ((data_copy & 0xFF) != MIDI_EOX); i++)
        data_copy >>= 8;
      if (i < 4) {
        in_sysex = false;
        i++;
      }
      printf("System Exclusive\n");
    } else if (command == MIDI_ON_NOTE && data2 != 0) {
      printf("NoteOn  Chan %2d Key %3d ", chan, data1);
      print_pitch(data1);
      print_velocity(data1, data2);
    } else if ((command == MIDI_ON_NOTE || command == MIDI_OFF_NOTE)) {
      printf("NoteOff Chan %2d Key %3d ", chan, data1);
      print_pitch(data1);
      print_velocity(data1, data2);
    } else if (command == MIDI_CH_PROGRAM) {
      printf("ProgChg Chan %2d Prog %2d\n", chan, data1 + 1);
    } else if (command == MIDI_CTRL) {
      /* controls 121 (MIDI_RESET_CONTROLLER) to 127 are channel
       * mode messages. */
      if (data1 < MIDI_ALL_SOUND_OFF) {
        printf("CtrlChg Chan %2d Ctrl %2d Val %2d\n", chan, data1, data2);
      } else {
        switch (data1) {
        case MIDI_ALL_SOUND_OFF:
          printf("All Sound Off, Chan %2d\n", chan);
          break;
        case MIDI_RESET_CONTROLLERS:
          printf("Reset All Controllers, Chan %2d\n", chan);
          break;
        case MIDI_LOCAL:
          printf("LocCtrl Chan %2d %s\n", chan, data2 ? "On" : "Off");
          break;
        case MIDI_ALL_OFF:
          printf("All Off Chan %2d\n", chan);
          break;
        case MIDI_OMNI_OFF:
          printf("OmniOff Chan %2d\n", chan);
          break;
        case MIDI_OMNI_ON:
          printf("Omni On Chan %2d\n", chan);
          break;
        case MIDI_MONO_ON:
          printf("Mono On Chan %2d", chan);
          if (data2) {
            printf(" to %d received channels\n", data2);
          } else {
            printf(" to all received channels\n");
          }
          break;
        case MIDI_POLY_ON:
          printf("Poly On Chan %2d\n", chan);
          break;
        }
      }
    } else if (command == MIDI_POLY_TOUCH) {
      printf("P.Touch Chan %2d Key %2d ", chan, data1);
      print_pitch(data1);
      printf("\n");
    } else if (command == MIDI_TOUCH) {
      printf("A.Touch Chan %2d Val %2d\n", chan, data1);
    } else if (command == MIDI_BEND) {
      printf("P.Bend  Chan %2d Val %2d\n", chan, (data1 + (data2 << 7)));
    } else if (status == MIDI_SONG_POINTER) {
      printf("    Song Position %d\n", (data1 + (data2 << 7)));
    } else if (status == MIDI_SONG_SELECT) {
      printf("    Song Select %d\n", data1);
    } else if (status == MIDI_TUNE_REQ) {
      printf("    Tune Request\n");
    } else if (status == MIDI_Q_FRAME) {
      printf("    Time Code Quarter Frame Type %d Values %d\n",
             (data1 & 0x70) >> 4, data1 & 0xf);
    } else if (status == MIDI_START) {
      printf("    Start\n");
    } else if (status == MIDI_CONTINUE) {
      printf("    Continue\n");
    } else if (status == MIDI_STOP) {
      printf("    Stop\n");
    } else if (status == MIDI_SYS_RESET) {
      printf("    System Reset\n");
    } else if (status == MIDI_TIME_CLOCK) {
      printf("    Clock\n");
    } else if (status == MIDI_ACTIVE_SENSING) {
      printf("    Active Sensing\n");
    }
  }
  return 0;
}

int main(int argc, char **argv) {
  cfg = config_init(argc, argv, NULL, &midi_cb, NULL);
  while (true) {
    Pa_Sleep(1000);
  }
  config_deinit();
  return 0;
}