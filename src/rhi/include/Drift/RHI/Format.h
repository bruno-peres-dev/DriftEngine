#pragma once

#include <cstdint>

namespace Drift::RHI {

    // Enumeração tipada para formatos de vertex
    enum class VertexFormat {
        // Formatos básicos
        R32_FLOAT,
        R32G32_FLOAT,
        R32G32B32_FLOAT,
        R32G32B32A32_FLOAT,
        
        // Formatos inteiros
        R32_UINT,
        R32G32_UINT,
        R32G32B32_UINT,
        R32G32B32A32_UINT,
        
        // Formatos normalizados
        R8G8B8A8_UNORM,
        R8G8B8A8_SNORM,
        R16G16_UNORM,
        R16G16B16A16_UNORM,
        
        // Formatos especiais
        R10G10B10A2_UNORM,
        R11G11B10_FLOAT
    };

    // Estrutura para descrição de elemento de input layout
    struct InputElementDesc {
        const char* semanticName;
        uint32_t semanticIndex;
        VertexFormat format;
        uint32_t offset;
        
        InputElementDesc(const char* name, uint32_t index, VertexFormat fmt, uint32_t off)
            : semanticName(name), semanticIndex(index), format(fmt), offset(off) {}
            
        bool operator==(const InputElementDesc& o) const {
            return semanticName == o.semanticName &&
                   semanticIndex == o.semanticIndex &&
                   format == o.format &&
                   offset == o.offset;
        }
    };

    // Conversão de VertexFormat para string (para compatibilidade)
    const char* VertexFormatToString(VertexFormat format);
    
    // Conversão de string para VertexFormat (para compatibilidade)
    VertexFormat StringToVertexFormat(const char* str);

} // namespace Drift::RHI 