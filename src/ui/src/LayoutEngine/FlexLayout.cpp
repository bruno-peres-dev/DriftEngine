#include "Drift/UI/LayoutEngine/FlexLayout.h"
#include "Drift/Core/Log.h"
#include <algorithm>
#include <numeric>

using namespace Drift::UI;

void FlexLayoutEngine::LayoutFlexContainer(UIElement* container, const FlexProperties& properties) {
    if (!container || !properties.IsFlexContainer()) {
        return;
    }

    // Coleta todos os itens flex
    auto items = CollectFlexItems(container, properties);
    if (items.empty()) {
        return;
    }

    // Ordena por order
    std::sort(items.begin(), items.end(), [](const FlexItem& a, const FlexItem& b) {
        return a.properties.order < b.properties.order;
    });

    // Cria linhas de flex (para wrap)
    auto lines = CreateFlexLines(items, properties, container->GetSize());

    // Calcula tamanhos no eixo principal
    CalculateMainAxisSizes(lines, properties, container->GetSize());

    // Calcula tamanhos no eixo cruzado
    CalculateCrossAxisSizes(lines, properties, container->GetSize());

    // Aplica o layout
    ApplyLayout(lines, properties, container->GetSize());
}

void FlexLayoutEngine::LayoutFlexItem(UIElement* item, const FlexProperties& properties) {
    if (!item || !properties.IsFlexItem()) {
        return;
    }

    // Por enquanto, apenas aplica as propriedades básicas
    // O layout real será calculado pelo container pai
}

std::vector<FlexItem> FlexLayoutEngine::CollectFlexItems(UIElement* container, const FlexProperties& properties) {
    std::vector<FlexItem> items;
    
    for (auto& child : container->GetChildren()) {
        // TODO: Implementar IsVisible() no UIElement
        // if (!child->IsVisible()) continue;

        FlexItem item;
        item.element = child.get();
        item.position = child->GetPosition();
        item.size = child->GetSize();
        item.minSize = glm::vec2(0.0f); // TODO: Implementar min/max size
        item.maxSize = glm::vec2(std::numeric_limits<float>::infinity());
        item.isVisible = true; // TODO: Implementar IsVisible() no UIElement

        // TODO: Carregar FlexProperties do elemento
        // Por enquanto, usa valores padrão
        item.properties = FlexProperties{};

        items.push_back(item);
    }

    return items;
}

std::vector<FlexLine> FlexLayoutEngine::CreateFlexLines(const std::vector<FlexItem>& items, const FlexProperties& properties, const glm::vec2& containerSize) {
    std::vector<FlexLine> lines;
    
    if (properties.wrap == FlexProperties::Wrap::Nowrap) {
        // Uma única linha
        FlexLine line;
        line.items = items;
        lines.push_back(line);
    } else {
        // Múltiplas linhas (wrap)
        FlexLine currentLine;
        float currentLineSize = 0.0f;
        float maxLineSize = IsMainAxisHorizontal(properties) ? containerSize.x : containerSize.y;

        for (const auto& item : items) {
            float itemSize = GetMainAxisSize(item.size, properties);
            
            if (currentLineSize + itemSize > maxLineSize && !currentLine.items.empty()) {
                // Quebra para nova linha
                lines.push_back(currentLine);
                currentLine = FlexLine{};
                currentLineSize = 0.0f;
            }

            currentLine.items.push_back(item);
            currentLineSize += itemSize + properties.gap;
        }

        if (!currentLine.items.empty()) {
            lines.push_back(currentLine);
        }
    }

    return lines;
}

