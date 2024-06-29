# Workbench Library

## Overview

The Workbench library provides a cookie-cutter for audio and MIDI processing using PortAudio and PortMidi. It includes functionality for initializing, configuring, and handling audio and MIDI input/output, along with utilities for logging and configuration management.

## Prerequisites

Before using the Workbench library, ensure you have the following prerequisites installed on your system:

- **PortAudio**: A cross-platform, open-source audio library.
- **PortMidi**: A library for MIDI input and output.
- **C Compiler**: A C compiler such as gcc or clang.
- **Pkg-config**: A helper tool used to configure compiler and linker flags for libraries.

## Installation

Ensure that you have PortAudio and PortMidi installed on your system. You can install these libraries using your package manager. For example, on Mac, you can install them using:

```bash
brew install pkg-config portaudio portmidi
```

Compilation
The Workbench library uses a Makefile to manage the build process. Below are the steps to compile the library and examples.

1. Clone the Repository

   Clone the repository containing the Workbench library source code:

   ```bash
   git clone https://github.com/your-repo/workbench.git
   cd workbench
   ```

2. Build the Library and Examples

   Run the following command to build the library and examples:

   ```bash
   make
   ```

   This command will compile the library and examples, placing the output binaries in the bin directory.

3. Generate Documentation

   If you have Doxygen installed, you can generate the documentation using:

   ```bash
   make docs
   ```

4. Clean the Build

   To clean the build directory, removing all compiled files, run:

   ```bash
   make clean
   ```

## Usage

Here are some basic usage examples to help you get started with the Workbench Audio and MIDI project.

### Audio

```c
#include "workbench.h"

// Disable midi
#define MIDI_ENABLED 0

Config *cfg;

void my_audio_callback(const void *input_buffer, void *output_buffer, unsigned long block_size, void *user_data) {
    const AudioSample_t *in = (const AudioSample_t *)input_buffer;
    AudioSample_t *out = (AudioSample_t *)output_buffer;

    // Process audio data here
}

int main(int argc, char **argv) {
  cfg = config_init(&my_audio_callback, NULL, NULL);
  while (true) {
    Pa_Sleep(1000);
  }
  return 0;
}
```

### MIDI

```c
#include "workbench.h"

// Disable audio
#define AUDIO_ENABLED 0

Config *cfg;

int my_midi_callback(const void *input_buffer, void *output_buffer, unsigned long block_size, void *user_data) {
    const PmEvent *in_buffer = (const PmEvent *)in;
    PmEvent *out_buffer = (PmEvent *)out;
    int out_len = 0;

    // Process MIDI data here

    return out_len;
}

int main(int argc, char **argv) {
  cfg = config_init(&my_audio_callback, NULL, NULL);
  while (true) {
    // Use porttime sleep function instead of portaudio
    Pt_Sleep(1000);
  }
  return 0;
}
```

### Configuration

The project includes a flexible configuration system using X-macros to manage various settings.

The `Config` structure holds all configuration parameters, and the following macros are used to define and manage these settings:

- `CONFIG`: Defines configuration fields and their default values.
- `FLAGS`: Lists feature flags for enabling or disabling various functionalities.
- `ENUMERATE` and `DEFINE`: Used to generate enumerations and bitwise values for feature flags.
