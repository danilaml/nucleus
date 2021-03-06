/**
 * (c) 2015 Alexandro Sanchez Bach. All rights reserved.
 * Released under GPL v2 license. Read LICENSE for more details.
 */

#include "rsx_pgraph.h"
#include "nucleus/emulator.h"
#include "nucleus/logger/logger.h"
#include "nucleus/gpu/rsx_enum.h"

namespace gpu {

U64 PGRAPH::HashTexture()
{
    return 0;
}

U64 PGRAPH::HashVertexProgram(rsx_vp_instruction_t* program)
{
    // 64-bit Fowler/Noll/Vo FNV-1a hash code
    U64 hash = 0xCBF29CE484222325ULL;
    do {
        hash ^= program->dword[0];
        hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) + (hash << 8) + (hash << 40);
        hash ^= program->dword[1];
        hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) + (hash << 8) + (hash << 40);
    } while (!(program++)->end);
    return hash;
}

U64 PGRAPH::HashFragmentProgram(rsx_fp_instruction_t* program)
{
    // 64-bit Fowler/Noll/Vo FNV-1a hash code
    bool end = false;
    U64 hash = 0xCBF29CE484222325ULL;
    do {
        hash ^= program->dword[0];
        hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) + (hash << 8) + (hash << 40);
        hash ^= program->dword[1];
        hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) + (hash << 8) + (hash << 40);
        end = ((program++)->word[0] >> 8) & 0x1; // NOTE: We can't acces program->end directly, since words require byte swapping
    } while (!end);
    return hash;
}

void PGRAPH::LoadVertexAttributes(U32 first, U32 count)
{
    // Bytes per vertex coordinate. Index is given by attribute::type.
    static const U32 vertexTypeSize[] = {
        0, 2, 4, 2, 1, 2, 4, 1
    };

    for (auto& attr : vpe.attr) {
        if (!attr.size) {
            continue;
        }

        // Get vertex buffer address
        U32 addr;
        if (attr.location == RSX_LOCATION_LOCAL) {
            addr = nucleus.memory(SEG_RSX_LOCAL_MEMORY).getBaseAddr() + attr.offset;
        } else {
            addr = nucleus.rsx.get_ea(attr.offset);
        }

        const U32 typeSize = vertexTypeSize[attr.type];
        attr.data.resize(count * attr.size * typeSize);

        // Copy data per vertex
        for (U32 i = 0; i < count; i++) {
            U32 src = addr + vertex_data_base_offset + attr.stride * (first + i + vertex_data_base_index);
            void* dst = &attr.data[i * attr.size * typeSize];

            switch (typeSize) {
            case 1:
                for (size_t j = 0; j < attr.size; j++) {
                    ((U8*)dst)[j] = nucleus.memory.read8(src + 1*j);
                }
                break;
            case 2:
                for (size_t j = 0; j < attr.size; j++) {
                    ((U16*)dst)[j] = nucleus.memory.read16(src + 2*j);
                }
                break;
            case 4:
                for (size_t j = 0; j < attr.size; j++) {
                    ((U32*)dst)[j] = nucleus.memory.read32(src + 4*j);
                }
                break;
            }
        }
    }
}

}  // namespace gpu
