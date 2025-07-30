#pragma once

#include "Font.h"
#include "FontManager.h"
#include "TextRenderer.h"
#include "FontAtlas.h"
#include "FontMetrics.h"
#include "FontRendering.h"

namespace Drift::UI {

/**
 * @brief Sistema de Fontes Profissional - Nível AAA
 * 
 * Este namespace contém um sistema de fontes completo e profissional,
 * integrado ao sistema de assets do Drift Engine.
 * 
 * Características principais:
 * - Integração completa com AssetsSystem
 * - Suporte a múltiplos formatos (TTF, OTF, WOFF, WOFF2)
 * - Renderização de alta qualidade com anti-aliasing
 * - Cache inteligente de glyphs e atlas
 * - Suporte completo a Unicode e internacionalização
 * - Otimizações de performance para jogos AAA
 * - Sistema de fallback de fontes
 * - Renderização de texto em tempo real
 * - Suporte a efeitos de texto (sombra, outline, gradiente)
 * 
 * Componentes principais:
 * - Font: Representa uma fonte individual com métricas completas
 * - FontManager: Gerencia o cache e carregamento de fontes
 * - FontAtlas: Sistema de atlas de glyphs otimizado
 * - FontMetrics: Cálculos de métricas e layout de texto
 * - FontRendering: Renderização de alta qualidade
 * - TextRenderer: Interface de alto nível para renderização
 */

/**
 * @brief Configuração global do sistema de fontes
 */
struct FontSystemConfig {
    // Configurações de qualidade
    bool enableSubpixelRendering = true;      // Renderização subpixel para melhor qualidade
    bool enableKerning = true;                // Kerning entre caracteres
    bool enableLigatures = true;              // Ligaduras tipográficas
    bool enableHinting = true;                // Hinting de fontes
    
    // Configurações de cache
    size_t maxFonts = 50;                     // Máximo de fontes em cache
    size_t maxAtlasSize = 2048;               // Tamanho máximo do atlas (pixels)
    size_t maxGlyphsPerFont = 4096;           // Máximo de glyphs por fonte
    
    // Configurações de renderização
    float defaultDPI = 96.0f;                 // DPI padrão
    float defaultScale = 1.0f;                // Escala padrão
    bool enableMSDF = true;                   // Multi-channel signed distance field
    
    // Configurações de fallback
    std::vector<std::string> fallbackFonts;   // Fontes de fallback
    bool enableUnicodeFallback = true;        // Fallback para caracteres Unicode
    
    // Configurações de performance
    bool enableAsyncLoading = true;           // Carregamento assíncrono
    bool enablePreloading = true;             // Pré-carregamento
    size_t maxConcurrentLoads = 4;            // Máximo de carregamentos simultâneos
};

/**
 * @brief Inicializa o sistema de fontes
 * @param config Configuração do sistema
 */
void InitializeFontSystem(const FontSystemConfig& config = {});

/**
 * @brief Finaliza o sistema de fontes
 */
void ShutdownFontSystem();

/**
 * @brief Obtém a configuração atual do sistema
 */
const FontSystemConfig& GetFontSystemConfig();

/**
 * @brief Atualiza a configuração do sistema
 */
void SetFontSystemConfig(const FontSystemConfig& config);

} // namespace Drift::UI 