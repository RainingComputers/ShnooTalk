#ifndef TRANSLATOR_PLATFORM
#define TRANSLATOR_PLATFORM

namespace translator
{
    enum Platform
    {
        DEFAULT,
        LINUX_x86_64,
        LINUX_ARM64,
        MACOS_x86_64,
        MACOS_ARM64,
        WASM32,
        WASM64,
        LINUX_x86_64_DYN,
        LINUX_ARM64_DYN,
        MACOS_x86_64_DYN,
        MACOS_ARM64_DYN
    };
}

#endif