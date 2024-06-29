/**
 * \example main.c
 * \brief An example of recording and playback using PortAudio.
 *
 * This file contains an example implementation of a simple audio recorder and
 * player using Workbench. The example demonstrates how to:
 * - Record audio input for a specified duration.
 * - Playback the recorded audio. ('p')
 * - Playback the recorded audio in reverse. ('r')
 * - Playback the recorded audio in loop. ('l')
 * - Playback the recorded audio in loop back and forth. ('b')
 *
 * The example includes functions to start and stop recording and playback,
 * handle audio processing in a callback function, and display progress using
 * a countdown timer.
 */
#include "workbench.h"
#include <stdio.h>
#include <stdlib.h>

/// Define ANSI escape codes to move cursor up and clear the line
#define SAME_LINE "\033[A\033[2K\r"

/// Maximum recording time in seconds and samples
#define REC_MAX_SEC 10
#define REC_MAX_SAMPLES (cfg->sample_rate * REC_MAX_SEC)

/// Animation update interval in milliseconds
#define ANIM_TIMEOUT 100
#define ANIM_WIDTH 20
#define ANIM_TEMPLATE "%.*s%.*s"
#define ANIM_ARGS                                                              \
  ANIM_WIDTH - progress, "********************", progress,                     \
      "...................."

/// Global configuration pointer
Config *cfg;

/// Recorder structure to hold recording/playback state and buffer
typedef struct {
  AudioSample_t *buffer; /**< Audio buffer to store samples */
  unsigned long elapsed; /**< Number of samples processed */
  unsigned long length;  /**< Total number of samples recorded */
  bool recording;        /**< Flag to indicate if recording is active */
  bool playing;          /**< Flag to indicate if playback is active */
  bool reverse;          /**< Flag to indicate if playback is in reverse */
  bool loop;             /**< Flag to indicate if playback is in reverse */
  bool backandforth;     /**< Play bask and forth endlessly */
} Recorder;

// Audio callback function to handle recording and playback
void audio_cb(const void *input_buffer, void *output_buffer,
              unsigned long block_size, void *user_data);

/// Start recording by initializing the recorder state and buffer
void start_record(Recorder *rec);

/// Stop recording and update the recorder state
void stop_record(Recorder *rec);

// Start playback by initializing the recorder state
void start_playback(Recorder *rec);

// Stop playback and reset the recorder state
void stop_playback(Recorder *rec);

/// Countdown function to update recording/playback progress
void countdoun(PtTimestamp timestamp, void *user_data);

int main(int argc, char **argv) {
  PtError err;
  Recorder rec = {0};
  cfg = config_init(argc, argv, audio_cb, NULL, &rec);
  rec.buffer = malloc(sizeof(AudioSample_t) * REC_MAX_SAMPLES);

  printf("Press Enter to start recording (Press 'q' to quit)...\n");

  int run = 1;
  while (run) {
    switch (getchar()) {
    case 'q':
      run = 0; // Exit the loop if 'q' is pressed
      break;
    case 'p':
      start_playback(&rec); // Start playback
      break;
    case 'l':
      rec.loop = true;
      start_playback(&rec); // Start playback
      break;
    case 'b':
      rec.loop = true;
      rec.backandforth = true;
      start_playback(&rec); // Start playback
      break;
    case 'r':
      rec.reverse = true;
      start_playback(&rec); // Start reverse playback
      break;
    case '\n':
      if (!rec.playing && !rec.recording) {
        start_record(&rec); // Start recording
      } else if (rec.recording) {
        stop_record(&rec); // Stop recording
      } else if (rec.playing) {
        stop_playback(&rec); // Stop playback
      }
      break;
    default:
      printf(SAME_LINE); // Clear the line for other keys
    }
    fflush(stdin);
  }
  free(rec.buffer); // Free the allocated buffer
  config_deinit();
  return 0;
}

