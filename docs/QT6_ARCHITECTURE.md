# ClearCore Qt6 Architecture Guide

## Overview

This document describes the Qt6 GUI layer for ClearCore, designed to coexist alongside the existing FTXUI terminal UI without breaking changes to the core simulation libraries.

---

## Design Principles

### 1. **Clean Separation of Concerns**

The architecture is split into three layers:

```
┌─────────────────────────────────────┐
│   Qt6 UI Layer (nsc_qt)             │
│  ┌──────────────────────────────┐   │
│  │ MainWindow, Widgets          │   │
│  │ (datapath, register, memory) │   │
│  └──────────────────────────────┘   │
├─────────────────────────────────────┤
│   SimulatorController               │
│  (Thread-safe bridge)               │
├─────────────────────────────────────┤
│   Core Libraries (Unchanged)        │
│  ┌──────────────────────────────┐   │
│  │ mips_core: IProcessor        │   │
│  │ nsc_core: Converter          │   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

**Benefit**: `mips_core` and `nsc_core` remain completely UI-agnostic. They can be:
- Unit-tested independently
- Used by other frontends (TUI, CLI, web)
- Compiled without Qt dependencies

### 2. **Qt Signal/Slot for Thread Safety**

The `SimulatorController` runs the processor in a background thread and emits Qt signals when state changes:

```cpp
// Background thread
SimulatorController::stepCycle() {
    // Processor executes (potentially long operation)
    processor_->step();
    
    // Signal emission is thread-safe across thread boundary
    emit cycleExecuted(cycle_count);
}

// Main (UI) thread
MainWindow::onCycleExecuted(uint64_t cycle_count) {
    // Update GUI safely
    datapath_widget_->update();
    register_widget_->updateDisplay();
}
```

**Why Qt signals?**
- Automatic thread marshalling (Qt's event loop handles it)
- No manual mutex/lock management in UI code
- Standard Qt pattern, familiar to developers

### 3. **RAII Ownership Model**

All Qt widgets and the processor are owned via `std::unique_ptr` and `QObject` parent/child relationships:

```cpp
class MainWindow : public QMainWindow {
    std::unique_ptr<SimulatorController> controller_;
    widgets::DatapathWidget* datapath_widget_;  // owned by parent layout
};
```

**Benefit**: Automatic cleanup, no memory leaks, exception-safe.

---

## Implementation Roadmap

### Phase 1: Minimal Viable Product (Week 1-2)

**Goal**: Get the window and basic signal flow working.

1. **Add Qt6 to CMakeLists.txt**
   - Use `find_package(Qt6 COMPONENTS Core Gui Widgets)` with FetchContent fallback
   - Create separate target `nsc_qt_ui`

2. **Implement MainWindow scaffolding**
   - Create empty QMainWindow subclass
   - Stub out 6 tabs
   - Connect (empty) SimulatorController

3. **Implement SimulatorController**
   - Write thread-safe getters (registerValue, memoryWord, etc.)
   - Implement `stepCycle()` to call `processor_->step()`
   - Emit signals on each cycle

4. **Create DatapathWidget placeholder**
   - Render a static 5-stage pipeline diagram
   - No interactivity yet

5. **Test**
   - Manually step the processor
   - Verify cycle count increments
   - Verify signals are emitted correctly

**Deliverable**: `./clearCore-gui --file program.mips` loads and steps through program.

### Phase 2: Core Visualization (Week 3-4)

**Goal**: Render actual pipeline state with color-coded stages.

1. **Enhance DatapathWidget**
   - Read `PipelineState` from controller signal
   - Render instruction text in each stage
   - Color-code by stage (IF=blue, ID=cyan, EX=green, MEM=yellow, WB=red)
   - Show hazard badges (red = data, yellow = control)

2. **Implement RegisterWidget**
   - 4×8 grid of register cells
   - Display values in hex/decimal
   - Highlight recently-written registers
   - Highlight operands in current instruction

3. **Implement MemoryWidget**
   - Hex dump of data segment
   - Scroll/search
   - Highlight memory regions (stack, heap)

4. **Implement PipelineTraceWidget**
   - Instruction × cycle grid (like WebRISC-V)
   - Color cells by stage occupation
   - Export to CSV

5. **Test**
   - Run a simple 4-instruction program
   - Verify datapath updates on each cycle
   - Verify register/memory display matches processor state

**Deliverable**: Full interactive visualization of execution.

### Phase 3: Polish & UX (Week 5-6)

**Goal**: Professional-grade interface.

1. **Code Editor Integration**
   - Embed QPlainTextEdit with syntax highlighting for MIPS assembly
   - "Assemble" button to parse assembly into instruction words
   - Show assembly errors in red

2. **Statistics Dashboard**
   - Display CPI (cycles per instruction)
   - Show hazard/forwarding/flush counters
   - Plot CPI over time (QChart/QGraphicsView)

3. **Preferences**
   - Dark/light color scheme
   - Execution speed slider
   - Zoom/pan sensitivity
   - Save/restore window geometry

4. **Keyboard Shortcuts**
   - Ctrl+O: Open program
   - Ctrl+S: Save trace
   - F10: Step
   - F5: Run
   - Shift+F5: Stop
   - Ctrl+R: Reset

5. **Help/Documentation**
   - Built-in tutorial
   - Keyboard shortcut reference
   - "About ClearCore" dialog with academic citations

**Deliverable**: Production-ready GUI.

---

## Key Implementation Details

### Thread Safety Pattern

```cpp
// In SimulatorController (background thread)
void SimulatorController::stepCycle() {
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        processor_->step();
        stats_.cycles_executed++;
        auto state = processor_->getPipelineState();
    }
    // Emit signal (thread-safe, queued delivery)
    emit cycleExecuted(stats_.cycles_executed);
}

