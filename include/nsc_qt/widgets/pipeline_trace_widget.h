#pragma once

#include "mips/processor.h"
#include <QWidget>
#include <cstdint>
#include <deque>
#include <map>
#include <vector>

class QTableWidget;

namespace nsc::qt {

class PipelineTraceWidget : public QWidget {
    Q_OBJECT

public:
    explicit PipelineTraceWidget(QWidget* parent = nullptr);

    void updateCycle(const mips::PipelineState& state);
    void clear();
    void setDarkMode(bool dark);

private:
    static constexpr int MAX_CYCLES = 20;

    void rebuildTable();

    // One row per instruction ever fetched (keyed by PC).
    struct InstrRow {
        uint32_t pc  = 0;
        uint32_t raw = 0;
        // stage_name[cycle_index], empty string = instruction not yet in that cycle
        std::deque<std::string> stages;
    };

    QTableWidget*          table_      = nullptr;
    std::vector<InstrRow>  rows_{};
    uint64_t               cycle_base_ = 0;   // column 0 maps to this cycle
    uint64_t               current_cycle_ = 0;
    bool                   dark_mode_  = false;
};

} // namespace nsc::qt
