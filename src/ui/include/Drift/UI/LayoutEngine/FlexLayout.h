#pragma once

#include "Drift/UI/UIElement.h"
#include <vector>
#include <glm/vec2.hpp>

namespace Drift::UI {

// Propriedades Flexbox para elementos
struct FlexProperties {
    enum class Direction {
        Row,        // flex-direction: row
        RowReverse, // flex-direction: row-reverse
        Column,     // flex-direction: column
        ColumnReverse // flex-direction: column-reverse
    };

    enum class JustifyContent {
        FlexStart,    // justify-content: flex-start
        FlexEnd,      // justify-content: flex-end
        Center,       // justify-content: center
        SpaceBetween, // justify-content: space-between
        SpaceAround,  // justify-content: space-around
        SpaceEvenly   // justify-content: space-evenly
    };

    enum class AlignItems {
        FlexStart, // align-items: flex-start
        FlexEnd,   // align-items: flex-end
        Center,    // align-items: center
        Stretch,   // align-items: stretch
        Baseline   // align-items: baseline
    };

    enum class AlignSelf {
        Auto,      // align-self: auto
        FlexStart, // align-self: flex-start
        FlexEnd,   // align-self: flex-end
        Center,    // align-self: center
        Stretch,   // align-self: stretch
        Baseline   // align-self: baseline
    };

    enum class Wrap {
        Nowrap,    // flex-wrap: nowrap
        Wrap,      // flex-wrap: wrap
        WrapReverse // flex-wrap: wrap-reverse
    };

    // Propriedades do container
    Direction direction{Direction::Row};
    JustifyContent justifyContent{JustifyContent::FlexStart};
    AlignItems alignItems{AlignItems::Stretch};
    Wrap wrap{Wrap::Nowrap};
    float gap{0.0f}; // gap entre elementos

    // Propriedades do item
    int order{0};           // order
    float flexGrow{0.0f};   // flex-grow
    float flexShrink{1.0f}; // flex-shrink
    float flexBasis{0.0f};  // flex-basis (0 = auto)
    AlignSelf alignSelf{AlignSelf::Auto};

    // Helpers
    bool IsFlexContainer() const { return direction != Direction::Row || justifyContent != JustifyContent::FlexStart || alignItems != AlignItems::Stretch || wrap != Wrap::Nowrap || gap > 0.0f; }
    bool IsFlexItem() const { return order != 0 || flexGrow != 0.0f || flexShrink != 1.0f || flexBasis != 0.0f || alignSelf != AlignSelf::Auto; }
};

// Item de layout calculado
struct FlexItem {
    UIElement* element{nullptr};
    FlexProperties properties;
    glm::vec2 position{0.0f};
    glm::vec2 size{0.0f};
    glm::vec2 minSize{0.0f};
    glm::vec2 maxSize{std::numeric_limits<float>::infinity()};
    bool isVisible{true};
};

// Linha de flex items (para wrap)
struct FlexLine {
    std::vector<FlexItem> items;
    glm::vec2 size{0.0f};
    float crossAxisSize{0.0f};
};

// Engine de layout Flexbox
class FlexLayoutEngine {
public:
    // Calcula o layout de um container flex
    static void LayoutFlexContainer(UIElement* container, const FlexProperties& properties);
    
    // Calcula o layout de um item flex
    static void LayoutFlexItem(UIElement* item, const FlexProperties& properties);

private:
    // Métodos internos de cálculo
    static std::vector<FlexItem> CollectFlexItems(UIElement* container, const FlexProperties& properties);
    static std::vector<FlexLine> CreateFlexLines(const std::vector<FlexItem>& items, const FlexProperties& properties, const glm::vec2& containerSize);
    static void CalculateMainAxisSizes(std::vector<FlexLine>& lines, const FlexProperties& properties, const glm::vec2& containerSize);
    static void CalculateCrossAxisSizes(std::vector<FlexLine>& lines, const FlexProperties& properties, const glm::vec2& containerSize);
    static void ApplyLayout(std::vector<FlexLine>& lines, const FlexProperties& properties, const glm::vec2& containerSize);
    
    // Helpers
    static bool IsMainAxisHorizontal(const FlexProperties& properties);
    static float GetMainAxisSize(const glm::vec2& size, const FlexProperties& properties);
    static float GetCrossAxisSize(const glm::vec2& size, const FlexProperties& properties);
    static void SetMainAxisSize(glm::vec2& size, float value, const FlexProperties& properties);
    static void SetCrossAxisSize(glm::vec2& size, float value, const FlexProperties& properties);
    static glm::vec2 GetMainAxisVector(const FlexProperties& properties);
    static glm::vec2 GetCrossAxisVector(const FlexProperties& properties);
};

} // namespace Drift::UI 