// In MainWindow (UI thread, connected to signal)
void MainWindow::onCycleExecuted(uint64_t cycle_count) {
    // No lock needed; signal guarantees we're in UI thread
    label_cycle_count_->setText(QString::number(cycle_count));
    datapath_widget_->update();  // Queue repaint
}
```

### Qt Signal Signature

Signals emitted across thread boundaries are automatically "queued":

```cpp
// SimulatorController.h
signals:
    void cycleExecuted(std::uint64_t cycle_count);  // Carried by value

// MainWindow.cpp (constructor)
connect(controller_.get(), &SimulatorController::cycleExecuted,
        this, &MainWindow::onCycleExecuted,
        Qt::QueuedConnection);  // Explicit (Qt uses this automatically for cross-thread)
```

Qt's event loop ensures `onCycleExecuted()` runs in the main thread, even if `cycleExecuted()` is emitted from a background thread.

### Custom Widget Rendering

For complex visuals (datapath diagram), use `QOpenGLWidget`:

```cpp
class DatapathWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render 5-stage boxes
        renderStageBox(0, "IF", ..., 50, 100, 150, 100);
        renderStageBox(1, "ID", ..., 250, 100, 150, 100);
        // ... etc
        
        // Render forwarding paths
        glLineWidth(2.0);
        glColor3f(0, 1, 0);  // Green
        glBegin(GL_LINES);
        glVertex2f(ex_x + ex_width, ex_y);  // From EX output
        glVertex2f(id_x, id_y + id_height / 2);  // To ID input
        glEnd();
    }
};
```

For simpler tables (register file, memory dump), use `QTableWidget` or `QAbstractTableModel`.

### Processor Integration

The controller assumes `IProcessor` has these methods:

```cpp
class IProcessor {
    virtual void step() = 0;
    virtual PipelineState getPipelineState() const = 0;
    virtual uint32_t getRegister(int index) const = 0;
    virtual void setRegister(int index, uint32_t value) = 0;
    virtual uint32_t getMemory(uint32_t byte_addr) const = 0;
    // ... etc
};
```

**If your `IProcessor` has different method names**, update the controller's implementation:

```cpp
// Old (in this design):
auto state = processor_->getPipelineState();

// Adapt to your actual API:
auto state = processor_->state();  // or processor_->queryState(), etc.
```

---

## File Structure

After implementation, your project tree looks like:

```
clearCore/
├── include/
│   ├── nsc_qt/
│   │   ├── simulator_controller.h
│   │   ├── main_window.h
│   │   └── widgets/
│   │       ├── datapath_widget.h
│   │       ├── register_widget.h
│   │       ├── memory_widget.h
│   │       └── pipeline_trace_widget.h
│   ├── mips/ (existing)
│   └── nsc/ (existing)
├── src/
│   ├── nsc_qt/
│   │   ├── main.cpp
│   │   ├── simulator_controller.cpp
│   │   ├── main_window.cpp
│   │   └── widgets/
│   │       ├── datapath_widget.cpp
│   │       ├── register_widget.cpp
│   │       ├── memory_widget.cpp
│   │       └── pipeline_trace_widget.cpp
│   ├── mips/ (existing)
│   └── nsc/ (existing)
├── CMakeLists.txt (updated with Qt6 option)
└── tests/ (existing)
```

---

## Building

### Build with both TUI and GUI

```bash
mkdir build && cd build
cmake .. -DBUILD_FTXUI_UI=ON -DBUILD_QT6_UI=ON -DBUILD_TESTS=ON
cmake --build .
```

**Output:**
- `clearCore` — FTXUI terminal UI
- `clearCore-gui` — Qt6 desktop GUI
- `decoder_test`, `cpu_test`, `nsc_tests` — unit tests

### Build Qt6 GUI only

```bash
cmake .. -DBUILD_FTXUI_UI=OFF -DBUILD_QT6_UI=ON
cmake --build .
```

### Build with Qt6 from system (faster)

If Qt6 is installed system-wide (e.g., `apt install qt6-base-dev`):

```bash
cmake .. -DBUILD_QT6_UI=ON
```

CMakeLists.txt will `find_package(Qt6)` first, then fall back to FetchContent if not found.

---

## Testing Strategy

### Unit Tests (No Qt Dependency)

Existing tests (`cpu_test`, `decoder_test`) remain unchanged. They test `mips_core` directly without any UI.

### Integration Tests (Qt)

Create `tests/qt_ui_test.cpp`:

```cpp
#include <QtTest>
#include "nsc_qt/simulator_controller.h"

