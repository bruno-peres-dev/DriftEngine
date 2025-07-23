#pragma once

#include <vector>
#include <memory>
#include <glm/vec2.hpp>
#include "Drift/RHI/Buffer.h" // IUIBatcher

namespace Drift {
class Transform2D;
}

namespace Drift::UI {

class UIContext;

class UIElement : public std::enable_shared_from_this<UIElement> {
public:
    explicit UIElement(UIContext* context);
    virtual ~UIElement() = default;

    // Hierarquia
    void AddChild(const std::shared_ptr<UIElement>& child);
    void RemoveChild(const std::shared_ptr<UIElement>& child);

    const std::vector<std::shared_ptr<UIElement>>& GetChildren() const { return m_Children; }
    UIElement* GetParent() const { return m_Parent; }

    // Transformações básicas
    void SetPosition(const glm::vec2& pos) { m_Position = pos; MarkDirty(); }
    void SetSize(const glm::vec2& size) { m_Size = size; MarkDirty(); }
    glm::vec2 GetPosition() const { return m_Position; }
    glm::vec2 GetSize() const { return m_Size; }
    
    // Posicionamento absoluto (considera hierarquia)
    glm::vec2 GetAbsolutePosition() const;

    // Ciclo de vida
    virtual void Update(float deltaSeconds);

    // Desenha o elemento usando um batcher 2D
    virtual void Render(Drift::RHI::IUIBatcher& batch);

    // Cor (ARGB)
    void SetColor(unsigned col) { m_Color = col; }
    unsigned GetColor() const { return m_Color; }
    
    // Método virtual para permitir que subclasses retornem cores baseadas em seu estado
    virtual unsigned GetRenderColor() const { return m_Color; }

protected:
    void MarkDirty() { m_Dirty = true; }
    
    // Membros protegidos para acesso pelos widgets
    UIContext* m_Context{nullptr};
    UIElement* m_Parent{nullptr};
    std::vector<std::shared_ptr<UIElement>> m_Children;
    glm::vec2 m_Position{0.0f};
    glm::vec2 m_Size{0.0f};
    bool m_Dirty{true};
    unsigned m_Color{0xFF00FFFF}; // ciano por padrão

public:
    // Sistema de layout avançado
    void SetLayoutDirty() { m_LayoutDirty = true; }
    bool IsLayoutDirty() const { return m_LayoutDirty; }
    void ClearLayoutDirty() { m_LayoutDirty = false; }
    
    // Propriedades de layout
    enum class LayoutType {
        None,       // Layout manual
        Flex,       // Flexbox
        Grid,       // CSS Grid (futuro)
        Stack       // Stack layout (futuro)
    };
    
    void SetLayoutType(LayoutType type) { m_LayoutType = type; SetLayoutDirty(); }
    LayoutType GetLayoutType() const { return m_LayoutType; }

private:
    // Layout interno simples (placeholder)
    void RecalculateLayout();
    
protected:
    bool m_LayoutDirty{true};
    LayoutType m_LayoutType{LayoutType::None};
};

} // namespace Drift::UI 