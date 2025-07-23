#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace Drift::RHI {

    // Enumeração unificada para todos os formatos
    enum class Format {
        // Formatos básicos/desconhecidos
        Unknown,
        
        // Formatos de textura/buffer
        R8_UNORM,
        R8G8_UNORM,
        R8G8B8A8_UNORM,
        R8G8B8A8_SNORM,
        R16_UINT,
        R16G16_UNORM,
        R16G16B16A16_UNORM,
        R32_UINT,
        R32G32_UINT,
        R32G32B32_UINT,
        R32G32B32A32_UINT,
        
        // Formatos de vertex específicos
        R32_FLOAT,
        R32G32_FLOAT,
        R32G32B32_FLOAT,
        R32G32B32A32_FLOAT,
        
        // Formatos especiais
        R10G10B10A2_UNORM,
        R11G11B10_FLOAT,
        D24_UNORM_S8_UINT,
        BC1_UNORM,
        BC3_UNORM
    };

    // Helpers para type safety e validação
    namespace FormatHelpers {
        constexpr bool IsVertexFormat(Format fmt) {
            return fmt >= Format::R32_FLOAT && fmt <= Format::R32G32B32A32_FLOAT;
        }
        
        constexpr bool IsTextureFormat(Format fmt) {
            return (fmt >= Format::R8_UNORM && fmt <= Format::R32G32B32A32_UINT) ||
                   (fmt >= Format::R10G10B10A2_UNORM && fmt <= Format::BC3_UNORM);
        }
        
        constexpr bool IsIndexFormat(Format fmt) {
            return fmt == Format::R16_UINT || fmt == Format::R32_UINT;
        }
        
        constexpr bool IsDepthStencilFormat(Format fmt) {
            return fmt == Format::D24_UNORM_S8_UINT;
        }
    }

    // Estrutura para descrição de elemento de input layout
    struct InputElementDesc {
        const char* semanticName;
        uint32_t semanticIndex;
        Format format;  // ← Agora usa Format unificado
        uint32_t offset;
        
        InputElementDesc(const char* name, uint32_t index, Format fmt, uint32_t off)
            : semanticName(name), semanticIndex(index), format(fmt), offset(off) {}
            
        bool operator==(const InputElementDesc& o) const {
            return semanticName == o.semanticName &&
                   semanticIndex == o.semanticIndex &&
                   format == o.format &&
                   offset == o.offset;
        }
    };

    // Conversão de Format para string (para compatibilidade)
    const char* FormatToString(Format format);
    
    // Conversão de string para Format (para compatibilidade)
    Format StringToFormat(const char* str);

    // Aliases para compatibilidade (deprecated - usar Format diretamente)
    using VertexFormat = Format;  // Para migração gradual

} // namespace Drift::RHI 