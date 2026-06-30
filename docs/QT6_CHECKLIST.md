# Qt6 Implementation Checklist

## Phase 1: Foundation (1 Week)

### Step 1: Update CMakeLists.txt
- [ ] Add `option(BUILD_QT6_UI "Build GUI with Qt6" OFF)`
- [ ] Add Qt6 component detection with FetchContent fallback
- [ ] Create `nsc_qt_ui` target with `-std=c++20 -Wall -Wextra`
- [ ] Link to `mips_core` and Qt6 libraries
- [ ] Create `clearCore-gui` executable
- [ ] Test: `cmake .. -DBUILD_QT6_UI=ON && make`

### Step 2: Create SimulatorController
- [ ] Create `include/nsc_qt/simulator_controller.h` (provided above)
- [ ] Create `src/nsc_qt/simulator_controller.cpp` (provided above)
- [ ] Add all header guards and includes
- [ ] **Key methods to stub**:
  - [ ] `loadProgram()`
  - [ ] `reset()`
  - [ ] `stepCycle()` ← Calls `processor_->step()` and emits signal
  - [ ] Thread-safe getters: `registerValue()`, `memoryWord()`, `pipelineState()`
  - [ ] `setBreakpoint()`, `clearBreakpoint()`, `hasBreakpoint()`

**Test**: 
```cpp
auto proc = std::make_unique<mips::PipelinedCpu>();
nsc::qt::SimulatorController ctrl(std::move(proc));
ctrl.stepCycle();
assert(ctrl.cycleCount() == 1);
```

### Step 3: Create MainWindow Skeleton
- [ ] Create `include/nsc_qt/main_window.h` (provided above)
- [ ] Create `src/nsc_qt/main_window.cpp`:
  - [ ] Constructor: create SimulatorController
  - [ ] `setupMenuBar()`: File, Simulation, View menus (stubs)
  - [ ] `setupToolBar()`: Step, Run/Pause, Reset buttons
  - [ ] `setupCentralWidget()`: Create QTabWidget with 6 tabs
  - [ ] `setupConnections()`: Connect controller signals to slots (5-6 connections)
- [ ] Create `src/nsc_qt/main.cpp` with `QApplication` boilerplate (provided above)

**Test**:
```bash
./clearCore-gui
# Should show a window with empty tabs
```

