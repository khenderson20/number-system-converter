
# 🗺️ Roadmap: MIPS CPU Emulator

This project's architecture is grounded in academic patterns and published literature on computer organization.

**Academic & Theoretical Foundation:**
- Harris & Harris, *Digital Design and Computer Architecture* (MIPS edition) — single-cycle datapath design fundamentals
- Patterson & Hennessy, *Computer Organization and Design* — pipeline architecture, hazards, forwarding logic
- Scholarly references: Ripes (Petersen, 2021), DrMIPS (Nova et al., 2013), WebRISC-V (Mariotti & Giorgi, 2022), Arches (Haydel et al., 2025)

**Design Patterns Adopted:**
1. **Pluggable-Backend Pattern** (Ripes): abstract `IProcessor` interface enables multiple ISA implementations
2. **Observable Pipeline State** (WebRISC-V + Arches): forwarding/stall flags as first-class visualization elements
3. **TUI-First Educational Focus** (Unique): FTXUI-based approach vs. GUI/web competitors for systems-level teaching
4. **Test-Driven Architecture Verification** (Rigorous): polymorphic test harness ensures behavioral parity across backends

### ✅ Stage 0 — Foundations
* [x] Instruction decoder (R/I/J formats, opcode + funct → mnemonic)
* [x] ALU control block + execute (arithmetic, logic, shifts, overflow flags)
* [x] Live MIPS decode in converter UI
* [x] RegisterFile (32 × uint32_t, `$zero` hardwired)
* [x] Memory (byte-addressable, word/half/byte access, alignment)

### ✅ Stage 1 — Single-Cycle Datapath
* [x] CPU: fetch → decode → control → execute → memory → writeback in one cycle
* [x] Control unit signals (reg_write, mem_read, mem_write, branch, jump, alu_src, etc.)
* [x] ISA subset: arithmetic, logic, shifts, loads, stores, branches, jumps
* [x] Program loader for flat instruction arrays
* [x] Unit tests (38 checks on known programs)

### ✅ Stage 1.5 — IProcessor Refactor & Pipelined Backend
* [x] Abstract `IProcessor` interface (Ripes/DrMIPS pattern)
* [x] `PipelineState` observable (IF/ID/EX/MEM/WB snapshots, forwarding flags, hazard indicators)
* [x] SingleCycleCpu: old logic now implements IProcessor
* [x] PipelinedCpu: full 5-stage pipeline
* [x] Forwarding unit (EX/MEM → EX, MEM/WB → EX)
* [x] Hazard detection (load-use stall, branch/jump flush)
* [x] Polymorphic test harness (95 checks, both implementations)
* [x] Backward-compat shim (`using Cpu = SingleCycleCpu`)

### 🟡 Stage 2 — TUI Execution Visualizer & CPU Switcher
* [x] CPU mode switcher (select single-cycle or pipelined at runtime)
* [x] Register file panel (all 32 registers, last-written highlighted)
* [x] Datapath panel (PC, current instruction, cycle counter, CPU mode)
* [x] Pipeline visualization (WebRISC-V style: IF/ID/EX/MEM/WB state per cycle, forwarding paths, hazard flags)
* [ ] **Memory panel** (hex dump, PC-highlighted address, configurable view range)
* [ ] **Instruction panel** (disassembled fields: opcode, rs, rt, rd, shamt, funct with color coding)
* [ ] **Adjustable execution speed** (cycle stepping, auto-advance modes)
* [ ] **Hazard visualization** (Arches-style: stall indicators, forwarding wire highlights, flush markers)
* [ ] **Run speed controls** (single-step, auto-run, breakpoint capability for Stage 3)

**Reference Pattern (WebRISC-V):** Implement "Full Loops" visualization mode showing instruction progression across pipeline stages, with cycle-by-cycle data flow traces. Include forwarding wire annotations (EX/MEM→EX, MEM/WB→EX) and load-use stall bubbles.

