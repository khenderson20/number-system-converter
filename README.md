<div align="center">
  <img src="assets/logo.jpeg" alt="ClearCore logo" width="480"/>
</div>

<div align="center">

```
 ██████╗██╗     ███████╗ █████╗ ██████╗  ██████╗ ██████╗ ██████╗ ███████╗
██╔════╝██║     ██╔════╝██╔══██╗██╔══██╗██╔════╝██╔═══██╗██╔══██╗██╔════╝
██║     ██║     █████╗  ███████║██████╔╝██║     ██║   ██║██████╔╝█████╗
██║     ██║     ██╔══╝  ██╔══██║██╔══██╗██║     ██║   ██║██╔══██╗██╔══╝
╚██████╗███████╗███████╗██║  ██║██║  ██║╚██████╗╚██████╔╝██║  ██║███████╗
 ╚═════╝╚══════╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝
```

**An interactive simulator of a MIPS32 CPU core, written in C++20, with swappable single-cycle and 5-stage pipelined microarchitecture models — and two front ends: an FTXUI terminal UI and a Qt6 desktop GUI.**

</div>

<div align="center">

![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![FTXUI](https://img.shields.io/badge/TUI-FTXUI%20v7.0.0-2AA198?style=flat-square)
![Qt6](https://img.shields.io/badge/GUI-Qt6-41CD52?style=flat-square&logo=qt&logoColor=white)
![Tests](https://img.shields.io/badge/tests-95%2F95%20passing-859900?style=flat-square)
![MIT License](https://img.shields.io/badge/license-MIT-268BD2?style=flat-square)
![Build](https://img.shields.io/badge/build-CMake%20%2B%20FetchContent-657B83?style=flat-square)
![Linux](https://img.shields.io/badge/platform-Linux-073642?style=flat-square)

![ClearCore TUI demo](assets/demo_small.gif)
</div>

---

## What ClearCore is

ClearCore is a **CPU core simulator** for teaching computer architecture. It models the *internal* cycle-by-cycle behavior of a MIPS32 processor core — datapath stages, hazard detection, forwarding, stalls, and flushes — exactly as presented in the standard textbooks: Harris & Harris, *Digital Design and Computer Architecture* (single-cycle datapath and control), and Patterson & Hennessy, *Computer Organization and Design* (pipelining, hazards, forwarding).

Two microarchitecture models implement the same abstract `IProcessor` interface and can be swapped at runtime:

- **`SingleCycleCpu`** — the non-pipelined datapath of H&H Chapter 7. One instruction completes per cycle.
- **`PipelinedCpu`** — the classic 5-stage pipeline (IF, ID, EX, MEM, WB) of H&H Chapter 8 / P&H Chapter 4, including load-use stall detection, EX/MEM→EX and MEM/WB→EX forwarding, and branch/jump flushes.

Both models expose the same observable `PipelineState`, so pipeline visualization, hazard badges, and telemetry behave identically in both front ends. Correctness is enforced by a parity test harness that runs identical programs through both models and asserts matching architectural results.

**Simulator, not emulator.** ClearCore does not aim to run existing MIPS binaries. There is no syscall layer, no interrupt or exception handling, and no OS emulation — deliberately. The goal is to make the *microarchitecture* visible and inspectable, not to provide binary compatibility. If you need to boot Linux on a simulated MIPS machine, use QEMU; if you want to watch a load-use hazard stall the pipeline in real time, use ClearCore.

**Why MIPS in 2026?** The commercial MIPS ISA is retired — MIPS (the company) ended development of its proprietary architecture in 2021 to build RISC-V cores, and has since been acquired by GlobalFoundries. As a *teaching* ISA, however, MIPS remains the reference architecture of both Harris & Harris and Patterson & Hennessy (MIPS editions), and the one most computer-organization courses still draw their datapath figures from. ClearCore targets that pedagogical MIPS32 exactly, so what students see on screen matches what's in the book, figure for figure.

## 🖼️ Qt6 desktop GUI

The GUI is the fuller-featured way to work with ClearCore: a resizable window with a code editor, a hex memory viewer, and a click-through pipeline datapath, alongside everything the terminal UI offers.

<table>
<tr>
<td width="50%">

**Datapath**
<img src="assets/screenshots/tab-01-datapath.png" alt="Datapath tab showing the 5-stage pipeline">

</td>
<td width="50%">

**Registers**
<img src="assets/screenshots/tab-02-registers.png" alt="Registers tab showing all 32 MIPS registers">

</td>
</tr>
<tr>
<td width="50%">

**Memory**
<img src="assets/screenshots/tab-03-memory.png" alt="Memory tab showing a hex dump with ASCII column">

</td>
<td width="50%">

**Pipeline Trace**
<img src="assets/screenshots/tab-04-pipeline.png" alt="Pipeline trace tab showing an instruction x cycle grid">

</td>
</tr>
<tr>
<td width="50%">

**Code Editor**
<img src="assets/screenshots/tab-05-codeEditor.png" alt="Code editor tab with an assembled and loaded MIPS program">

</td>
<td width="50%">

**Statistics**
<img src="assets/screenshots/tab-06-stats.png" alt="Statistics tab showing cycles, CPI, hazards, and forwarding counts">

</td>
</tr>
</table>

**What each tab does:**

| Tab                | Purpose                                                                                                                                                                                                |
|--------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Datapath**       | The 5-stage pipeline (IF/ID/EX/MEM/WB) rendered live. Double-click a stage for a full decode of the instruction sitting in it; right-click to set or clear a breakpoint on that instruction's address. |
| **Registers**      | All 32 registers with ABI aliases (`$t0`, `$sp`, …), updated every cycle.                                                                                                                              |
| **Memory**         | A scrollable hex dump (16 bytes/row with an ASCII column) from any base address you enter.                                                                                                             |
| **Pipeline Trace** | An instruction × cycle grid — the classic pipeline diagram from Patterson & Hennessy, generated from your program's actual execution instead of drawn by hand.                                         |
| **Code Editor**    | Write MIPS assembly directly, or load one of the built-in example programs, then Assemble and Load it into the running simulator.                                                                      |
| **Statistics**     | Cycles, instructions retired, CPI, and per-category hazard/forwarding/stall/flush counts.                                                                                                              |

The built-in assembler covers the same instruction subset as the simulator core (`add`, `addi`, `lw`/`sw`, `beq`/`bne`, `j`/`jal`, shifts, and related instructions) plus labels, so branches and loops assemble without hand-computing offsets. It is intentionally *not* a full MIPS toolchain: pseudo-instructions (`li`, `move`, `la`), assembler directives, and data segments are not yet supported (see [Roadmap](#️-roadmap)).

### Building the GUI

The Qt6 GUI is built by default alongside the terminal UI:

```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug --target clearCore-gui
./cmake-build-debug/clearCore-gui
```

> ⚠️ **Qt6 is required by default.** `BUILD_QT6_UI` defaults to `ON`, so configuration will fail if Qt6 isn't installed. Either install it, or configure with the GUI turned off:
>
> ```bash
> # Install Qt6 (pick your platform)
> sudo dnf install qt6-qtbase-devel qt6-qtbase-gui   # Fedora/RHEL
> sudo apt install qt6-base-dev                      # Ubuntu/Debian
> brew install qt@6                                   # macOS
>
> # ...or skip the GUI entirely and build the TUI only
> cmake -S . -B cmake-build-debug -DBUILD_QT6_UI=OFF
> ```

## ✨ Key Features

- **Two swappable microarchitecture models** — toggle between the single-cycle and 5-stage pipelined cores at runtime, no rebuild required. Both implement `IProcessor` and are verified for architectural parity.
- **Cycle-by-cycle pipeline visualization** — all five stages rendered every cycle, with color-coded forwarding paths (EX→EX, WB→EX) and hazard badges (load-use stall, branch/jump flush) — in both the TUI and the Qt6 datapath view.
- **In-app assembler** — write or load MIPS assembly with labels, branches, and loops directly in the Qt6 Code Editor tab; no external toolchain needed for the supported subset.
- **Breakpoints & stage inspection** — set breakpoints from the Qt6 Datapath tab and step through a run instruction by instruction.
- **Telemetry & CPI** — running cycle counters, stall/forward/flush tallies, and a live CPI readout, shared by both front ends.
- **MIPS instruction decoding** — enter a raw 32-bit word and see its mnemonic, register fields, and binary breakdown decoded live.
- **Live number conversion** — the utility ClearCore grew out of: two-way conversion between binary, hex, and decimal, backed by a single `uint64_t` source of truth with input validation. Still useful when reading raw instruction encodings.
- **Signal Monitor** *(TUI only)* — an ambient oscilloscope panel that animates while the CPU runs.
- **Control & navigation** — the TUI is fully keyboard-driven (`Tab` to move between panels, `F10` to step, `Esc` to quit); the Qt6 GUI supports mouse and keyboard, including arrow-key navigation and Enter/Space shortcuts on the Datapath tab.

## 🖥️ Interfaces

ClearCore ships two independent front ends over the same `mips_core`/`nsc_core` libraries. Pick whichever fits your environment — or build both.

### Terminal UI (`number_system_converter`)

| # | Tab                | Purpose                                              |
|---|--------------------|------------------------------------------------------|
| 0 | **Converter**      | Live binary/hex/decimal conversion                   |
| 1 | **CPU Dashboard**  | Registers, pipeline stages, hazard badges, telemetry |
| 2 | **CPU Config**     | Switch between single-cycle and pipelined models     |
| 3 | **Program Loader** | Load a flat instruction-word program into memory     |
| 4 | **Signal Monitor** | Ambient oscilloscope animation during execution      |

### Qt6 desktop GUI (`clearCore-gui`)

See the [screenshot gallery and tab reference](#️-qt6-desktop-gui) above.

## 🚀 Quick Start

### Build

The TUI build is fully self-contained via CMake FetchContent (FTXUI v7.0.0) — no system-wide FTXUI install required. Qt6 is a separate system dependency (see [Building the GUI](#building-the-gui) above); if you don't want to install it, configure with `-DBUILD_QT6_UI=OFF` first.

```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug --target number_system_converter
```

### Run

> ⚠️ **Important:** FTXUI requires a real terminal to render correctly (it uses ANSI escape codes). If running from an IDE, enable "Emulate terminal in output console" or run directly from the shell.

```bash
./cmake-build-debug/number_system_converter
```

Start by entering `255` in DEC and watch HEX (`FF`) and BIN (`11111111`) update live — then switch to the CPU Dashboard tab and step a program through the pipeline.

Prefer the desktop GUI? Build and run `clearCore-gui` instead — see [Building the GUI](#building-the-gui).

### Testing

All 95 checks pass across the decoder, both CPU models, and the converter core, including the parity harness that runs identical programs through both microarchitectures. The Qt6 GUI has its own smoke-test suite (`qt_ui_test`), built only when `BUILD_QT6_UI` is on.

```bash
cmake --build cmake-build-debug --target decoder_test cpu_test processor_test nsc_tests qt_ui_test
ctest --test-dir cmake-build-debug --output-on-failure
```

## 🧠 Technical Architecture

The system is split into two decoupled core libraries plus two independent UI layers, wired together with CMake:

- **`mips_core`** — the processor simulation logic, behind the `IProcessor` interface. Pure logic, no UI dependency.
- **`nsc_core`** — the number system converter logic. Pure logic, no UI dependency.
- **`nsc_ui`** — FTXUI wiring for the terminal UI; depends only on the core libraries' public interfaces.
- **`nsc_qt`** — Qt6 widgets for the desktop GUI; likewise depends on `mips_core` only through `IProcessor`, via a `SimulatorController` bridge that owns the processor and re-emits its state as Qt signals for the widgets to render.

### Pluggable microarchitecture models

`IProcessor` is the contract between either UI and any execution model, following the pluggable-backend pattern established by Ripes. Both front ends drive both models identically:

| Model              | Reference                        | Behavior                                                                                             |
|--------------------|----------------------------------|------------------------------------------------------------------------------------------------------|
| `SingleCycleCpu`   | H&H Ch. 7                        | Non-pipelined datapath; one instruction per cycle                                                      |
| `PipelinedCpu`     | H&H Ch. 8 / P&H Ch. 4            | 5-stage pipeline with load-use stall detection, EX/MEM→EX and MEM/WB→EX forwarding, branch/jump flush |

### Core module responsibilities

| Module                  | Responsibility                                                                   | NSC Core | MIPS Core | Qt GUI |
|:------------------------|:---------------------------------------------------------------------------------|:--------:|:---------:|:------:|
| **Converter**           | Manages `uint64_t` state, exposes base views                                     |    ✅     |           |        |
| **Parser/Formatter**    | String validation and serialization across bases                                 |    ✅     |           |        |
| **IProcessor**          | Abstract interface for execution model + visualizer contract                     |          |     ✅     |        |
| **CPUs (SC/Pipe)**      | Datapath simulation logic                                                        |          |     ✅     |        |
| **Decoder / ALU**       | Instruction format detection, control signals, arithmetic/logic                  |          |     ✅     |        |
| **SimulatorController** | Owns an `IProcessor`, re-emits its state as Qt signals for widgets to render     |          |           |   ✅    |
| **In-app assembler**    | Parses MIPS assembly with labels into instruction words for the Code Editor tab  |          |           |   ✅    |

### Built with
- **Languages/tools:** C++20 (`std::format`, `std::optional`), FTXUI v7.0.0, Qt6 Widgets, CMake FetchContent.
- **Design conventions:** polymorphism via `IProcessor` keeps models swappable; `enum class` for hardware fields; `[[nodiscard]]` on pure queries; strict warning flags (`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror`).

## ⚠️ Scope and limitations

ClearCore is honest about what it doesn't do. If you need these, the [comparison table](#-how-clearcore-compares) points to tools that have them:

- **Instruction subset, not full MIPS32.** Core integer instructions, loads/stores, branches, and jumps. No FPU, no `mult`/`div` HI/LO pipeline, no coprocessor instructions.
- **No pseudo-instructions or assembler directives yet** (`li`, `move`, `.data`, etc.) — planned for Stage 3.
- **No syscalls, interrupts, or exceptions.** ClearCore programs run to completion in a flat address space.
- **No cache or branch-prediction modeling yet** — both are roadmap items (Stage 5+).
- **Textbook microarchitecture, not silicon.** The pipeline models are cycle-by-cycle implementations of the H&H/P&H teaching designs. They are not timing models of any fabricated MIPS core.
- **Linux-first.** Developed and tested on Linux; macOS builds are expected to work but are not CI-verified.

## 🌍 How ClearCore compares

ClearCore sits alongside established educational simulators, applying the Ripes pluggable-backend pattern to the pedagogical MIPS32 ISA — and it is the only tool in this set that ships both an interactive terminal UI and a native desktop GUI over the same core.

| Aspect            | ClearCore                    | Ripes               | DrMIPS              | EduMIPS64        | QtMips                        | WebRISC-V   |
|-------------------|------------------------------|---------------------|---------------------|------------------|-------------------------------|-------------|
| **Language**      | C++20                        | C++/Qt              | Java                | Java             | C++/Qt                        | PHP/JS      |
| **UI**            | FTXUI (TUI) **+** Qt6 (GUI)  | Qt (GUI, web build) | Swing (GUI, mobile) | Swing (GUI), CLI | Qt (GUI, web build)           | Web browser |
| **ISA**           | MIPS32 (teaching subset)     | RISC-V              | MIPS I              | MIPS64           | MIPS32                        | RISC-V      |
| **µarch models**  | 2 (single-cycle / 5-stage)   | 5+ models           | 2 (+ custom datapaths) | 1 (pipelined) | Configurable (SC / pipelined) | 1 (5-stage) |
| **Visualization** | Pipeline state + hazards     | Datapath schematic  | Datapath diagram    | Register/memory  | Datapath + cache              | Cycle grid  |
| **Status (2026)** | Active                       | Active              | Unmaintained        | Active           | Superseded by QtRVSim (RISC-V) | Active     |

Notably, both C++/Qt MIPS peers have moved on: QtMips's authors direct new work to its RISC-V successor QtRVSim, and DrMIPS has been unmaintained for years. That leaves ClearCore as an actively developed, modern-C++ simulator for the MIPS32 datapath that H&H and P&H (MIPS editions) actually teach.

Reference texts: **Harris & Harris**, *Digital Design and Computer Architecture* (single-cycle datapath, control signal generation), and **Patterson & Hennessy**, *Computer Organization and Design* (pipelining, hazards, forwarding).

## 🗺️ Roadmap

- ✅ **Stage 1** — Number converter core + MIPS decoder
- ✅ **Stage 1.5** — `IProcessor` refactor; single-cycle and pipelined models
- ✅ **Stage 2** — TUI execution visualizer (memory panel, instruction decode, hazard badges, speed controls, telemetry)
- ✅ **Stage 2.5** — Qt6 desktop GUI (datapath, registers, memory, pipeline trace, code editor with in-app assembler, statistics)
- ⬜ **Stage 3** — Full two-pass assembler: symbol table, label resolution, pseudo-instruction expansion *(the Qt6 Code Editor's assembler covers labels and branches already; pseudo-instructions and directives are outstanding)*
- ⬜ **Stage 4** — TUI parity for the instruction × cycle grid and per-stage telemetry *(the Qt6 Pipeline Trace and Statistics tabs already cover this on the GUI side)*
- ⬜ **Stage 5** — Branch prediction and speculative execution

See [docs/ROADMAP.md](docs/ROADMAP.md) for the full breakdown.

## 📄 Documentation

- **🚀 For learners:** [USER_GUIDE.md](docs/USER_GUIDE.md) — learn MIPS pipeline concepts through the visualization.
- **🧠 For developers:** [ARCHITECTURE_DESIGN.md](docs/ARCHITECTURE_DESIGN.md) — design patterns, hardware abstractions, and academic grounding.
- **🖼️ Qt6 GUI architecture:** [QT6_ARCHITECTURE.md](docs/QT6_ARCHITECTURE.md) — how `nsc_qt` and `SimulatorController` are structured.
- **⚙️ For contributors:** [CONTRIBUTING.md](docs/CONTRIBUTING.md) — branching model, code style, and testing guidelines.
- **🗺️ Roadmap:** [ROADMAP.md](docs/ROADMAP.md) — staged feature plan and reference patterns.

## 📄 License

MIT — see [LICENSE](LICENSE).
