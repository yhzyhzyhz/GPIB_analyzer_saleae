# GPIB Analyzer

A Saleae Logic Analyzer plugin that decodes IEEE-488 / GPIB bus transfers. The analyzer reconstructs GPIB byte frames from the GPIB data lines and control signals, and supports simulation data generation for testing.

## Features

- Decodes 8-bit GPIB data words using the DAV strobe signal
- Supports standard GPIB control lines, including ATN, NRFD, NDAC, EOI, IFC, REN, and SRQ
- Outputs per-frame results in the Saleae Analyzer results view
- Includes simulation data generation for use with the Saleae software simulator

## Requirements

- CMake 3.13 or newer
- A C++ compiler compatible with the Saleae Analyzer SDK
- Internet access on first configure to fetch the Analyzer SDK via CMake FetchContent

## Building

From the repository root:

### Windows

```bat
mkdir build
cd build
cmake .. -A x64
cmake --build . --config Release
```

### Linux / macOS

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The analyzer plugin binary will be installed into the `build/Analyzers` directory.

## Installing the Analyzer

1. Open the Saleae application.
2. Open the developer directory for analyzers.
3. Copy the built plugin from `build/Analyzers` into the Saleae `Analyzers` folder.
4. Restart Saleae.

For Saleae developer directory setup, see:
https://support.saleae.com/faq/technical-faq/setting-up-developer-directory

## Repository Layout

- `CMakeLists.txt` — project build script
- `cmake/ExternalAnalyzerSDK.cmake` — fetches and configures the Saleae Analyzer SDK
- `src/` — analyzer source code and settings
- `docs/Analyzer_API.md` — Saleae Analyzer SDK documentation reference

## Notes

- The project uses `add_analyzer_plugin(GPIBAnalyzer ...)` to create the plugin target.
- `GPIBAnalyzer.cpp` implements the main worker thread and extracts bytes on the DAV falling edge.
- `GPIBSimulationDataGenerator.cpp` provides simulated GPIB traffic for verification.

## Building from source

If you need to modify or recompile the analyzer, build from source as described above. This repository is configured to download the Saleae Analyzer SDK automatically during CMake configure.
