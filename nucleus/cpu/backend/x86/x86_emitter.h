/**
 * (c) 2015 Alexandro Sanchez Bach. All rights reserved.
 * Released under GPL v2 license. Read LICENSE for more details.
 */

#pragma once

#include "nucleus/common.h"

#include "externals/xbyak/xbyak.h"

namespace cpu {
namespace backend {
namespace x86 {

enum X86Extension {
    AVX   = (1 << 0),  // Advanced Vector Extensions
    AVX2  = (1 << 1),  // Advanced Vector Extensions 2
    MOVBE = (1 << 2),  // Move Data After Swapping Bytes
};

class X86Emitter : public Xbyak::CodeGenerator {
    enum Mode {
        X86_MODE_32BITS = (1 << 0),
        X86_MODE_64BITS = (1 << 1),
    };

    // Available x86 extensions
    U32 extensions;

    // Chosen x86 mode
    U32 mode;

public:
    bool isExtensionAvailable(U32 queriedExtensions) const;
};

}  // namespace x86
}  // namespace backend
}  // namespace cpu
