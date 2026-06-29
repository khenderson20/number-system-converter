
# 🗺️ Roadmap: MIPS CPU Emulator

This project's architecture is heavily influenced by academic patterns (Ripes/DrMIPS, WebRISC-V) and published literature on computer organization. References include Harris & Harris, *Digital Design and Computer Architecture* (MIPS edition), Patterson & Hennessy, *Computer Organization and Design*, and scholarly works: Ripes (Petersen, 2021), DrMIPS (Nova et al., 2013), WebRISC-V (Giorgi & Mariotti, 2024).

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
* [ ] Memory panel (hex dump, PC-highlighted address)
* [ ] Adjustable run speed
* [ ] Instruction panel (disassembled fields: opcode, rs, rt, rd, shamt, funct)

### Stage 3 — Assembler
* [ ] Two-pass assembler with symbol table
* [ ] Label resolution for branches and jumps
* [ ] Pseudo-instructions (li, move, la, nop)
* [ ] Inline editor or file loader in TUI

### Stage 4 — Advanced Visualizations (Arches, WebRISC-V patterns)
* [ ] Instruction × cycle grid (WebRISC-V): show which instruction occupies each stage per cycle
* [ ] Per-module telemetry layer (stall type logging, CPI readout)
* [ ] Cycle-level stall counters (data hazard, control hazard, load-use)

### Stage 5 — Branch Prediction (PSBE / Pipelined MIPS Simulation pattern)
* [ ] 1-bit and 2-bit saturating predictors
* [ ] Branch Target Buffer (BTB)
* [ ] Misprediction flush visualization

### Stretch Goals
* [ ] Configurable datapath from external description file (DrMIPS pattern)
* [ ] RISC-V backend behind the same visualizer (Ripes pattern)
* [ ] Memory-mapped I/O (console output)
* [ ] Breakpoints and watchpoints
* [ ] Cycle counters and CPI analysis
* [ ] Reverse execution with snapshots (DrMIPS feature)