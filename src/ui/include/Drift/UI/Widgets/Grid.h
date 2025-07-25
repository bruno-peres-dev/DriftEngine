#pragma once

#include "Drift/UI/UIElement.h"
#include "Drift/UI/LayoutTypes.h"
#include <vector>

namespace Drift::UI {

class Grid : public UIElement {
public:
    Grid(UIContext* context);
    virtual ~Grid() = default;

    // Grid definition
    void SetRowDefinitions(const std::vector<GridUnit>& rows);
    void SetColumnDefinitions(const std::vector<GridUnit>& columns);
    void SetGridDefinition(const GridDefinition& definition);
    
    const std::vector<GridUnit>& GetRowDefinitions() const { return m_RowDefinitions; }
    const std::vector<GridUnit>& GetColumnDefinitions() const { return m_ColumnDefinitions; }

    // Layout override
    virtual void RecalculateLayout() override;
    
    // Render override
    virtual void Render(Drift::RHI::IUIBatcher& batch) override;

protected:
    std::vector<GridUnit> m_RowDefinitions;
    std::vector<GridUnit> m_ColumnDefinitions;
    
    // Calculate grid cell positions and sizes
    void CalculateGridLayout();
    std::vector<float> CalculateGridSizes(const std::vector<GridUnit>& definitions, float availableSize);
};

} // namespace Drift::UI 