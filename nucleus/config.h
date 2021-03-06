/**
 * (c) 2015 Alexandro Sanchez Bach. All rights reserved.
 * Released under GPL v2 license. Read LICENSE for more details.
 */

#pragma once

#include <string>

// Nucleus Settings
enum ConfigLanguage {
    LANGUAGE_DEFAULT,
    LANGUAGE_DE_DE,    // German (Germany)
    LANGUAGE_EN_US,    // English (United States)
    LANGUAGE_ES_ES,    // Spanish (Spain)
};

// CPU Settings
enum ConfigPpuTranslator {
    PPU_TRANSLATOR_INSTRUCTION,
    PPU_TRANSLATOR_BLOCK,
    PPU_TRANSLATOR_FUNCTION,
    PPU_TRANSLATOR_MODULE,
};

enum ConfigSpuTranslator {
    SPU_TRANSLATOR_INSTRUCTION,
    SPU_TRANSLATOR_BLOCK,
    SPU_TRANSLATOR_FUNCTION,
};

// GPU Settings
enum ConfigGpuBackend {
    GPU_BACKEND_NULL,
    GPU_BACKEND_SOFTWARE,
    GPU_BACKEND_OPENGL,
    GPU_BACKEND_DIRECT3D,
};

class Config
{
public:
    // Immediate settings
    std::string boot;       // Boot the specified file automatically
    bool console = false;   // Run Nucleus in console-only mode, preventing UI or GPU backends from running
    bool debugger = false;  // Start Nerve debugging server

    // Saved settings
    ConfigLanguage language = LANGUAGE_DEFAULT;
    ConfigPpuTranslator ppuTranslator = PPU_TRANSLATOR_FUNCTION;
    ConfigSpuTranslator spuTranslator = SPU_TRANSLATOR_INSTRUCTION;
    ConfigGpuBackend gpuBackend = GPU_BACKEND_OPENGL;

    // Modify settings with arguments or JSON files
    void parseArguments(int argc, char** argv);
    void parseFile(const std::string& path);
};

extern Config config;