class QtUiTest : public QObject {
    Q_OBJECT
private slots:
    void testControllerStepCycle() {
        auto proc = std::make_unique<mips::PipelinedCpu>();
        nsc::qt::SimulatorController ctrl(std::move(proc));
        
        QSignalSpy spy(&ctrl, &nsc::qt::SimulatorController::cycleExecuted);
        ctrl.stepCycle();
        
        QCOMPARE(spy.count(), 1);
        QCOMPARE(ctrl.cycleCount(), 1);
    }
};
```

---

## Migration Path: FTXUI → Qt6

**Option A: Dual-Build (Recommended)**

Maintain both `src/nsc/ui.cpp` (FTXUI) and `src/nsc_qt/` (Qt6) indefinitely:

```bash
./clearCore      # Terminal UI (fast, accessible, no GUI dependencies)
./clearCore-gui  # Desktop GUI (professional, interactive visualization)
```

Users choose based on context (headless server → TUI; classroom → GUI).

**Option B: Full Migration** (Later)

Once Qt6 GUI is mature and feature-complete, deprecate FTXUI:

- Archive `src/nsc/ui.cpp` in git history
- Remove FTXUI from CMakeLists.txt
- Update README: "ClearCore is now Qt6-only"

---

## Common Pitfalls & Solutions

### Pitfall 1: Emitting Signals with Complex Types

❌ **Wrong:**
```cpp
signals:
    void pipelineStateChanged(const mips::PipelineState& state);  // Reference!
```

✅ **Right:**
```cpp
signals:
    void pipelineStateChanged(const mips::PipelineState state);  // Value (copy)
```

Qt serializes signal parameters across thread boundaries. Use pass-by-value for custom types.

### Pitfall 2: Modifying UI from Background Thread

❌ **Wrong:**
```cpp
void SimulatorController::stepCycle() {
    processor_->step();
    ui_label_->setText("Cycle done");  // ⚠️ Crashes in background thread!
}
```

✅ **Right:**
```cpp
void SimulatorController::stepCycle() {
    processor_->step();
    emit cycleExecuted(stats_.cycles_executed);  // Signal to UI thread
}

void MainWindow::onCycleExecuted(uint64_t count) {
    label_cycle_->setText(QString::number(count));  // Safe in UI thread
}
```

### Pitfall 3: Forgetting to Add Files to CMakeLists.txt

Every new `.cpp` file must be added to `target_sources()`, and every header with Q_OBJECT must be in the MOC path:

```cmake
add_library(nsc_qt_ui STATIC)
target_sources(nsc_qt_ui PRIVATE
    src/nsc_qt/simulator_controller.cpp    # ← Add here
    src/nsc_qt/main_window.cpp             # ← And here
    src/nsc_qt/widgets/datapath_widget.cpp # ← And here
    ...
)
```

---

## Debugging Tips

### Check Qt Version

```cpp
#include <QtCore/qglobal.h>
qDebug() << "Qt Version:" << QT_VERSION_STR;
```

### Inspect Signal Connections

```cpp
connect(controller_.get(), &SimulatorController::cycleExecuted,
        this, &MainWindow::onCycleExecuted);

// Check if connection succeeded (Qt 5.6+):
QObject::connect(controller_.get(), &SimulatorController::cycleExecuted,
                 this, &MainWindow::onCycleExecuted,
                 Qt::QueuedConnection);  // Explicit queued (cross-thread safe)
```

### Trace Signal Emissions

```cpp
// In main.cpp before creating windows:
QLoggingCategory::setFilterRules("qt.qpa.*=false");  // Suppress noisy Qt logs
```

Add debug output:

```cpp
void SimulatorController::stepCycle() {
    qDebug() << "Stepping cycle" << stats_.cycles_executed;
    emit cycleExecuted(stats_.cycles_executed);
}
```

---

## References

- **Qt6 Documentation**: https://doc.qt.io/qt-6/
- **Qt Signal/Slots**: https://doc.qt.io/qt-6/signalsandslots.html
- **QOpenGLWidget**: https://doc.qt.io/qt-6/qopenglwidget.html
- **Thread Safety in Qt**: https://doc.qt.io/qt-6/threads-qobject.html

---

## Questions?

If you hit issues during implementation:

1. **Qt compilation error**: Check Qt6 is installed (`cmake -G "Unix Makefiles" .. && make VERBOSE=1`)
2. **Signal not emitted**: Verify `Q_OBJECT` macro is in class definition and MOC ran (check `build/moc_*.cpp` files)
3. **Crash in background thread**: Ensure all UI updates go through signals, never direct widget calls
4. **Memory issues**: Use `valgrind --leak-check=full ./clearCore-gui` to spot leaks

Good luck! 🚀