void FlexLayoutEngine::CalculateMainAxisSizes(std::vector<FlexLine>& lines, const FlexProperties& properties, const glm::vec2& containerSize) {
    for (auto& line : lines) {
        float availableSpace = GetMainAxisSize(containerSize, properties);
        float totalFlexGrow = 0.0f;
        float totalFlexShrink = 0.0f;
        float totalFixedSize = 0.0f;

        // Calcula totais
        for (auto& item : line.items) {
            totalFlexGrow += item.properties.flexGrow;
            totalFlexShrink += item.properties.flexShrink;
            
            if (item.properties.flexGrow == 0.0f) {
                totalFixedSize += GetMainAxisSize(item.size, properties);
            }
        }

        // Remove gaps
        float totalGaps = (line.items.size() - 1) * properties.gap;
        availableSpace -= totalGaps;

        // Distribui espaço flexível
        if (totalFlexGrow > 0.0f) {
            float flexSpace = availableSpace - totalFixedSize;
            if (flexSpace > 0.0f) {
                for (auto& item : line.items) {
                    if (item.properties.flexGrow > 0.0f) {
                        float itemFlexSize = (flexSpace * item.properties.flexGrow) / totalFlexGrow;
                        SetMainAxisSize(item.size, itemFlexSize, properties);
                    }
                }
            }
        }

        // Calcula tamanho total da linha
        float lineSize = 0.0f;
        for (const auto& item : line.items) {
            lineSize += GetMainAxisSize(item.size, properties);
        }
        lineSize += totalGaps;
        SetMainAxisSize(line.size, lineSize, properties);
    }
}

void FlexLayoutEngine::CalculateCrossAxisSizes(std::vector<FlexLine>& lines, const FlexProperties& properties, const glm::vec2& containerSize) {
    for (auto& line : lines) {
        float maxCrossSize = 0.0f;

        // Encontra o maior tamanho no eixo cruzado
        for (auto& item : line.items) {
            float itemCrossSize = GetCrossAxisSize(item.size, properties);
            maxCrossSize = std::max(maxCrossSize, itemCrossSize);
        }

        // Aplica align-items
        for (auto& item : line.items) {
            auto alignType = (item.properties.alignSelf == FlexProperties::AlignSelf::Auto) ? 
                            static_cast<int>(properties.alignItems) : 
                            static_cast<int>(item.properties.alignSelf);
            
            switch (alignType) {
                case static_cast<int>(FlexProperties::AlignItems::Stretch):
                    SetCrossAxisSize(item.size, maxCrossSize, properties);
                    break;
                case static_cast<int>(FlexProperties::AlignItems::Center):
                    // Mantém tamanho original, posição será ajustada em ApplyLayout
                    break;
                case static_cast<int>(FlexProperties::AlignItems::FlexStart):
                case static_cast<int>(FlexProperties::AlignItems::FlexEnd):
                case static_cast<int>(FlexProperties::AlignItems::Baseline):
                    // Mantém tamanho original
                    break;
            }
        }

        line.crossAxisSize = maxCrossSize;
    }
}