void audio_cb(const void *input_buffer, void *output_buffer,
              unsigned long block_size, void *user_data) {
  Recorder *rec = (Recorder *)user_data;
  if (rec == NULL) {
    return;
  }
  const AudioSample_t *in = (const float *)input_buffer;
  AudioSample_t *out = (AudioSample_t *)output_buffer;
  if (rec->recording) {
    // Handle recording
    if (rec->length >= REC_MAX_SAMPLES) {
      stop_record(rec); // Handle recording
      return;
    }
    // Copy input samples to buffer
    for (unsigned long i = 0; i < block_size; ++i) {
      rec->buffer[rec->length + i] = in[i];
    }
    rec->length += block_size; // Update the length of recorded samples
  } else if (rec->playing) {
    // Handle playback
    if (rec->elapsed >= rec->length) {
      if (!rec->loop) {
        stop_playback(rec); // Stop playback if all samples have been played
        return;
      }
      if (rec->backandforth) {
        rec->reverse = !rec->reverse;
      }
      rec->elapsed = 0;
    }
    unsigned long idx =
        rec->reverse ? rec->length - rec->elapsed : rec->elapsed;
    // Copy buffer samples to output
    for (unsigned long i = 0; i < block_size; i++) {
      idx += rec->reverse ? -1 : 1;
      int channels = cfg->out_channel_count;
      while (channels--) {
        *out++ = rec->buffer[idx];
      }
    }
    rec->elapsed += block_size; // Update elapsed time
  } else {
    // Clear buffer if nothing to play
    for (unsigned long i = 0; i < block_size * cfg->out_channel_count; ++i) {
      *out++ = 0;
    }
  }
}

void start_record(Recorder *rec) {
  printf(SAME_LINE "Record started! Press Enter again to stop it...\n");
  // Clear the buffer
  for (unsigned long i = 0; i < REC_MAX_SAMPLES; i++) {
    rec->buffer[i] = 0;
  }
  rec->length = 0;  // Reset the length of recorded samples
  rec->elapsed = 0; // Reset elapsed time
  rec->playing = false;
  rec->recording = true;
  Pt_Stop();
  Pt_Start(ANIM_TIMEOUT, countdoun, rec); // Restart the countdown timer
}

void stop_record(Recorder *rec) {
  Pt_Stop(); // Stop the timer
  rec->recording = false;
  printf("Record finished! Press 'p' to listen it.\n");
}

void start_playback(Recorder *rec) {
  printf("Playback started! Press Enter to stop it...\n");
  rec->recording = false;
  rec->playing = true;
  Pt_Stop();
  Pt_Start(ANIM_TIMEOUT, countdoun, rec); // Restart the countdown timer
}

void stop_playback(Recorder *rec) {
  Pt_Stop(); // Stop the timer
  rec->playing = false;
  rec->reverse = false;
  rec->loop = false;
  rec->backandforth = false;
  rec->elapsed = 0; // Reset elapsed time
  printf("Playback finished! Press 'p' listen again or Enter to start "
         "new recording.\n");
}

void countdoun(PtTimestamp timestamp, void *user_data) {
  Recorder *rec = (Recorder *)user_data;
  int progress;
  if (rec->recording) {
    // Calculate and display recording progress
    progress = ((float)rec->length / (float)REC_MAX_SAMPLES) * ANIM_WIDTH;
    printf("%lu seconds recorded   " ANIM_TEMPLATE "\n" SAME_LINE,
           (unsigned long)((double)rec->length / cfg->sample_rate), ANIM_ARGS);
  } else if (rec->playing) {
    // Calculate and display playback progress
    progress = ((float)rec->elapsed / (float)rec->length) * ANIM_WIDTH;
    printf("%lu seconds played     " ANIM_TEMPLATE "\n" SAME_LINE,
           (unsigned long)((double)rec->elapsed / cfg->sample_rate), ANIM_ARGS);
  }
}
