/**
 * @file workbench_audio.h
 * @brief Audio system configuration for Workbench applications.
 *
 * This header file defines macros for configuring the audio system used in
 * Workbench applications with the PortAudio library. It includes type
 * definitions for audio samples based on the specified sample format.
 */
#pragma once

/** @brief  Defines the sample format to be used. Change this to change the type
 * of the audio sample. This macro should be set to one of the following values:
 * `Float32`, `Int32`, `Int24`, `Int16`, or `Int8`.
 */
#ifndef SAMPLE_FORMAT
#define SAMPLE_FORMAT Float32
#endif

#define _SAMPLE_TYPE_NAME(T) _SAMPLE_##T
#define _SAMPLE_TYPE(T) _SAMPLE_TYPE_NAME(T)

#define _SAMPLE_Float32 float
#define _SAMPLE_Int32 uint32_t
#define _SAMPLE_Int24 uint32_t
#define _SAMPLE_Int16 uint16_t
#define _SAMPLE_Int8 uint8_t

/**
 * @brief Defines the type for audio samples based on `SAMPLE_FORMAT`.
 *
 * This macro expands to the type corresponding to `SAMPLE_FORMAT`.
 * For example, if `SAMPLE_FORMAT` is `Float32`,
 * this macro will expand to `float`.
 */
#define SAMPLE _SAMPLE_TYPE(SAMPLE_FORMAT)

#define _PA_SAMPLE_FORMAT_HELPER(x) pa##x
#define _PA_SAMPLE_FORMAT(x) _PA_SAMPLE_FORMAT_HELPER(x)

/** @brief Sample format used by PortAudio */
#define PA_SAMPLE_FORMAT _PA_SAMPLE_FORMAT(SAMPLE_FORMAT)
