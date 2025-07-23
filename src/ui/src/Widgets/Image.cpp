#include "Drift/UI/Widgets/Image.h"
#include "Drift/UI/UIContext.h"
#include "Drift/Core/Log.h"

using namespace Drift::UI;

Image::Image(UIContext* context)
    : UIElement(context)
{
    // Tamanho padrão da imagem
    SetSize(glm::vec2(100.0f, 100.0f));
}

void Image::Update(float deltaSeconds)
{
    UIElement::Update(deltaSeconds);
}

void Image::Render(Drift::RHI::IUIBatcher& batch)
{
    glm::vec2 absPos = GetAbsolutePosition();
    
    // TODO: Implementar renderização de textura real
    // Por enquanto, renderiza um retângulo colorido representando a imagem
    
    if (!m_ImagePath.empty()) {
        // Renderiza a imagem (placeholder)
        batch.AddRect(absPos.x, absPos.y, m_Size.x, m_Size.y, m_TintColor);
        
        // Renderiza uma borda para indicar que é uma imagem
        unsigned borderColor = 0xFF888888;
        float borderWidth = 1.0f;
        
        // Borda superior
        batch.AddRect(absPos.x, absPos.y, m_Size.x, borderWidth, borderColor);
        // Borda inferior
        batch.AddRect(absPos.x, absPos.y + m_Size.y - borderWidth, m_Size.x, borderWidth, borderColor);
        // Borda esquerda
        batch.AddRect(absPos.x, absPos.y, borderWidth, m_Size.y, borderColor);
        // Borda direita
        batch.AddRect(absPos.x + m_Size.x - borderWidth, absPos.y, borderWidth, m_Size.y, borderColor);
    } else {
        // Renderiza um placeholder se não há imagem
        unsigned placeholderColor = 0xFF444444;
        batch.AddRect(absPos.x, absPos.y, m_Size.x, m_Size.y, placeholderColor);
    }

    // Renderiza filhos
    for (auto& child : m_Children) {
        child->Render(batch);
    }
}

bool Image::LoadTexture(const std::string& path)
{
    m_ImagePath = path;
    
    // TODO: Implementar carregamento real de textura
    // Por enquanto, apenas simula o carregamento
    
    Core::Log("[Image] Carregando textura: " + path);
    
    // Simula carregamento bem-sucedido
    return true;
} 