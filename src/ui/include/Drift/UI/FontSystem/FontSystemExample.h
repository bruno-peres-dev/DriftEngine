#pragma once

namespace Drift::UI {

/**
 * @brief Exemplo completo do Sistema de Fontes Profissional
 * 
 * Demonstra todas as funcionalidades do sistema de fontes:
 * - Inicialização e configuração
 * - Carregamento de fontes
 * - Gerenciamento de cache
 * - Layout de texto
 * - Utilitários de texto
 * - Atlas de fontes
 * - Renderização
 */
class FontSystemExample {
public:
    /**
     * @brief Executa o exemplo completo do sistema de fontes
     */
    static void RunExample();

private:
    /**
     * @brief Inicializa o sistema de fontes para o exemplo
     */
    static void InitializeFontSystemExample();
    
    /**
     * @brief Demonstra carregamento de fontes
     */
    static void DemonstrateFontLoading();
    
    /**
     * @brief Demonstra gerenciamento de cache
     */
    static void DemonstrateCacheManagement();
    
    /**
     * @brief Demonstra layout de texto
     */
    static void DemonstrateTextLayout();
    
    /**
     * @brief Demonstra utilitários de texto
     */
    static void DemonstrateTextUtils();
    
    /**
     * @brief Demonstra atlas de fontes
     */
    static void DemonstrateFontAtlas();
    
    /**
     * @brief Demonstra renderização
     */
    static void DemonstrateRendering();
    
    /**
     * @brief Finaliza o sistema de fontes
     */
    static void ShutdownFontSystemExample();
};

} // namespace Drift::UI 