#include "Drift/UI/FontSystem/FontSystem.h"
#include "Drift/UI/FontSystem/Font.h"
#include "Drift/UI/FontSystem/FontManager.h"
#include "Drift/UI/FontSystem/FontAtlas.h"
#include "Drift/UI/FontSystem/FontMetrics.h"
#include "Drift/UI/FontSystem/FontRendering.h"
#include "Drift/Core/Log.h"
#include "Drift/Core/Profiler.h"
#include <iostream>
#include <chrono>

namespace Drift::UI {

void FontSystemExample::RunExample() {
    DRIFT_PROFILE_FUNCTION();
    
    DRIFT_LOG_INFO("=== Exemplo do Sistema de Fontes Profissional ===");
    
    // 1. Inicializar o sistema de fontes
    InitializeFontSystemExample();
    
    // 2. Demonstrar carregamento de fontes
    DemonstrateFontLoading();
    
    // 3. Demonstrar cache e gerenciamento
    DemonstrateCacheManagement();
    
    // 4. Demonstrar layout de texto
    DemonstrateTextLayout();
    
    // 5. Demonstrar utilitários de texto
    DemonstrateTextUtils();
    
    // 6. Demonstrar atlas de fontes
    DemonstrateFontAtlas();
    
    // 7. Demonstrar renderização
    DemonstrateRendering();
    
    // 8. Finalizar
    ShutdownFontSystemExample();
    
    DRIFT_LOG_INFO("=== Exemplo do Sistema de Fontes Concluído ===");
}

void FontSystemExample::InitializeFontSystemExample() {
    DRIFT_PROFILE_FUNCTION();
    
    DRIFT_LOG_INFO("1. Inicializando Sistema de Fontes...");
    
    // Configurar o sistema
    FontSystemConfig config;
    config.enableAsyncLoading = true;
    config.enablePreloading = true;
    config.enableSubpixelRendering = true;
    config.enableKerning = true;
    config.enableLigatures = true;
    config.maxFonts = 20;
    config.maxAtlasSize = 1024;
    config.defaultQuality = FontQuality::High;
    config.fallbackFonts = {
        "fonts/Arial-Regular.ttf",
        "fonts/DejaVuSans.ttf"
    };
    
    // Inicializar sistema
    InitializeFontSystem(config);
    
    DRIFT_LOG_INFO("Sistema de fontes inicializado com sucesso!");
}

void FontSystemExample::DemonstrateFontLoading() {
    DRIFT_PROFILE_FUNCTION();
    
    DRIFT_LOG_INFO("2. Demonstrando Carregamento de Fontes...");
    
    auto& fontManager = FontManager::GetInstance();
    
    // Carregar fonte básica
    FontLoadConfig config;
    config.size = 16.0f;
    config.quality = FontQuality::High;
    config.enableKerning = true;
    config.enableHinting = true;
    
    auto font = fontManager.LoadFont("fonts/Arial-Regular.ttf", config);
    if (font) {
        DRIFT_LOG_INFO("Fonte carregada: {} ({}pt, {} glyphs)", 
                      font->GetName(), font->GetSize(), font->GetGlyphCount());
        
        // Verificar métricas
        const auto& metrics = font->GetMetrics();
        DRIFT_LOG_INFO("Métricas: ascent={:.1f}, descent={:.1f}, lineHeight={:.1f}", 
                      metrics.ascent, metrics.descent, metrics.lineHeight);
    }
    
    // Carregar múltiplas qualidades
    std::vector<FontQuality> qualities = {FontQuality::Low, FontQuality::Medium, FontQuality::High, FontQuality::Ultra};
    for (auto quality : qualities) {
        FontLoadConfig qualityConfig = config;
        qualityConfig.quality = quality;
        
        auto qualityFont = fontManager.LoadFont("fonts/Arial-Regular.ttf", qualityConfig);
        if (qualityFont) {
            DRIFT_LOG_INFO("Fonte {} carregada com qualidade {}", font->GetName(), static_cast<int>(quality));
        }
    }
    
    // Carregamento assíncrono
    DRIFT_LOG_INFO("Carregando fonte de forma assíncrona...");
    auto asyncFuture = fontManager.LoadFontAsync("fonts/Minecraft.ttf", config);
    
    // Simular trabalho enquanto carrega
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    if (asyncFuture.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready) {
        auto asyncFont = asyncFuture.get();
        if (asyncFont) {
            DRIFT_LOG_INFO("Fonte assíncrona carregada: {}", asyncFont->GetName());
        }
    }
}

void FontSystemExample::DemonstrateCacheManagement() {
    DRIFT_PROFILE_FUNCTION();
    
    DRIFT_LOG_INFO("3. Demonstrando Gerenciamento de Cache...");
    
    auto& fontManager = FontManager::GetInstance();
    
    // Pré-carregar fontes comuns
    DRIFT_LOG_INFO("Pré-carregando tamanhos comuns...");
    std::vector<float> commonSizes = {8, 12, 16, 24, 32, 48, 64};
    fontManager.PreloadCommonSizes("fonts/Arial-Regular.ttf", commonSizes);
    
    // Pré-carregar conjunto de caracteres
    DRIFT_LOG_INFO("Pré-carregando conjunto de caracteres...");
    std::vector<uint32_t> commonChars;
    for (uint32_t i = 32; i <= 126; ++i) { // ASCII básico
        commonChars.push_back(i);
    }
    // Adicionar caracteres especiais
    commonChars.insert(commonChars.end(), {0x00E7, 0x00C7, 0x00E1, 0x00E0, 0x00E2, 0x00E3}); // ç, Ç, á, à, â, ã
    fontManager.PreloadCharSet("fonts/Arial-Regular.ttf", commonChars);
    
    // Verificar estatísticas do cache
    auto stats = fontManager.GetStats();
    DRIFT_LOG_INFO("Estatísticas do cache:");
    DRIFT_LOG_INFO("  Total de fontes: {}", stats.totalFonts);
    DRIFT_LOG_INFO("  Fontes carregadas: {}", stats.loadedFonts);
    DRIFT_LOG_INFO("  Cache hits: {}", stats.cacheHits);
    DRIFT_LOG_INFO("  Cache misses: {}", stats.cacheMisses);
    DRIFT_LOG_INFO("  Uso de memória: {} bytes", stats.totalMemoryUsage);
    
    // Testar cache hit
    DRIFT_LOG_INFO("Testando cache hit...");
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10; ++i) {
        auto cachedFont = fontManager.GetFont("Arial-Regular", 16.0f);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    DRIFT_LOG_INFO("Tempo para 10 acessos ao cache: {} μs", duration.count());
    
    // Verificar estatísticas atualizadas
    stats = fontManager.GetStats();
    DRIFT_LOG_INFO("Cache hits após teste: {}", stats.cacheHits);
}

void FontSystemExample::DemonstrateTextLayout() {
    DRIFT_PROFILE_FUNCTION();
    
    DRIFT_LOG_INFO("4. Demonstrando Layout de Texto...");
    
    auto& fontManager = FontManager::GetInstance();
    auto font = fontManager.GetFont("Arial-Regular", 16.0f);
    
    if (!font) {
        DRIFT_LOG_ERROR("Fonte não disponível para teste de layout");
        return;
    }
    
    // Texto de teste
    std::string testText = "Olá, mundo! Este é um teste do sistema de fontes profissional.\n"
                          "Ele suporta múltiplas linhas, caracteres especiais (ç, á, ã) e quebra de linha automática.";
    
    // Configurar layout
    TextLayoutConfig layoutConfig;
    layoutConfig.maxWidth = 300.0f;
    layoutConfig.enableWordWrap = true;
    layoutConfig.horizontalAlign = TextAlign::Left;
    layoutConfig.verticalAlign = TextVerticalAlign::Top;
    layoutConfig.lineSpacing = 1.2f;
    layoutConfig.enableKerning = true;
    
    // Calcular layout
    auto layout = CalculateTextLayout(testText, font, layoutConfig);
    
    DRIFT_LOG_INFO("Layout calculado:");
    DRIFT_LOG_INFO("  Tamanho total: {:.1f}x{:.1f}", layout.totalSize.x, layout.totalSize.y);
    DRIFT_LOG_INFO("  Número de linhas: {}", layout.lineCount);
    DRIFT_LOG_INFO("  Número de caracteres: {}", layout.charCount);
    DRIFT_LOG_INFO("  Largura máxima: {:.1f}", layout.maxLineWidth);
    
    // Mostrar informações das linhas
    for (size_t i = 0; i < layout.lines.size(); ++i) {
        const auto& line = layout.lines[i];
        DRIFT_LOG_INFO("  Linha {}: '{}' (pos: {:.1f}, {:.1f}, tamanho: {:.1f}x{:.1f})", 
                      i, line.text, line.position.x, line.position.y, line.size.x, line.size.y);
    }
    
    // Testar diferentes alinhamentos
    DRIFT_LOG_INFO("Testando diferentes alinhamentos...");
    
    std::vector<TextAlign> aligns = {TextAlign::Left, TextAlign::Center, TextAlign::Right};
    std::vector<std::string> alignNames = {"Esquerda", "Centro", "Direita"};
    
    for (size_t i = 0; i < aligns.size(); ++i) {
        layoutConfig.horizontalAlign = aligns[i];
        auto alignedLayout = CalculateTextLayout("Texto alinhado", font, layoutConfig);
        DRIFT_LOG_INFO("  Alinhamento {}: primeira linha em x={:.1f}", 
                      alignNames[i], alignedLayout.lines[0].position.x);
    }
}

void FontSystemExample::DemonstrateTextUtils() {
    DRIFT_PROFILE_FUNCTION();
    
    DRIFT_LOG_INFO("5. Demonstrando Utilitários de Texto...");
    
    // Testar formatação de texto
    std::string testText = "Olá, MUNDO! Este é um teste com acentos: ç, á, ã, é, ê.";
    
    DRIFT_LOG_INFO("Texto original: {}", testText);
    DRIFT_LOG_INFO("Sem acentos: {}", TextUtils::RemoveAccents(testText));
    DRIFT_LOG_INFO("Minúsculas: {}", TextUtils::ToLower(testText));
    DRIFT_LOG_INFO("Maiúsculas: {}", TextUtils::ToUpper(testText));
    DRIFT_LOG_INFO("Title Case: {}", TextUtils::ToTitleCase("hello world test"));
    
    // Testar formatação de números
    DRIFT_LOG_INFO("Formatação de números:");
    DRIFT_LOG_INFO("  1234567 -> {}", TextUtils::FormatNumber(1234567));
    DRIFT_LOG_INFO("  -987654 -> {}", TextUtils::FormatNumber(-987654));
    DRIFT_LOG_INFO("  3.14159 -> {}", TextUtils::FormatDecimal(3.14159, 3));
    
    // Testar formatação de tempo
    DRIFT_LOG_INFO("Formatação de tempo:");
    DRIFT_LOG_INFO("  65 segundos -> {}", TextUtils::FormatTime(65.0f));
    DRIFT_LOG_INFO("  125 segundos -> {}", TextUtils::FormatTime(125.0f));
    DRIFT_LOG_INFO("  3661 segundos -> {}", TextUtils::FormatTime(3661.0f));
    
    // Testar formatação de bytes
    DRIFT_LOG_INFO("Formatação de bytes:");
    DRIFT_LOG_INFO("  1024 bytes -> {}", TextUtils::FormatBytes(1024));
    DRIFT_LOG_INFO("  1048576 bytes -> {}", TextUtils::FormatBytes(1048576));
    DRIFT_LOG_INFO("  1073741824 bytes -> {}", TextUtils::FormatBytes(1073741824));
    
    // Testar truncamento
    auto& fontManager = FontManager::GetInstance();
    auto font = fontManager.GetFont("Arial-Regular", 16.0f);
    
    if (font) {
        std::string longText = "Este é um texto muito longo que deve ser truncado com elipses quando exceder a largura máxima disponível.";
        std::string truncated = TextUtils::TruncateWithEllipsis(longText, font, 200.0f);
        DRIFT_LOG_INFO("Texto truncado: {}", truncated);
    }
}

void FontSystemExample::DemonstrateFontAtlas() {
    DRIFT_PROFILE_FUNCTION();
    
    DRIFT_LOG_INFO("6. Demonstrando Atlas de Fontes...");
    
    auto& fontManager = FontManager::GetInstance();
    auto font = fontManager.GetFont("Arial-Regular", 16.0f);
    
    if (!font) {
        DRIFT_LOG_ERROR("Fonte não disponível para teste de atlas");
        return;
    }
    
    // Verificar atlas da fonte
    auto atlas = font->GetAtlas();
    if (atlas) {
        DRIFT_LOG_INFO("Atlas da fonte:");
        DRIFT_LOG_INFO("  Tamanho: {}x{}", atlas->GetConfig().width, atlas->GetConfig().height);
        DRIFT_LOG_INFO("  Número de glyphs: {}", atlas->GetGlyphCount());
        DRIFT_LOG_INFO("  Uso: {:.1f}%", atlas->GetUsagePercentage());
        DRIFT_LOG_INFO("  Uso de memória: {} bytes", atlas->GetMemoryUsage());
        
        // Verificar se atlas está cheio
        if (atlas->IsFull()) {
            DRIFT_LOG_WARNING("Atlas está cheio!");
        }
    }
    
    // Testar carregamento de glyphs específicos
    DRIFT_LOG_INFO("Testando carregamento de glyphs...");
    std::vector<uint32_t> testChars = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
    
    for (uint32_t codepoint : testChars) {
        const GlyphInfo* glyph = font->GetGlyph(codepoint);
        if (glyph) {
            DRIFT_LOG_INFO("  Glyph '{}': tamanho {:.1f}x{:.1f}, advance {:.1f}", 
                          static_cast<char>(codepoint), glyph->size.x, glyph->size.y, glyph->advance);
        }
    }
    
    // Testar kerning
    DRIFT_LOG_INFO("Testando kerning...");
    std::vector<std::pair<uint32_t, uint32_t>> kerningPairs = {
        {'A', 'V'}, {'T', 'a'}, {'W', 'a'}, {'P', 'a'}
    };
    
    for (const auto& pair : kerningPairs) {
        float kerning = font->GetKerning(pair.first, pair.second);
        DRIFT_LOG_INFO("  Kerning '{}{}': {:.1f}", 
                      static_cast<char>(pair.first), static_cast<char>(pair.second), kerning);
    }
}

void FontSystemExample::DemonstrateRendering() {
    DRIFT_PROFILE_FUNCTION();
    
    DRIFT_LOG_INFO("7. Demonstrando Renderização...");
    
    auto& fontManager = FontManager::GetInstance();
    auto font = fontManager.GetFont("Arial-Regular", 16.0f);
    
    if (!font) {
        DRIFT_LOG_ERROR("Fonte não disponível para teste de renderização");
        return;
    }
    
    // Simular renderização (sem dispositivo RHI real)
    DRIFT_LOG_INFO("Simulando renderização de texto...");
    
    std::string renderText = "Texto para renderizar";
    glm::vec2 position(100.0f, 100.0f);
    glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Calcular medidas do texto
    float textWidth = CalculateTextWidth(renderText, font);
    float textHeight = CalculateTextHeight(renderText, font);
    
    DRIFT_LOG_INFO("Texto: '{}'", renderText);
    DRIFT_LOG_INFO("Posição: ({:.1f}, {:.1f})", position.x, position.y);
    DRIFT_LOG_INFO("Cor: ({:.1f}, {:.1f}, {:.1f}, {:.1f})", color.r, color.g, color.b, color.a);
    DRIFT_LOG_INFO("Medidas: {:.1f}x{:.1f}", textWidth, textHeight);
    
    // Simular renderização com efeitos
    DRIFT_LOG_INFO("Simulando efeitos de texto...");
    
    // Outline
    float outlineWidth = 2.0f;
    glm::vec4 outlineColor(0.0f, 0.0f, 0.0f, 1.0f);
    DRIFT_LOG_INFO("Outline: largura {:.1f}, cor ({:.1f}, {:.1f}, {:.1f}, {:.1f})", 
                  outlineWidth, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
    
    // Shadow
    glm::vec2 shadowOffset(2.0f, 2.0f);
    glm::vec4 shadowColor(0.0f, 0.0f, 0.0f, 0.5f);
    DRIFT_LOG_INFO("Sombra: offset ({:.1f}, {:.1f}), cor ({:.1f}, {:.1f}, {:.1f}, {:.1f})", 
                  shadowOffset.x, shadowOffset.y, shadowColor.r, shadowColor.g, shadowColor.b, shadowColor.a);
    
    // Gradiente
    glm::vec4 gradientStart(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 gradientEnd(0.0f, 0.0f, 1.0f, 1.0f);
    DRIFT_LOG_INFO("Gradiente: de ({:.1f}, {:.1f}, {:.1f}) para ({:.1f}, {:.1f}, {:.1f})", 
                  gradientStart.r, gradientStart.g, gradientStart.b,
                  gradientEnd.r, gradientEnd.g, gradientEnd.b);
}

void FontSystemExample::ShutdownFontSystemExample() {
    DRIFT_PROFILE_FUNCTION();
    
    DRIFT_LOG_INFO("8. Finalizando Sistema de Fontes...");
    
    // Mostrar estatísticas finais
    auto& fontManager = FontManager::GetInstance();
    auto finalStats = fontManager.GetStats();
    
    DRIFT_LOG_INFO("Estatísticas finais:");
    DRIFT_LOG_INFO("  Total de fontes carregadas: {}", finalStats.totalFonts);
    DRIFT_LOG_INFO("  Cache hits: {}", finalStats.cacheHits);
    DRIFT_LOG_INFO("  Cache misses: {}", finalStats.cacheMisses);
    DRIFT_LOG_INFO("  Taxa de hit: {:.1f}%", 
                  finalStats.cacheHits + finalStats.cacheMisses > 0 ? 
                  (float)finalStats.cacheHits / (finalStats.cacheHits + finalStats.cacheMisses) * 100.0f : 0.0f);
    DRIFT_LOG_INFO("  Uso total de memória: {} bytes", finalStats.totalMemoryUsage);
    DRIFT_LOG_INFO("  Tempo médio de carregamento: {:.2f}ms", finalStats.averageLoadTime);
    
    // Finalizar sistema
    ShutdownFontSystem();
    
    DRIFT_LOG_INFO("Sistema de fontes finalizado!");
}

} // namespace Drift::UI 