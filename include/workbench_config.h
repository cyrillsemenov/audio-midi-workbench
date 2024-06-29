/**
 * @file workbench_config.h
 * @brief Configuration settings and macros for Workbench audio and MIDI
 * applications.
 *
 * This header file defines the configuration structure and related macros for
 * managing audio and MIDI settings in Workbench applications. It includes
 * default values, flag enumerations, and macros for initializing and managing
 * the configuration.
 */
#pragma once

/**
 * @brief Macro to define a field in the configuration structure.
 *
 * The `WORKBENCH_CONFIG_DEFINE_STRUCT` macro specifies how each field
 * in the configuration structure should be declared.
 *
 * This macro is intended to be passed to the `CONFIG` macro to generate
 * the structure definition.
 *
 * @param type The data type of the field.
 * @param field The name of the field.
 * @param default The default value of the field (not used here).
 */
#define WORKBENCH_CONFIG_DEFINE_STRUCT(type, field, default) type field;

/**
 * @brief Macro to set default values for configuration fields.
 *
 * The `WORKBENCH_CONFIG_SET_DEFAULTS` macro initializes the fields of the
 * configuration structure with default values.
 *
 * This macro is intended to be used when defining the default configuration
 * settings.
 *
 * @param type The data type of the field.
 * @param field The name of the field.
 * @param default The default value of the field.
 */
#define WORKBENCH_CONFIG_SET_DEFAULTS(type, field, default) .field = default,

/**
 * @brief Macro to define setter function declarations for configuration fields.
 *
 * The `WORKBENCH_CONFIG_DEFINE_SETTERS` macro generates the declaration of
 * setter functions for each field in the configuration structure. Each
 * generated function allows setting the value of a specific configuration
 * field.
 *
 * This macro is intended to be passed to the `CONFIG` macro to generate
 * the setter function declarations.
 *
 * @param type The data type of the field.
 * @param field The name of the field.
 * @param default The default value of the field (not used in this macro, but
 * included for consistency).
 */
#define WORKBENCH_CONFIG_DEFINE_SETTERS(type, field, default)                  \
  void config_set_##field(type field);

/**
 * @brief Macro to implement setter functions for configuration fields.
 *
 * The `WORKBENCH_CONFIG_IMPLEMENT_SETTERS` macro generates the implementation
 * of setter functions for each field in the configuration structure. Each
 * generated function allows setting the value of a specific configuration
 * field.
 *
 * @param type The data type of the field.
 * @param field The name of the field.
 * @param default The default value of the field (not used here).
 */
#define WORKBENCH_CONFIG_IMPLEMENT_SETTERS(type, field, default)               \
  __attribute__((weak)) void config_set_##field(type field) {                  \
    __cfg.field = field;                                                       \
  }

/**
 * @brief Macro to parse configuration arguments.
 *
 * The `WORKBENCH_CONFIG_PARSE_ARGUMENTS` macro adds parsed arguments to an
 * array if they match a configuration field name. This is used during the
 * configuration parsing process.
 *
 * @param type The data type of the field.
 * @param field The name of the field.
 * @param default The default value of the field (not used here).
 */
#define WORKBENCH_CONFIG_PARSE_ARGUMENTS(type, field, default)                 \
  else if (strcmp(arg, #field) == 0) {                                         \
    arguments[(*args_parsed)++] = (Argument_t){.arg = arg, .val = val};        \
  }

/**
 * @brief Macro to parse configuration field values from strings.
 *
 * The `WORKBENCH_CONFIG_PARSE_STRING` macro parses the value of a configuration
 * field from a string representation and assigns it to the appropriate field in
 * the configuration structure.
 *
 * @param type The data type of the field.
 * @param field The name of the field.
 * @param default The default value of the field.
 */
#define WORKBENCH_CONFIG_PARSE_STRING(type, field, default)                    \
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

/**
 * @brief Defines actual values for feature flags.
 *
 * The `DEFINE` macro assigns bitwise values to each feature flag by shifting
 * `1` left by the corresponding bit index from the `flag_bits` enumeration.
 *
 * @param FLAG The macro parameter representing a feature flag.
 */
#define WORKBENCH_FLAG_DEFINE(FLAG) FLAG = (1 << FLAG##_BIT),

/**
 * @brief Enumerates flag bits.
 *
 * The `ENUMERATE` macro appends `_BIT` to each flag name, creating an
 * enumeration of flag bits. This enumeration assigns a unique index to each
 * flag, which can be used to generate bitwise representations.
 *
 * @param FLAG The macro parameter representing a feature flag.
 */
#define WORKBENCH_FLAG_ENUMERATE(FLAG) FLAG##_BIT,