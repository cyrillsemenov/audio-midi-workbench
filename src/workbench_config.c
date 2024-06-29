#include "workbench.h"

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

static void read_config_from_file(const char *filename, Config *config);
static void parse_val(Config *config, char *arg, char *val);
static void argparse(int argc, char **argv, Argument_t *arguments,
                     int *args_parsed, char **config_file);

static Config __cfg = {0};

Config *config_get() { return &__cfg; }
uint8_t get_log_level() { return __cfg.log_level; }

CONFIG(WORKBENCH_CONFIG_IMPLEMENT_SETTERS)

// void config_set_flags(uint32_t flags) { __cfg.flags |= flags; }
// void config_clear_flags(uint32_t flags) { __cfg.flags ^= flags; }

Config *config_init(int argc, char **argv, AudioCallback audio_cb,
                    MidiCallback midi_cb, void *user_data) {
  log_d("Start config");
  // Initialize with default values
  Config config = {CONFIG(WORKBENCH_CONFIG_SET_DEFAULTS)};

  // Read command line arguments
  Argument_t *cl_args = malloc(sizeof(Argument_t) * 128);
  int cl_args_num = 0;
  char *config_file = NULL;
  argparse(argc, argv, cl_args, &cl_args_num, &config_file);

  // Apply values from config file
  if (config_file)
    read_config_from_file(config_file, &config);

  // Apply values from command line argumends
  while (cl_args_num--) {
    parse_val(&config, cl_args[cl_args_num].arg, cl_args[cl_args_num].val);
  }

  // Copy config to actual struct
  memcpy(&__cfg, &config, sizeof(Config));

  if (config.log_level > 2)
    config_print();

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

/**
 * \brief Parse a value from a string and set the corresponding field in the
 * configuration.
 *
 * \param config Pointer to the Config structure.
 * \param arg The argument name.
 * \param val The argument value as a string.
 */
void parse_val(Config *config, char *arg, char *val) {
  // Allocate enough space
  void *result = malloc(sizeof(long double));

  CONFIG(WORKBENCH_CONFIG_PARSE_STRING) { log_w("Unknown argument %s", arg); }

  free(result);
}

/*!
 * \brief Reads configuration settings from a file.
 *
 * \param filename The name of the file to read the configuration from.
 * \param config A pointer to the configuration structure to populate.
 */
void read_config_from_file(const char *restrict filename, Config *config) {
  if (!filename)
    return;

  FILE *cfg = fopen(filename, "r");
  if (!cfg)
    return;

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
  return;
}

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
      CONFIG(WORKBENCH_CONFIG_PARSE_ARGUMENTS)
      else {
        log_w("Unknown arg: \"%s=%s\"", arg, val);
      }
    }
  };
}

void config_print() {
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