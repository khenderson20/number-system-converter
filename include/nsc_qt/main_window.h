#pragma once

#include "nsc_qt/simulator_controller.h"
#include "mips/processor.h"
#include <QMainWindow>
#include <memory>

class QTabWidget;
class QLabel;
class QPlainTextEdit;
class QAction;
class QToolBar;

namespace nsc::qt {

class DatapathWidget;
class RegisterWidget;
class MemoryWidget;
class PipelineTraceWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onStep();
    void onRunPause();
    void onReset();
    void onOpenFile();
    void onSaveTrace();
    void onAssemble();
    void onLoad();
    void onShowPreferences();
    void onCycleExecuted(uint64_t count);
    void onPipelineStateChanged(mips::PipelineState state);
    void onStatisticsUpdated(nsc::qt::SimulatorStatistics stats);
    void onHalted();
    void onFaulted();
    void onBreakpointToggle(uint32_t pc);
    void onStageDetailRequested(int stage_index, uint32_t pc, uint32_t raw);

private:
    void setupMenuBar();
    void setupToolBar();
    void setupCentralWidget();
    void setupConnections();
    void applyColorScheme(bool dark);

    QWidget* createCodeEditorTab();
    QWidget* createStatisticsTab();

    // Controller
    std::unique_ptr<SimulatorController> controller_;

    // Widgets
    QTabWidget*          tabs_             = nullptr;
    DatapathWidget*      datapath_widget_  = nullptr;
    RegisterWidget*      register_widget_  = nullptr;
    MemoryWidget*        memory_widget_    = nullptr;
    PipelineTraceWidget* trace_widget_     = nullptr;
    QPlainTextEdit*      code_editor_      = nullptr;
    QLabel*              asm_status_lbl_   = nullptr;

    // Statistics labels
    QLabel* stat_cycles_lbl_      = nullptr;
    QLabel* stat_instrs_lbl_      = nullptr;
    QLabel* stat_cpi_lbl_         = nullptr;
    QLabel* stat_data_haz_lbl_    = nullptr;
    QLabel* stat_ctrl_haz_lbl_    = nullptr;
    QLabel* stat_fwd_lbl_         = nullptr;
    QLabel* stat_stalls_lbl_      = nullptr;
    QLabel* stat_flushes_lbl_     = nullptr;

    // Status bar labels
    QLabel* status_cycles_lbl_  = nullptr;
    QLabel* status_instrs_lbl_  = nullptr;
    QLabel* status_cpi_lbl_     = nullptr;

    // Actions
    QAction* act_step_      = nullptr;
    QAction* act_run_pause_ = nullptr;
    QAction* act_reset_     = nullptr;
    QAction* act_open_      = nullptr;
    QAction* act_save_      = nullptr;

    bool dark_mode_ = false;

    // Assembled words waiting to be loaded
    std::vector<uint32_t> assembled_words_;
};

} // namespace nsc::qt
