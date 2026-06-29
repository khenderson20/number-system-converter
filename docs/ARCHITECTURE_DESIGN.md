
# 🧠 Architectural Deep Dive

This document explains the core design choices in ClearCore, focusing on how we map academic concepts from Patterson & Hennessy and Harris & Harris into modern C++20 components, while positioning the project within the broader ecosystem of educational CPU simulators.

## 🏛️ Design Philosophy: Three Core Pillars

### 1. Separation of Concerns
The system splits cleanly into three independent libraries:
- **`nsc_core`**: Number System Converter (pure logic, zero UI dependencies)
- **`mips_core`**: MIPS CPU Emulator (architecture simulation, zero UI dependencies)
- **`nsc_ui`**: FTXUI Frontend (orchestrates the above, reactive to user input)

This decoupling enables:
- Unit testing of conversion and CPU logic without a running TUI
- Swapping UI implementations (TUI → GUI → Web) without touching core logic
- Multiple bindings to the same core (CLI tools, headless servers, etc.)

### 2. Pluggable-Backend Architecture (Ripes/DrMIPS Pattern)
The **`IProcessor`** abstract interface is the cornerstone design feature:
```cpp
class IProcessor {
  virtual StepResult step() = 0;
  virtual const PipelineState& pipeline_state() const = 0;
  virtual const RegisterFile& regs() const = 0;
  // ... other accessor methods
};
```

This pattern, pioneered by Ripes (Petersen, 2021) and DrMIPS, enables:
- **Multiple backends** behind a single interface (currently: `SingleCycleCpu`, `PipelinedCpu`)
- **Future ISA support** (RISC-V, ARM) without changing the visualizer
- **Behavioral parity testing** (polymorphic harness validates both implementations identically)
- **Easy swapping at runtime** (users toggle CPU mode without losing state)

### 3. Observable Pipeline State (Arches + WebRISC-V Pattern)
The **`PipelineState`** struct exposes all five pipeline stages and hazard indicators as first-class visualization elements:
```cpp
struct PipelineState {
  std::array<StageSnapshot, 5> stages;  // IF, ID, EX, MEM, WB
  bool fwd_ex_to_ex_a, fwd_ex_to_ex_b;  // Forwarding flags
  bool fwd_mem_to_ex_a, fwd_mem_to_ex_b;
  bool load_stall, branch_flush;        // Hazard indicators
  std::size_t cycle;
};
```

This design formalizes what Arches (Haydel et al., 2025) discovered: **forwarding and stall flags should be observable, not hidden implementation details**. The TUI can render these directly:
- Color-code pipeline stages by state (valid, stalled, flushed)
- Draw forwarding wire annotations (EX/MEM→EX paths)
- Highlight which cycle stalls/flushes occur and why

## 📚 Reference Textbooks & Academic Grounding
The theoretical foundation is built upon industry standards and scholarly works:

* **Computer Organization and Design** (Patterson & Hennessy): Modern pipeline techniques, hazard handling, forwarding logic, branch prediction
* **Digital Design and Computer Architecture** (Harris & Harris): Single-cycle datapath fundamentals, control signal generation, ALU design

### Academic Influences
- **Ripes (Petersen, 2021)**: Pluggable-backend pattern for multiple ISAs
- **DrMIPS (Nova et al., 2013)**: Visual pipeline state and register highlighting
- **WebRISC-V (Mariotti & Giorgi, 2022)**: Cycle-by-cycle pipeline visualization, "squashed loops" mode
- **Arches (Haydel et al., 2025)**: Forwarding/stall flags as observable first-class elements
- **SimpleScalar (Austin et al., 1994-2003)**: Multi-level simulator philosophy (functional → cache → out-of-order)

## 🌍 Ecosystem Positioning: Where We Fit

| Aspect              | ClearCore                | Ripes                     | DrMIPS           | EduMIPS64            | QtMips            | WebRISC-V         | Arches                   |
|---------------------|--------------------------|---------------------------|------------------|----------------------|-------------------|-------------------|--------------------------|
| **Language**        | C++20                    | C++/Qt                    | Java             | Java                 | C++/Qt            | PHP/JS            | C++                      |
| **UI**              | FTXUI (TUI)              | Qt (GUI)                  | Swing (GUI)      | Swing (GUI)          | Qt (GUI)          | Web Browser       | CLI/Logs                 |
| **ISAs**            | MIPS (extensible)        | RISC-V                    | MIPS             | MIPS64               | MIPS              | RISC-V            | RISC-V                   |
| **Backends**        | 2 (SC/5-stage)           | 5+ models                 | ~2               | ~1                   | ~1                | ~1                | ~1                       |
| **Visualization**   | Pipeline state + hazards | Datapath schematic        | Visual datapath  | Register/Memory      | Datapath + Memory | Cycle grid        | Telemetry logs           |
| **Syscalls**        | Planned (Stage 3+)       | None                      | None             | Partial              | Yes (O32 ABI)     | I/O only          | Yes                      |
| **Publication**     | WCAE target              | WCAE'21 ✓                 | —                | IEEE Trans. on Ed. ✓ | —                 | SoftwareX ✓       | CGF/HPG 2025 ✓           |
| **Unique Strength** | Minimal deps, pure C++20 | RISC-V focus, FPGA bridge | Portable, visual | Long-term validated  | Full ABI support  | Web accessibility | Research-grade telemetry |

