#pragma once

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <limits>
#include <algorithm>
#include <vector> // Added for std::vector

namespace Drift::UI {

// ============================================================================
// LAYOUT TYPES
// ============================================================================

enum class LayoutType {
    None,
    Stack,      // StackPanel (vertical/horizontal)
    Grid,       // Grid layout
    Absolute    // Absolute positioning
};

enum class StackDirection {
    Vertical,
    Horizontal
};

enum class GridUnitType {
    Auto,       // Size to content
    Fixed,      // Fixed pixel size
    Star        // Proportional size (*)
};

// ============================================================================
// LAYOUT PROPERTIES
// ============================================================================

struct LayoutProperties {
    // Margins and padding
    glm::vec4 margin = glm::vec4(0.0f);   // left, top, right, bottom
    glm::vec4 padding = glm::vec4(0.0f);  // left, top, right, bottom
    
    // Alignment
    enum class HorizontalAlign {
        Left,
        Center,
        Right,
        Stretch
    };
    
    enum class VerticalAlign {
        Top,
        Center,
        Bottom,
        Stretch
    };
    
    HorizontalAlign horizontalAlign = HorizontalAlign::Stretch;
    VerticalAlign verticalAlign = VerticalAlign::Stretch;
    
    // Size constraints
    glm::vec2 minSize = glm::vec2(0.0f);
    glm::vec2 maxSize = glm::vec2(std::numeric_limits<float>::max());
    
    // Layout type
    LayoutType layoutType = LayoutType::None;
    
    // Stack properties
    StackDirection stackDirection = StackDirection::Vertical;
    float stackSpacing = 0.0f;
    
    // Grid properties
    int gridRow = 0;
    int gridColumn = 0;
    int gridRowSpan = 1;
    int gridColumnSpan = 1;
    
    // Overflow handling
    bool clipContent = false;
};

// ============================================================================
// GRID DEFINITIONS
// ============================================================================

struct GridUnit {
    GridUnitType type = GridUnitType::Auto;
    float value = 0.0f;  // For Fixed: pixel size, For Star: weight
    
    GridUnit() = default;
    GridUnit(GridUnitType t, float v = 0.0f) : type(t), value(v) {}
    
    static GridUnit Auto() { return GridUnit(GridUnitType::Auto); }
    static GridUnit Fixed(float pixels) { return GridUnit(GridUnitType::Fixed, pixels); }
    static GridUnit Star(float weight = 1.0f) { return GridUnit(GridUnitType::Star, weight); }
};

struct GridDefinition {
    std::vector<GridUnit> rows;
    std::vector<GridUnit> columns;
    
    GridDefinition() = default;
    GridDefinition(const std::vector<GridUnit>& r, const std::vector<GridUnit>& c) 
        : rows(r), columns(c) {}
};

// ============================================================================
// LAYOUT MARGINS
// ============================================================================

struct LayoutMargins {
    float left, top, right, bottom;
    
    LayoutMargins() : left(0), top(0), right(0), bottom(0) {}
    LayoutMargins(float all) : left(all), top(all), right(all), bottom(all) {}
    LayoutMargins(float horizontal, float vertical) 
        : left(horizontal), top(vertical), right(horizontal), bottom(vertical) {}
    LayoutMargins(float l, float t, float r, float b) 
        : left(l), top(t), right(r), bottom(b) {}
    
    glm::vec4 ToVec4() const { return glm::vec4(left, top, right, bottom); }
    
    float GetHorizontal() const { return left + right; }
    float GetVertical() const { return top + bottom; }
};

// ============================================================================
// LAYOUT RECT
// ============================================================================

struct LayoutRect {
    float x, y, width, height;
    
    LayoutRect() : x(0), y(0), width(0), height(0) {}
    LayoutRect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    LayoutRect(const glm::vec2& pos, const glm::vec2& size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}
    
    float GetCenterX() const { return x + width * 0.5f; }
    float GetCenterY() const { return y + height * 0.5f; }
    glm::vec2 GetCenter() const { return glm::vec2(GetCenterX(), GetCenterY()); }
    
    bool Contains(const glm::vec2& point) const {
        return point.x >= x && point.x <= x + width && 
               point.y >= y && point.y <= y + height;
    }
    
    bool Intersects(const LayoutRect& other) const {
        return x < other.x + other.width && x + width > other.x &&
               y < other.y + other.height && y + height > other.y;
    }
    
    LayoutRect Intersection(const LayoutRect& other) const {
        if (!Intersects(other)) return LayoutRect();
        
        float ix = std::max(x, other.x);
        float iy = std::max(y, other.y);
        float iw = std::min(x + width, other.x + other.width) - ix;
        float ih = std::min(y + height, other.y + other.height) - iy;
        
        return LayoutRect(ix, iy, iw, ih);
    }
    
    glm::vec2 GetPosition() const { return glm::vec2(x, y); }
    glm::vec2 GetSize() const { return glm::vec2(width, height); }
    
    void SetPosition(const glm::vec2& pos) { x = pos.x; y = pos.y; }
    void SetSize(const glm::vec2& size) { width = size.x; height = size.y; }
};

// ============================================================================
// LAYOUT MEASURE
// ============================================================================

struct LayoutMeasure {
    float width, height;
    bool isStretched;
    
    LayoutMeasure() : width(0), height(0), isStretched(false) {}
    LayoutMeasure(float w, float h, bool stretched = false) 
        : width(w), height(h), isStretched(stretched) {}
    
    static LayoutMeasure Auto(float w, float h) { return LayoutMeasure(w, h, false); }
    static LayoutMeasure Stretch(float w, float h) { return LayoutMeasure(w, h, true); }
};

} // namespace Drift::UI 