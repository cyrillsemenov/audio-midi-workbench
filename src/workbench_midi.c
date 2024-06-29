#include "workbench.h"

#define MIDI_TRY(x)                                                            \
  err = (x);                                                                   \
  if (err != pmNoError) {                                                      \
    log_e("%s", Pm_GetErrorText(err));                                         \
    midi_deinit();                                                             \
    return;                                                                    \
  }

static Config *cfg = NULL;
static PmStream *midi_in;
static PmStream *midi_out;
static PmEvent *midi_in_buffer;
static PmEvent *midi_out_buffer;
static PmDeviceID __midi_in_id, __midi_out_id;

void config_set_midi_input(char *midi_input) {
  free(cfg->midi_input);
  cfg->midi_input = strdup(midi_input);
}

void config_set_midi_output(char *midi_output) {
  free(cfg->midi_output);
  cfg->midi_output = strdup(midi_output);
}

static bool midi_device_find(char *pattern, bool input) {
  if (pattern == NULL)
    return false;
  for (int i = 0; i < Pm_CountDevices(); i++) {
    const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
    if ((info->input == input) && (strcmp(info->name, pattern) == 0)) {
      *(input ? &__midi_in_id : &__midi_out_id) = i;
      return true;
    }
  }
  return false;
}

void midi_init() {
  log_d("Midi init start");
  PmError err;
  cfg = config_get();
  MIDI_TRY(Pm_Initialize());
  midi_in_buffer = malloc(sizeof(PmEvent) * cfg->midi_buffer_size);
  midi_out_buffer = malloc(sizeof(PmEvent) * cfg->midi_buffer_size);

  if (!cfg->audio_callback)
    Pt_Start(1, __midi_callback, cfg->user_data);
  if (!(cfg->flags & DISABLE_MIDI_IN)) {
    if (!cfg->midi_input) {
      __midi_in_id = Pm_GetDefaultInputDeviceID();
    } else if (!midi_device_find(cfg->midi_input, true)) {
      __midi_in_id = Pm_GetDefaultInputDeviceID();
      log_w("Input midi device \"%s\" was not found. Using default instead.",
            cfg->midi_input);
    }

    MIDI_TRY(Pm_OpenInput(&midi_in, __midi_in_id, NULL, cfg->midi_buffer_size,
                          NULL, NULL));
    MIDI_TRY(Pm_SetFilter(midi_in, PM_FILT_ACTIVE));
  }
  if (!(cfg->flags & DISABLE_MIDI_OUT)) {
    if (!cfg->midi_output) {
      __midi_out_id = Pm_GetDefaultOutputDeviceID();
    } else if (!midi_device_find(cfg->midi_output, false)) {
      __midi_out_id = Pm_GetDefaultOutputDeviceID();
      log_w("Output midi device \"%s\" was not found. Using default instead.",
            cfg->midi_output);
    }

    MIDI_TRY(Pm_OpenOutput(&midi_out, __midi_out_id, NULL,
                           cfg->midi_buffer_size, NULL, NULL,
                           cfg->midi_output_latecncy));
  }
  log_d("Midi init finish");
}

#define PRINT_ERROR(x)                                                         \
  err = (x);                                                                   \
  if (err != pmNoError) {                                                      \
    log_e("%s", Pm_GetErrorText(err));                                         \
  }

void midi_deinit() {
  PmError err;
#if AUDIO_ENABLED == 0
  Pt_Stop();
#endif
  free(midi_in_buffer);
  free(midi_out_buffer);
  config_set_flags(DISABLE_MIDI);
  PRINT_ERROR(Pm_Close(midi_in));
  PRINT_ERROR(Pm_Close(midi_out));
  PRINT_ERROR(Pm_Terminate());
}

void __midi_callback(int32_t timestamp, void *userData) {
  (void)timestamp;
  Config *cfg = config_get();
  if (!cfg->midi_callback)
    return;
  int in_queue_length = Pm_Read(midi_in, midi_in_buffer, cfg->midi_buffer_size);

  int out_queue_length = cfg->midi_callback(midi_in_buffer, midi_out_buffer,
                                            in_queue_length, userData);
  if (out_queue_length > 0) {
    Pm_Write(midi_out, midi_out_buffer, out_queue_length);
  }
}

bool in_sysex = false;

char val_format[] = "    Val %d\n";
char vel_format[] = "    Vel %d\n";
char nib_to_hex[] = "0123456789ABCDEF";

static int put_pitch(int p) {
  char result[8];
  static char *ptos[] = {"c",  "cs", "d",  "ef", "e",  "f",
                         "fs", "g",  "gs", "a",  "bf", "b"};
  /* note octave correction below */
  sprintf(result, "%s%d", ptos[p % 12], (p / 12) - 1);
  printf("%s", result);
  return strlen(result);
}

static void showbytes(uint8_t status, uint8_t data1, uint8_t data2, int len) {
  PmMessage data = Pm_Message(status, data1, data2);
  int count = 0;
  int i;
  for (i = 0; i < len; i++) {
    putchar(nib_to_hex[(data >> 4) & 0xF]);
    putchar(nib_to_hex[data & 0xF]);
    count += 2;
    if (count > 72) {
      putchar('.');
      putchar('.');
      putchar('.');
      break;
    }
    data >>= 8;
  }
  putchar(' ');
}

