#include "workbench_config.h"
#include "workbench_audio.h"
#include "workbench_logger.h"
#include "workbench_midi.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef STRING_MAX
#define STRING_MAX 256
#endif

#define IS_TEXT_TYPE(x) _Generic((x), char * : 1, const char * : 1, default : 0)
#define IS_INT_TYPE(x)                                                         \
  _Generic((x), int8_t : 1, int16_t : 1, int32_t : 1, int64_t : 1, default : 0)
#define IS_UINT_TYPE(x)                                                        \
  _Generic((x), uint8_t : 1, uint16_t : 1, uint32_t : 1, uint64_t : 1,         \
           default : 0)
#define IS_FLOAT_TYPE(x) _Generic((x), float : 1, double : 1, default : 0)

static Config __cfg = {0};

Config *get_config() { return &__cfg; }
uint8_t get_log_level() { return __cfg.log_level; }

#define IMPLEMENT_SETTERS(type, field, default)                                \
  __attribute__((weak)) void config_set_##field(type field) {                  \
    __cfg.field = field;                                                       \
  }
CONFIG(IMPLEMENT_SETTERS)
#undef IMPLEMENT_SETTERS

// void config_set_flags(uint32_t flags) { __cfg.flags |= flags; }
// void config_clear_flags(uint32_t flags) { __cfg.flags ^= flags; }

Config *config_init(int argc, char **argv, AudioCallback audio_cb,
                    MidiCallback midi_cb, void *user_data) {
  log_d("Start config");
  // Initialize with default values
#define DEFAULT(type, field, default) .field = default,
  Config config = {CONFIG(DEFAULT)};
#undef DEFAULT

  char *config_file = NULL;

  // Read command line arguments
  Argument_t *cl_args = malloc(sizeof(Argument_t) * 128);
  int cl_args_num = 0;
  argparse(argc, argv, cl_args, &cl_args_num, &config_file);

  // Apply values from config file
  if (config_file)
    read_config_from_file(config_file, &config);

  // Apply values from command line argumends
  while (cl_args_num--) {
    parse_val(&config, cl_args[cl_args_num].arg, cl_args[cl_args_num].val);
  }

  // Copy config to actual struct
#define INIT(type, field, default) __cfg.field = config.field;
  CONFIG(INIT);
#undef INIT

  print_config();

  // Set pointers and callbacks
  __cfg.audio_callback = audio_cb;
  __cfg.midi_callback = midi_cb;
  __cfg.user_data = user_data;
  if (midi_cb)
    midi_init();
  if (audio_cb)
    audio_init();

  // Cleanup
  if (config_file)
    free(config_file);
  if (cl_args)
    free(cl_args);

  log_d("End");
  return &__cfg;
}

void config_deinit() {
  log_d("Start deinit");
  audio_deinit();
  midi_deinit();
  if (__cfg.midi_input)
    free(__cfg.midi_input);
  if (__cfg.midi_output)
    free(__cfg.midi_output);
  if (__cfg.user_data)
    free(__cfg.user_data);
  log_d("End deinit");
}

size_t starts_with(const char *restrict buffer, const char *prefix) {
  size_t buffer_len = strlen(buffer);
  size_t prefix_len = strlen(prefix);
  if (buffer_len < prefix_len)
    return 0;
  int res = strspn(buffer, prefix);
  if (res < prefix_len)
    return 0;
  return res;
}

void parse_val(Config *config, char *arg, char *val) {
  // Allocate enough space
  void *result = malloc(sizeof(long double));

#define PARSE(type, field, default)                                            \
  if (strcmp(arg, #field) == 0) {                                              \
    if (IS_TEXT_TYPE((type) default)) {                                        \
      *((char **)result) = strdup(val);                                        \
    } else if (IS_INT_TYPE((type) default)) {                                  \
      *((int32_t *)result) = strtoll(val, (char **)NULL, 10);                  \
    } else if (IS_UINT_TYPE((type) default)) {                                 \
      *((uint32_t *)result) = strtoull(val, (char **)NULL, 10);                \
    } else if (IS_FLOAT_TYPE((type) default)) {                                \
      *((double *)result) = strtod(val, (char **)NULL);                        \
    } else {                                                                   \
      log_w("Unknown type '%s'. Can not take '%s' from config.", #type, arg);  \
      free(result);                                                            \
      return;                                                                  \
    }                                                                          \
    config->field = *((type *)result);                                         \
  } else
  CONFIG(PARSE) { log_w("Unknown argument %s", arg); }
#undef PARSE
  free(result);
}

int read_config_from_file(const char *restrict filename, Config *config) {
  if (!filename)
    return 0;
  FILE *cfg = fopen(filename, "r");
  if (!cfg)
    return 0;

  char buffer[STRING_MAX];

  while (fgets(buffer, STRING_MAX, cfg)) {
    // Ignore comments
    if (starts_with(buffer, "#"))
      continue;
    // Trim newline characters at the end of the line
    buffer[strcspn(buffer, "\r\n")] = 0;

    log_d("> %s", buffer);

    char *arg = strtok(buffer, ": ");
    char *val = strtok(NULL, ":");

    // Trim space characters before value
    while (*val == ' ')
      val++;

    parse_val(config, arg, val);
  }
  fclose(cfg);
  return 1;
}

void argparse(int argc, char **argv, Argument_t *arguments, int *args_parsed,
              char **config_file) {
  __cfg.log_level = 3;
  // Parse command-line arguments
  for (int i = 1; i < argc;) {
    char *arg = argv[i++];
    char *val;
    int dashes = 0;
    while (*arg == '-') {
      arg++;
      dashes++;
    }
    // printf("Dashes:%i\tRaw: %s\n", dashes, arg);
    switch (dashes) {
    case 0:
      break;
    case 1:
      // handle flag
      log_d("Flags: %s", arg);
      break;
    default:
      // parse value string
      arg = strtok(arg, "=");
      val = strtok(NULL, "=");
      while (!val || (*val == '=')) {
        val = argv[i++];
      }
      if (*val == '-') {
        log_w("No value for arg '%s'", arg);
        i--;
        continue;
      }
      // Handle config file argument
      if (strcmp(arg, "config") == 0) {
        log_d("Read config from file: \"%s\"", val);
        *config_file = strdup(val);
      }
#define PARSE_ARGUMENTS(type, field, default)                                  \
  else if (strcmp(arg, #field) == 0) {                                         \
    arguments[(*args_parsed)++] = (Argument_t){.arg = arg, .val = val};        \
  }
      CONFIG(PARSE_ARGUMENTS)
#undef PARSE_ARGUMENTS
      else {
        log_w("Unknown arg: \"%s=%s\"", arg, val);
      }
    }
  };
}

void print_config() {
  printf("Config:\n"
         "  MIDI:\n"
         "    midi_input: \t%s\n"
         "    midi_output: \t%s\n"
         "    midi_output_ltc: \t%i\n"
         "    midi_buffer_size: \t%i\n"
         "  AUDIO:\n"
         "    audio_input: \t%s\n"
         "    audio_output: \t%s\n"
         "    sample_rate: \t%f\n"
         "    block_size: \t%i\n"
         "    channels(i/o): \t%i/%i\n",
         __cfg.midi_input, __cfg.midi_output, __cfg.midi_output_latecncy,
         __cfg.midi_buffer_size, __cfg.audio_input, __cfg.audio_output,
         __cfg.sample_rate, __cfg.block_size, __cfg.in_channel_count,
         __cfg.out_channel_count);
}