### Our Competitive Advantages

1. **Minimal Dependency Stack**: Just FTXUI + C++20 stdlib. No Qt, Java VM, or PHP/Node backend required.
2. **Pure C++20 Implementation**: Modern idioms (`std::optional`, `std::variant`, concepts) make the code readable and maintainable.
3. **TUI-First Approach**: Unique in the ecosystem. Most simulators default to GUI (Qt, Swing) or web. TUI is lighter, scriptable, and excellent for teaching terminals/shells.
4. **Integrated Narrative**: "Number converter → CPU emulator" is a cohesive educational story. Users learn base representation, then see bits in hardware. Most simulators start isolated.
5. **Rigorous Test-Driven Design**: 95 passing tests with polymorphic harness. Most educational simulators skip this level of validation.
6. **Explicitly Academic**: We cite our sources (H&H, P&H, Ripes, Arches). Code reflects published research, not ad hoc design.

### Planned Differentiation (Stages 2–5)

- **Stage 2**: WebRISC-V-style pipeline visualization + Arches-style hazard telemetry, all in TUI
- **Stage 3**: EduMIPS64-inspired assembler with full GNU directives + symbol export
- **Stage 4**: Arches-level per-stage telemetry (stall counters, CPI analysis) in TUI
- **Stage 5**: SimpleScalar-style branch prediction with visual prediction accuracy tracking



## 🔄 MIPS Implementation Details
### Single-Cycle vs. Pipelined Models
*   **SingleCycleCpu:** This serves as a behavioral model, simulating all stages (Fetch $\to$ Decode $\to$ Execute $\to$ Memory $\to$ Writeback) within a single function call (`step()`). While computationally inefficient in reality, it provides the clearest initial demonstration of data path flow. It implements the core functionality outlined in **Harris & Harris** for basic instruction execution.
*   **PipelinedCpu:** This model implements concurrent operations using five dedicated pipeline registers (IF/ID, ID/EX, EX/MEM, MEM/WB). The design follows best practices from **Computer Organization and Design** concerning stage boundaries:
    *   **Hazard Detection Unit:** Responsible for identifying data hazards (e.g., Load-Use) by comparing register file read addresses (`rs`, `rt`) against the write address of subsequent stages. When a hazard is detected, the pipeline inserts stalls or flushes instructions.

### Control Path Implementation
The control logic (`derive_control`) is a centralized state machine mapping the combination of the opcode and function code to all necessary hardware signals (e.g., `mem_read`, `alu_src`). This centralization prevents duplicated switch tables between the single-cycle and pipelined implementations, enforcing modularity across both backends.

## 💡 Extensibility Roadmap & Academic Vision

The architecture is designed for progressive enhancement following academic patterns:

### Immediate Extensibility (Stage 2–3)

**IProcessor Interface**: Any new ISA (RISC-V, ARM, x86) can be plugged in by implementing:
```cpp
class MyNewArchCpu : public IProcessor {
  StepResult step() override;
  const PipelineState& pipeline_state() const override;
  // ... 
};
```
The existing TUI, assembler, and all visualizations work without modification.

**Control Signal Derivation**: The `derive_control()` function is the only place to add new instruction types. This centralizes control logic and prevents duplication between backends.

### Medium-Term Goals (Stage 4–5)

Build toward the **SimpleScalar philosophy**: layer functional → cache-aware → performance-predictive simulation levels. Users can toggle between:
- **Functional mode**: Fast, ignores memory hierarchy
- **Cache-aware mode**: Realistic memory timings
- **Predictive mode**: Full microarchitectural detail (hazards, branch prediction, out-of-order effects)

### Publication & Academic Integration

**Near-term (Stage 2):**
- Submit to **ACM WCAE** (Workshop on Computer Architecture Education) — pluggable-backend + TUI pattern
- Target **IEEE Transactions on Education** when pipeline visualization completes

**Medium-term (Stage 3–4):**
- Cross-cite with Ripes, WebRISC-V, Arches in academic community
- Engage FTXUI ecosystem (first major TUI CPU simulator)
- Add RISC-V backend; position as "Ripes for TUI"

**Long-term (Stage 5):**
- Research paper on branch prediction + visualizer
- Integration with HPC/architecture curriculum materials