__attribute__((weak)) void __process_midi(uint8_t status, uint8_t data1,
                                          uint8_t data2) {
  /* NOTE: This function Should not be modified, when the callback is needed,
the process_midi could be implemented in the user file
*/
  int command = status & MIDI_CODE_MASK;
  int chan = status & MIDI_CHN_MASK;
  int len;

  if (in_sysex || status == MIDI_SYSEX) {
    int i;
    PmMessage data_copy = Pm_Message(status, data1, data2);
    ;
    in_sysex = true;
    /* look for MIDI_EOX in first 3 bytes
     * if realtime messages are embedded in sysex message, they will
     * be printed as if they are part of the sysex message
     */
    for (i = 0; (i < 4) && ((data_copy & 0xFF) != MIDI_EOX); i++)
      data_copy >>= 8;
    if (i < 4) {
      in_sysex = false;
      i++; /* include the EOX byte in output */
    }
    showbytes(status, data1, data2, i);
    log_i("System Exclusive\n");
  } else if (command == MIDI_ON_NOTE && data2 != 0) {
    showbytes(status, data1, data2, 3);
    log_i("NoteOn  Chan %2d Key %3d ", chan, data1);
    // len = put_pitch(data1);
    // log_i(vel_format + len, data2);
  } else if ((command == MIDI_ON_NOTE || command == MIDI_OFF_NOTE)) {
    showbytes(status, data1, data2, 3);
    log_i("NoteOff Chan %2d Key %3d ", chan, data1);
    // len = put_pitch(data1);
    // log_i(vel_format + len, data2);

  } else if (command == MIDI_CH_PROGRAM) {
    showbytes(status, data1, data2, 2);
    log_i("  ProgChg Chan %2d Prog %2d\n", chan, data1 + 1);
  } else if (command == MIDI_CTRL) {
    /* controls 121 (MIDI_RESET_CONTROLLER) to 127 are channel
     * mode messages. */
    if (data1 < MIDI_ALL_SOUND_OFF) {
      showbytes(status, data1, data2, 3);
      log_i("CtrlChg Chan %2d Ctrl %2d Val %2d\n", chan, data1, data2);

    } else {
      showbytes(status, data1, data2, 3);
      switch (data1) {
      case MIDI_ALL_SOUND_OFF:
        log_i("All Sound Off, Chan %2d\n", chan);
        break;
      case MIDI_RESET_CONTROLLERS:
        log_i("Reset All Controllers, Chan %2d\n", chan);
        break;
      case MIDI_LOCAL:
        log_i("LocCtrl Chan %2d %s\n", chan, data2 ? "On" : "Off");
        break;
      case MIDI_ALL_OFF:
        log_i("All Off Chan %2d\n", chan);
        break;
      case MIDI_OMNI_OFF:
        log_i("OmniOff Chan %2d\n", chan);
        break;
      case MIDI_OMNI_ON:
        log_i("Omni On Chan %2d\n", chan);
        break;
      case MIDI_MONO_ON:
        log_i("Mono On Chan %2d\n", chan);
        if (data2) {
          log_i(" to %d received channels\n", data2);
        } else {
          log_i(" to all received channels\n");
        }
        break;
      case MIDI_POLY_ON:
        log_i("Poly On Chan %2d\n", chan);
        break;
      }
    }
  } else if (command == MIDI_POLY_TOUCH) {
    showbytes(status, data1, data2, 3);
    log_i("P.Touch Chan %2d Key %2d ", chan, data1);
    // len = put_pitch(data1);
    // printf(val_format + len, data2);
  } else if (command == MIDI_TOUCH) {
    showbytes(status, data1, data2, 2);
    log_i("  A.Touch Chan %2d Val %2d\n", chan, data1);
  } else if (command == MIDI_BEND) {
    showbytes(status, data1, data2, 3);
    log_i("P.Bend  Chan %2d Val %2d\n", chan, (data1 + (data2 << 7)));
  } else if (status == MIDI_SONG_POINTER) {
    showbytes(status, data1, data2, 3);
    log_i("    Song Position %d\n", (data1 + (data2 << 7)));
  } else if (status == MIDI_SONG_SELECT) {
    showbytes(status, data1, data2, 2);
    log_i("    Song Select %d\n", data1);
  } else if (status == MIDI_TUNE_REQ) {
    showbytes(status, data1, data2, 1);
    log_i("    Tune Request\n");
  } else if (status == MIDI_Q_FRAME) {
    showbytes(status, data1, data2, 2);
    log_i("    Time Code Quarter Frame Type %d Values %d\n",
          (data1 & 0x70) >> 4, data1 & 0xf);
  } else if (status == MIDI_START) {
    showbytes(status, data1, data2, 1);
    log_i("    Start\n");
  } else if (status == MIDI_CONTINUE) {
    showbytes(status, data1, data2, 1);
    log_i("    Continue\n");
  } else if (status == MIDI_STOP) {
    showbytes(status, data1, data2, 1);
    log_i("    Stop\n");
  } else if (status == MIDI_SYS_RESET) {
    showbytes(status, data1, data2, 1);
    log_i("    System Reset\n");
  } else if (status == MIDI_TIME_CLOCK) {
    showbytes(status, data1, data2, 1);
    log_i("    Clock\n");
  } else if (status == MIDI_ACTIVE_SENSING) {
    showbytes(status, data1, data2, 1);
    log_i("    Active Sensing\n");
  } else {
    showbytes(status, data1, data2, 3);
  }
  fflush(stdout);
}