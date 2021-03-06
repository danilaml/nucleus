/**
 * (c) 2015 Nucleus project. All rights reserved.
 * Released under GPL v2 license. Read LICENSE for more details.
 */

#include "spu_tables.h"

// Instruction entry
#define INSTRUCTION(name) { ENTRY_INSTRUCTION, nullptr, #name, &Analyzer::name, &Interpreter::name, &Recompiler::name }

// Table entry
# define TABLE(caller) { ENTRY_TABLE, caller, nullptr, nullptr, nullptr, nullptr }

namespace cpu {
namespace spu {

/**
 * SPU tables:
 * Initialized as static constant data when Nucleus starts up.
 * Invalid entries are zero-filled, therefore matching the ENTRY_INVALID type.
 * NOTE: Use designated initializers instead as soon as they become available on C++.
 */

template <int N>
struct Table {
    Entry table[N];

    const Entry& operator[] (const u32 i) const {
        return table[i];
    }
};

}  // namespace spu
}  // namespace cpu