### Step 4: Implement Minimal DatapathWidget
- [ ] Create `include/nsc_qt/widgets/datapath_widget.h` (provided above)
- [ ] Create `src/nsc_qt/widgets/datapath_widget.cpp`:
  - [ ] `initializeGL()`: Set clear color
  - [ ] `paintGL()`: Render 5 static stage boxes in a row
  - [ ] `setPipelineState()`: Store state (don't render yet)
  - [ ] No interaction yet
- [ ] In MainWindow: embed DatapathWidget in tab 1

**Visual**: Static boxes labeled "IF | ID | EX | MEM | WB"

**Test**: 
```bash
./clearCore-gui
# Tab 1 should show 5 gray boxes
```

### Step 5: Connect Controller → Widgets
- [ ] In MainWindow constructor:
  ```cpp
  connect(controller_.get(), &SimulatorController::cycleExecuted,
          this, [this](uint64_t count) {
              label_cycle_count_->setText(QString::number(count));
          });
  connect(controller_.get(), &SimulatorController::pipelineStateChanged,
          this, &MainWindow::onPipelineStateChanged);
  ```
- [ ] Implement `MainWindow::onPipelineStateChanged()`:
  - [ ] Call `datapath_widget_->setPipelineState(state)`
  - [ ] Update cycle counter label

**Test**:
```bash
./clearCore-gui
# Load a 4-instruction program
# Click "Step" button
# Cycle counter should increment, datapath updates
```

---

## Phase 2: Core Visualization (2 Weeks)

### Step 6: Enhance DatapathWidget Rendering
- [ ] `paintGL()` now reads from `current_pipeline_state_`:
  - [ ] **Stage 0 (IF)**: Show PC and instruction word in hex
  - [ ] **Stage 1 (ID)**: Show decoded opcode (add, lw, etc.) and operands
  - [ ] **Stage 2 (EX)**: Show ALU operation and result
  - [ ] **Stage 3 (MEM)**: Show memory address (if applicable)
  - [ ] **Stage 4 (WB)**: Show destination register and write value
- [ ] Color each stage box:
  - [ ] IF = light blue (#E3F2FD)
  - [ ] ID = light cyan (#E0F7FA)
  - [ ] EX = light green (#E8F5E9)
  - [ ] MEM = light yellow (#FFFDE7)
  - [ ] WB = light red (#FFEBEE)
- [ ] Render instruction text in monospace font inside each box
- [ ] Add pipeline stage labels above boxes (smaller font)

**Reference**: Look at Ripes' datapath visualization for inspiration.

**Test**:
```bash
# Run through a `add $t0, $t1, $t2` instruction
# Cycle 1: IF shows the add instruction
# Cycle 2: ID shows "add $t0,$t1,$t2"
# Cycle 3: EX shows ALU computation
# Cycle 4: MEM empty (no memory access)
# Cycle 5: WB shows $t0 ← result
```

### Step 7: Implement RegisterWidget
- [ ] Create `src/nsc_qt/widgets/register_widget.cpp`:
  - [ ] `setupRegisterCells()`: Create 4×8 grid of QLabel pairs
  - [ ] `setPipelineState()`: Update all 32 register displays
  - [ ] `updateCellDisplay(int index)`: Render one cell with:
    - [ ] Register name ($0, $1, ..., $31)
    - [ ] Register alias ($zero, $at, ..., $ra)
    - [ ] Current value (hex)
  - [ ] Track modification history (fade highlight)
- [ ] Layout: Use QGridLayout with 4 columns, 8 rows
- [ ] Color scheme:
  - [ ] Recently written = green highlight (fade over 5 cycles)
  - [ ] Read in current instruction = cyan highlight
  - [ ] $0 = always gray (read-only)

**Test**:
```bash
# Run `add $t0, $t1, $t2` (t0=$8, t1=$9, t2=$10)
# Cycle 5 (WB): $8 cell should highlight green
# Cycles 6-10: Green fades to normal
```

### Step 8: Implement MemoryWidget (Simplified)
- [ ] Create `src/nsc_qt/widgets/memory_widget.cpp`:
  - [ ] QTableWidget with columns: Address | Bytes 0-15 | ASCII
  - [ ] Display starting address 0x10000000 (data segment)
  - [ ] Show 16 rows (256 bytes total)
  - [ ] Highlight recently-written bytes (yellow bg)
  - [ ] Read-only display for now (editing comes later)
- [ ] Manual address navigation: QSpinBox at top

**Test**:
```bash
# Run a `sw $t0, 0($sp)` instruction (store to memory)
# Memory cell should highlight yellow
```

### Step 9: Implement PipelineTraceWidget (Simplified)
- [ ] Create `src/nsc_qt/widgets/pipeline_trace_widget.cpp`:
  - [ ] QTableWidget: rows = instructions, columns = cycles
  - [ ] `recordInstructionFetch()`: Add row when new instruction fetched
  - [ ] `updateCycle()`: Fill current column with stage info
  - [ ] Color each cell by stage (IF=blue, ID=cyan, EX=green, MEM=yellow, WB=red)
- [ ] Show only last 10 cycles to keep table readable

**Test**:
```bash
# Run 10-cycle program
# Table should show instruction × stage timeline
# Each instruction progresses left-to-right (IF → WB)
```

### Step 10: Connect All Widgets to Controller
- [ ] In MainWindow:
  - [ ] Connect `pipelineStateChanged` → `register_widget->setPipelineState()`
  - [ ] Connect `pipelineStateChanged` → `memory_widget->updateDisplay()`
  - [ ] Connect `cycleExecuted` → `trace_widget->updateCycle()`
- [ ] Implement `onPipelineStateChanged()` to update all widgets in sequence

**Test**:
```bash
# Click Step 5 times
# All widgets update synchronously
# Datapath, registers, memory, and trace all agree
```

---

## Phase 3: Polish (1 Week)

### Step 11: Add Code Editor Tab
- [ ] Create `src/nsc_qt/main_window.cpp::createProgramLoaderTab()`:
  - [ ] QPlainTextEdit for assembly code
  - [ ] "Assemble" button
  - [ ] "Load" button
  - [ ] Status label (green = success, red = error)
- [ ] Implement assembler bridge:
  - [ ] Parse MIPS mnemonics into instruction words
  - [ ] Call `controller_->loadProgram(std::vector<uint32_t>)`
  - [ ] Show assembly errors (e.g., "invalid register $ra" on line 3)

**Minimal assembler**: Accept `add $t0, $t1, $t2`, `lw $t0, 0($sp)`, etc.

**Test**:
```
[Code Editor]
add $t0, $t1, $t2
sw $t0, 0($sp)

[Assemble button clicked]
Status: ✓ 2 instructions assembled
[Load button clicked]
Status: ✓ Program loaded
[Step 5 times]
Datapath shows execution
```

### Step 12: Add Statistics Panel
- [ ] Create `src/nsc_qt/main_window.cpp::createStatisticsTab()`:
  - [ ] QLabel fields for:
    - [ ] Cycles executed
    - [ ] Instructions retired
    - [ ] CPI (cycles per instruction)
    - [ ] Data hazards
    - [ ] Control hazards
    - [ ] Forwarding events
    - [ ] Stalls/flushes (pipelined mode)
- [ ] Implement `SimulatorController::statistics()` getter
- [ ] Connect `statisticsUpdated` signal to update labels

**Test**:
```bash
# Run 100-cycle program
# Statistics tab shows CPI ≈ 1.0-1.5 depending on program
```

### Step 13: Add Preferences Dialog
- [ ] Menu → View → Preferences
- [ ] QDialog with:
  - [ ] Color scheme (Light/Dark radio buttons)
  - [ ] Execution speed slider (0-100%)
  - [ ] Font size (for code editor and displays)
  - [ ] Show/hide register aliases
- [ ] Save to QSettings (platform-specific config file)
- [ ] Restore on startup

**Test**:
```bash
./clearCore-gui
# View → Preferences → Dark mode
# Window recolors (main background → dark gray, text → white)
```

### Step 14: Keyboard Shortcuts
- [ ] F10 = Step one cycle
- [ ] F5 = Run
- [ ] Shift+F5 = Stop
- [ ] Ctrl+R = Reset
- [ ] Ctrl+O = Open program file
- [ ] Ctrl+S = Save trace to CSV
- [ ] Ctrl+? = Show shortcuts

Implement in `MainWindow::setupMenuBar()` using `QAction::setShortcut()`.

### Step 15: Breakpoints & Inspection
- [ ] Right-click on instruction in datapath → "Set Breakpoint"
- [ ] Datapath shows breakpoints as red circles on instruction addresses
- [ ] `Run` button stops when breakpoint hit
- [ ] Double-click instruction → Show details dialog (full decode, register values)

**Test**:
```bash
# Set breakpoint on instruction at 0x1000
# Click Run
# Execution halts when PC reaches 0x1000
# Signal breakpointHit() emitted
```

---

## Phase 4: Testing & Deployment (1 Week)

### Step 16: Unit Tests
- [ ] Create `tests/qt_ui_test.cpp`:
  - [ ] Test SimulatorController signal emission
  - [ ] Test RegisterWidget state tracking
  - [ ] Test MemoryWidget navigation
  - [ ] Test PipelineTraceWidget timeline
- [ ] Run: `ctest --output-on-failure`

### Step 17: Integration Testing
- [ ] Test full workflow:
  1. [ ] Load 10-instruction program
  2. [ ] Step through all cycles
  3. [ ] Verify all widgets update correctly
  4. [ ] Export trace to CSV
  5. [ ] Reset and reload program

### Step 18: Cross-Platform Testing
- [ ] Build on Linux (Fedora 44) ✓
- [ ] Build on macOS (if possible)
- [ ] Build on Windows (GitHub Actions CI/CD)

### Step 19: Documentation
- [ ] Update README.md with Qt6 GUI section
- [ ] Create user guide (keyboard shortcuts, menu reference)
- [ ] Add screenshots to GitHub wiki

### Step 20: Package & Release
- [ ] Update CHANGELOG.md
- [ ] Tag release: `v1.1.0-qt6`
- [ ] Upload release artifacts (Linux AppImage, macOS DMG, Windows MSI)

---

## Estimated Effort

| Phase | Duration | Deliverable |
|-------|----------|-------------|
| 1. Foundation | 1 week | `clearCore-gui` binary, static datapath, signals flowing |
| 2. Visualization | 2 weeks | Interactive pipeline, register, memory, trace displays |
| 3. Polish | 1 week | Code editor, statistics, preferences, breakpoints |
| 4. Testing | 1 week | Unit/integration tests, cross-platform builds, docs |
| **Total** | **5 weeks** | Production-ready GUI |

---

## Key Files to Create (Summary)

1. **CMakeLists.txt** — Build config (update existing)
2. **include/nsc_qt/simulator_controller.h** ✓
3. **src/nsc_qt/simulator_controller.cpp** ✓
4. **include/nsc_qt/main_window.h** ✓
5. **src/nsc_qt/main_window.cpp** — Implement setup methods
6. **src/nsc_qt/main.cpp** ✓
7. **include/nsc_qt/widgets/datapath_widget.h** ✓
8. **src/nsc_qt/widgets/datapath_widget.cpp** — Implement rendering
9. **include/nsc_qt/widgets/register_widget.h** ✓
10. **src/nsc_qt/widgets/register_widget.cpp** — Implement grid
11. **include/nsc_qt/widgets/memory_widget.h** ✓
12. **src/nsc_qt/widgets/memory_widget.cpp** — Implement hex dump
13. **include/nsc_qt/widgets/pipeline_trace_widget.h** ✓
14. **src/nsc_qt/widgets/pipeline_trace_widget.cpp** — Implement timeline

**Total: ~4000 lines of code**

---

## Pro Tips

1. **Start with mock data**: Before connecting to `SimulatorController`, populate widgets with hardcoded test data. This lets you develop UI independently.

2. **Use Qt Designer** (optional): For complex layouts (register grid), you can design in Qt Designer and export `.ui` files. Then use `#include "ui_register_widget.h"`.

3. **Profile rendering**: Use `QElapsedTimer` to measure paintGL() time. Aim for <5ms per frame (60 FPS).

4. **Version Qt6 early**: Pin Qt6 version in CMakeLists.txt:
   ```cmake
   find_package(Qt6 6.4 COMPONENTS Core Gui Widgets REQUIRED)
   ```

5. **Use Qt Creator IDE** (free): Download from qt.io. It has built-in Qt debugger and Qt Designer.

---

Good luck! Start with Phase 1, get the window opening and signals flowing, then iterate.
