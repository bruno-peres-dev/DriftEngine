#pragma once

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include "Drift/RHI/UIBatcher.h"
#include "Drift/Core/Color.h"
#include "Drift/UI/Transform2D.h"
#include "Drift/UI/LayoutTypes.h"
#include "Drift/Core/Log.h"

namespace Drift {
class Transform2D;
}

namespace Drift::UI {

class UIContext;

class UIElement : public std::enable_shared_from_this<UIElement> {
public:
    explicit UIElement(UIContext* context);
    virtual ~UIElement() = default;

    // === HIERARQUIA ===
    void AddChild(const std::shared_ptr<UIElement>& child);
    void RemoveChild(const std::shared_ptr<UIElement>& child);
    void RemoveFromParent();
    
    const std::vector<std::shared_ptr<UIElement>>& GetChildren() const { return m_Children; }
    UIElement* GetParent() const { return m_Parent; }
    UIElement* GetRoot() const;

    // === TRANSFORMAÇÕES BÁSICAS ===
    void SetPosition(const glm::vec2& pos) { m_Position = pos; MarkDirty(); }
    void SetSize(const glm::vec2& size) { m_Size = size; MarkDirty(); }
    void SetScale(const glm::vec2& s) { m_Transform.scale = s; MarkDirty(); }
    void SetRotation(float r) { m_Transform.rotation = r; MarkDirty(); }
    
    glm::vec2 GetPosition() const { return m_Position; }
    glm::vec2 GetSize() const { return m_Size; }
    glm::vec2 GetScale() const { return m_Transform.scale; }
    float GetRotation() const { return m_Transform.rotation; }

    // === VISIBILIDADE E OPACIDADE ===
    void SetVisible(bool v) { m_Visible = v; }
    void SetOpacity(float o) { m_Opacity = std::clamp(o, 0.0f, 1.0f); }
    bool IsVisible() const { return m_Visible; }
    float GetOpacity() const { return m_Opacity; }
    
    // === POSICIONAMENTO ABSOLUTO ===
    glm::vec2 GetAbsolutePosition() const;
    glm::mat4 GetWorldTransform() const { return m_WorldTransform; }
    glm::vec2 GetAbsoluteSize() const;

    // === CICLO DE VIDA ===
    virtual void Update(float deltaSeconds);
    virtual void PreRender(const glm::mat4& parentTransform = glm::mat4(1.0f));
    virtual void PostRender();
    virtual void Render(Drift::RHI::IUIBatcher& batch);

    // === COR E ESTILO ===
    void SetColor(Drift::Color col) { m_Color = col; }
    Drift::Color GetColor() const { return m_Color; }
    virtual Drift::Color GetRenderColor() const { return m_Color; }

    // === LAYOUT PROPERTIES ===
    void SetLayoutProperties(const LayoutProperties& props) { m_LayoutProps = props; MarkLayoutDirty(); }
    const LayoutProperties& GetLayoutProperties() const { return m_LayoutProps; }
    
    // === LAYOUT CALCULATION ===
    virtual void RecalculateLayout();
    
    // === DIRTY FLAGS ===
    void MarkDirty() { m_Dirty = true; }
    void MarkLayoutDirty() {
        m_LayoutDirty = true;
        if (m_Parent) {
            m_Parent->MarkLayoutDirty();
        }
    }
    bool IsDirty() const { return m_Dirty; }
    bool IsLayoutDirty() const { return m_LayoutDirty; }
    void ClearDirty() { m_Dirty = false; }
    void ClearLayoutDirty() { m_LayoutDirty = false; }

    // === HIT TESTING ===
    virtual bool HitTest(const glm::vec2& point) const;
    virtual UIElement* HitTestChildren(const glm::vec2& point);

    // === EVENTOS DE MOUSE ===
    virtual void OnMouseEnter() {}
    virtual void OnMouseLeave() {}
    virtual void OnMouseDown(const glm::vec2& position) {}
    virtual void OnMouseUp(const glm::vec2& position) {}
    virtual void OnMouseClick(const glm::vec2& position) {}

    // === IDENTIFICAÇÃO ===
    void SetName(const std::string& name) { m_Name = name; }
    const std::string& GetName() const { return m_Name; }

protected:
    // Membros protegidos para acesso pelos widgets
    UIContext* m_Context{nullptr};
    UIElement* m_Parent{nullptr};
    std::vector<std::shared_ptr<UIElement>> m_Children;
    
    // Transformações
    glm::vec2 m_Position{0.0f};
    glm::vec2 m_Size{0.0f};
    Transform2D m_Transform{};
    glm::mat4 m_WorldTransform{1.0f};
    
    // Estado
    bool m_Dirty{true};
    bool m_LayoutDirty{true};
    bool m_Visible{true};
    float m_Opacity{1.0f};
    Drift::Color m_Color{0xFF00FFFF}; // ciano por padrão
    
    // Layout
    LayoutProperties m_LayoutProps{};
    
    // Identificação
    std::string m_Name{};

private:
    void RecalculateTransform(const glm::mat4& parentTransform);
};

} // namespace Drift::UI 