### Stage 3 — Assembler & Program Composition
* [ ] **Two-pass assembler** with symbol table (EduMIPS64 pattern)
* [ ] **Label resolution** for branches (BEQ/BNE forward/backward) and jumps (J/JAL)
* [ ] **Pseudo-instructions** (li, move, la, nop) with expansion rules
* [ ] **Directive support** (.word, .ascii, .data, .text sections)
* [ ] **Inline editor** in TUI or .asm file loader (following EduMIPS64/QtMips design)
* [ ] **Syntax validation** with line-by-line error reporting
* [ ] **Symbol export** for debugging/inspection in Stage 2 visualizer

**Reference Pattern (EduMIPS64):** GNU assembler directives, case-insensitive mnemonics, and forward-reference support. Validation errors should report line numbers and invalid tokens clearly. Output: flat instruction array loadable directly into Stage 2.

### Stage 4 — Advanced Visualizations & Telemetry
* [ ] **Instruction × Cycle Grid** (WebRISC-V pattern): visual table showing which instruction occupies each stage per cycle
* [ ] **Per-Stage Telemetry** (Arches pattern): stall-type logging as first-class visualization
    - Data hazard counters (load-use stalls)
    - Control hazard counters (branch/jump flushes)
    - Forwarding event counters (bypasses performed)
* [ ] **CPI Analysis** (Cycles Per Instruction): running average, per-instruction breakout
* [ ] **Performance Summary Panel**: total cycles, total instructions, CPI readout, hazard distribution pie chart
* [ ] **Cycle-by-Cycle Breakdown**: annotate each cycle with reason (normal advance, stall type, or flush type)
* [ ] **"Squashed Loops" Mode** (WebRISC-V): compact visualization for repetitive loop execution

**Reference Pattern (Arches + WebRISC-V):** Make telemetry data exportable for analysis. Provide both visual and tabular representations. Arches demonstrates that forwarding/stall flags should be observable first-class citizens in the pipeline state.

### Stage 5 — Branch Prediction & Speculative Execution
* [ ] **Predictors**: 1-bit, 2-bit saturating counters (Patterson & Hennessy §4.7)
* [ ] **Branch Target Buffer (BTB)**: associative cache of recent branch targets
* [ ] **Return Address Stack**: for function call/return prediction
* [ ] **Misprediction Visualization**: show flushed instructions in pipeline with "prediction wrong" annotation
* [ ] **Prediction Accuracy Metrics**: correct/incorrect counts, per-branch breakdown
* [ ] **Pattern Analyzer**: identify branch correlations (if-after-if patterns, loop-invariant conditions)

**Reference Pattern (SimpleScalar):** Implement at increasing levels of sophistication (always-taken → saturating counter → BTB-backed). Each level should be selectable for teaching. Visualization should highlight both correct predictions and recoveries from mispredictions.

### Stretch Goals & Future Directions

**Core Extensibility:**
* [ ] **RISC-V Backend** (Ripes pattern): implement IProcessor for 32/64-bit RISC-V, reuse all existing visualizers
* [ ] **Configurable Datapath** (DrMIPS pattern): load processor config from JSON/YAML to swap component implementations
* [ ] **Cache Simulator** (EduMIPS64 + Dinero pattern): configurable L1/L2, replacement policies, coherence

**System Integration (QtMips pattern):**
* [ ] **Memory-Mapped I/O**: console output via syscalls, breakpoint support
* [ ] **Linux ABI Support** (O32 for MIPS): syscall emulation (read, write, exit) for running real programs
* [ ] **WebAssembly Build** (QtMips + Ripes): port to WASM for browser deployment

**Advanced Educational:**
* [ ] **Breakpoints & Watchpoints**: pause on address/register changes (DrMIPS feature)
* [ ] **Reverse Execution** (DrMIPS feature): undo previous cycles for analysis
* [ ] **Snapshot/Restore**: save and reload execution states for comparison
* [ ] **Performance Regression Tests**: catch architecture changes that hurt CPI

**Academic Publication Path:**
* [ ] Submit Stage 2 completion to **ACM WCAE** (Workshop on Computer Architecture Education)
* [ ] Publish in **IEEE Transactions on Education** when full pipeline + telemetry complete
* [ ] Cross-cite with Ripes, WebRISC-V, Arches in academic community