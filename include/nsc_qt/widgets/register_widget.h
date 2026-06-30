#pragma once

#include "mips/processor.h"
#include <QWidget>
#include <array>

class QLabel;
class QGridLayout;

namespace nsc::qt {

class RegisterWidget : public QWidget {
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget* parent = nullptr);

    void setPipelineState(const mips::PipelineState& state);
    void setShowAliases(bool show);
    void setDarkMode(bool dark);

    // Reset all cells to zero and clear highlights.
    void clear();

    // Push a fresh snapshot of all 32 register values and refresh display.
    void updateValues(const std::array<uint32_t, 32>& vals);

    // Direct read-only access for tests.
    [[nodiscard]] uint32_t value(int idx) const noexcept { return values_[idx]; }

private:
    void buildGrid();
    void updateCell(int idx);

    struct Cell {
        QLabel* name  = nullptr;
        QLabel* value = nullptr;
        int     fade  = 0;  // cycles remaining for green highlight (0 = none)
        bool    read  = false;
    };

    std::array<Cell, 32> cells_{};
    std::array<uint32_t, 32> values_{};
    QGridLayout* grid_   = nullptr;
    bool show_aliases_   = true;
    bool dark_mode_      = false;

    // Registers read by the current instruction in ID stage (highlight cyan).
    uint8_t read_rs_ = 0xFF;
    uint8_t read_rt_ = 0xFF;
};

} // namespace nsc::qt