void FlexLayoutEngine::ApplyLayout(std::vector<FlexLine>& lines, const FlexProperties& properties, const glm::vec2& containerSize) {
    float currentCrossPosition = 0.0f;

    for (auto& line : lines) {
        float currentMainPosition = 0.0f;
        float lineCrossSize = line.crossAxisSize;

        // Aplica justify-content
        float totalMainSize = GetMainAxisSize(line.size, properties);
        float availableMainSpace = GetMainAxisSize(containerSize, properties) - totalMainSize;
        float mainOffset = 0.0f;

        switch (properties.justifyContent) {
            case FlexProperties::JustifyContent::Center:
                mainOffset = availableMainSpace * 0.5f;
                break;
            case FlexProperties::JustifyContent::FlexEnd:
                mainOffset = availableMainSpace;
                break;
            case FlexProperties::JustifyContent::SpaceBetween:
                if (line.items.size() > 1) {
                    float spacing = availableMainSpace / (line.items.size() - 1);
                    // Aplicar espaçamento entre itens
                }
                break;
            case FlexProperties::JustifyContent::SpaceAround:
                if (!line.items.empty()) {
                    float spacing = availableMainSpace / line.items.size();
                    mainOffset = spacing * 0.5f;
                    // Aplicar espaçamento ao redor de cada item
                }
                break;
            case FlexProperties::JustifyContent::SpaceEvenly:
                if (!line.items.empty()) {
                    float spacing = availableMainSpace / (line.items.size() + 1);
                    mainOffset = spacing;
                    // Aplicar espaçamento uniforme
                }
                break;
            case FlexProperties::JustifyContent::FlexStart:
            default:
                mainOffset = 0.0f;
                break;
        }

        currentMainPosition += mainOffset;

        // Posiciona itens na linha
        for (auto& item : line.items) {
            // Calcula posição no eixo principal
            float mainPos = currentMainPosition;
            if (properties.direction == FlexProperties::Direction::RowReverse) {
                mainPos = GetMainAxisSize(containerSize, properties) - currentMainPosition - GetMainAxisSize(item.size, properties);
            }

            // Calcula posição no eixo cruzado
            float crossPos = currentCrossPosition;
            auto alignType = (item.properties.alignSelf == FlexProperties::AlignSelf::Auto) ? 
                            static_cast<int>(properties.alignItems) : 
                            static_cast<int>(item.properties.alignSelf);
            
            switch (alignType) {
                case static_cast<int>(FlexProperties::AlignItems::Center):
                    crossPos += (lineCrossSize - GetCrossAxisSize(item.size, properties)) * 0.5f;
                    break;
                case static_cast<int>(FlexProperties::AlignItems::FlexEnd):
                    crossPos += lineCrossSize - GetCrossAxisSize(item.size, properties);
                    break;
                case static_cast<int>(FlexProperties::AlignItems::FlexStart):
                case static_cast<int>(FlexProperties::AlignItems::Stretch):
                case static_cast<int>(FlexProperties::AlignItems::Baseline):
                default:
                    break;
            }

            // Aplica posição
            if (IsMainAxisHorizontal(properties)) {
                item.position = glm::vec2(mainPos, crossPos);
            } else {
                item.position = glm::vec2(crossPos, mainPos);
            }

            // Aplica tamanho e posição ao elemento
            item.element->SetPosition(item.position);
            item.element->SetSize(item.size);

            // Avança para próximo item
            currentMainPosition += GetMainAxisSize(item.size, properties) + properties.gap;
        }

        // Avança para próxima linha
        currentCrossPosition += lineCrossSize;
        if (properties.wrap == FlexProperties::Wrap::WrapReverse) {
            currentCrossPosition = containerSize.y - currentCrossPosition;
        }
    }
}

// Helpers
bool FlexLayoutEngine::IsMainAxisHorizontal(const FlexProperties& properties) {
    return properties.direction == FlexProperties::Direction::Row || 
           properties.direction == FlexProperties::Direction::RowReverse;
}

float FlexLayoutEngine::GetMainAxisSize(const glm::vec2& size, const FlexProperties& properties) {
    return IsMainAxisHorizontal(properties) ? size.x : size.y;
}

float FlexLayoutEngine::GetCrossAxisSize(const glm::vec2& size, const FlexProperties& properties) {
    return IsMainAxisHorizontal(properties) ? size.y : size.x;
}

void FlexLayoutEngine::SetMainAxisSize(glm::vec2& size, float value, const FlexProperties& properties) {
    if (IsMainAxisHorizontal(properties)) {
        size.x = value;
    } else {
        size.y = value;
    }
}

void FlexLayoutEngine::SetCrossAxisSize(glm::vec2& size, float value, const FlexProperties& properties) {
    if (IsMainAxisHorizontal(properties)) {
        size.y = value;
    } else {
        size.x = value;
    }
}

glm::vec2 FlexLayoutEngine::GetMainAxisVector(const FlexProperties& properties) {
    return IsMainAxisHorizontal(properties) ? glm::vec2(1.0f, 0.0f) : glm::vec2(0.0f, 1.0f);
}

glm::vec2 FlexLayoutEngine::GetCrossAxisVector(const FlexProperties& properties) {
    return IsMainAxisHorizontal(properties) ? glm::vec2(0.0f, 1.0f) : glm::vec2(1.0f, 0.0f);
} 