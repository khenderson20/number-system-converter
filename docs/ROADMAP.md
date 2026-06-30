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

### ✅ Stage 2 — TUI Execution Visualizer & CPU Switcher
* [x] CPU mode switcher (select single-cycle or pipelined at runtime)
* [x] Register file panel (all 32 registers, last-written highlighted, signed-decimal annotation)
* [x] Datapath panel (PC, current instruction, cycle counter, CPU mode) — `Datapath View` tab: IF fetch, ID register reads, EX/MEM/WB control signal snapshot
* [x] Pipeline visualization (WebRISC-V style: IF/ID/EX/MEM/WB state per cycle, forwarding paths, hazard flags) — `render_pipeline()`
* [x] **Memory panel** — hex dump, PC-highlighted row, halt-idiom detection, PgUp/PgDn/Home-to-PC navigation (`render_memory()`)
* [x] **Instruction panel** — disassembled fields (opcode, rs, rt, rd, shamt, funct) with per-field color coding, plus reconstructed assembly text and raw binary breakdown (`render_instr_decode()`, `reconstruct_asm()`)
* [x] **Adjustable execution speed** — single-step, toggleable auto-run, `Run→Halt`, and a speed slider (10–1000ms/cycle)
* [x] **Hazard visualization** (Arches-style) — stall/forward/flush badges inline in the pipeline strip, plus a dedicated hazard & forwarding status panel
* [x] **Run speed controls** — step / auto-run / run-to-halt / reset, all wired to telemetry reset

Beyond original scope, also implemented during this stage: an 8-entry execution trace panel (last committed instructions, WB-stage sourced), a telemetry panel with live gauges for stalls/forwards/flushes and a running CPI readout, an oscilloscope-style startup splash animation, and an ambient "pipeline flow" canvas strip that animates only while auto-run is active.

**Recently fixed (see commit history):** a `Container::Tab` focus-routing defect where tabs 4–5 silently aliased onto tabs 0–1's interactive components via index wraparound (confirmed against FTXUI's `ActiveChild()` source), and emoji in layout-critical text (tab bar, header) that FTXUI's `IsFullWidth()` table doesn't classify as double-width, causing terminal-dependent border misalignment.

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
* [~] **Per-Stage Telemetry** (Arches pattern): stall-type logging as first-class visualization — counters for stalls/forwards/flushes already live in the Stage 2 dashboard (`tel_cycles`, `tel_stalls`, `tel_forwards`, `tel_flushes`); remaining work is per-instruction-type breakout, not just per-cycle totals
* [~] **CPI Analysis**: running CPI is already computed and displayed (dashboard telemetry panel); remaining work is per-instruction-type breakout and historical tracking across runs
* [ ] **Performance Summary Panel**: total cycles, total instructions, CPI readout, hazard distribution pie chart (current gauges are live/per-session only, not a post-run summary view)
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

---

## Suggested next steps

In rough priority order, based on the actual state of `src/` and `include/` as of this update:

1. **Start Stage 3 (Assembler).** This is the next genuinely unstarted milestone — there is no `assembler.h`/`.cpp` anywhere in the tree yet, and Stage 2 is now functionally complete. Recommend scoping the first increment narrowly: a single-pass assembler covering only the R/I-format instructions `Decoder` already supports (no labels, no pseudo-instructions, no directives), emitting the same flat `std::vector<uint32_t>` that `load_hex_file()` consumes today. That gets a working `assembler_test.cpp` into `tests/mips/` (maintaining the project's src/tests parity convention) before the harder parts — label resolution for forward/backward branches, pseudo-instruction expansion, `.data`/`.text` directives — are layered on.

2. **Decide the fate of the "Utility Tools" tab.** It's currently a static placeholder with no content and no focus target (the `util_focus` container intentionally absorbs input and does nothing). Either remove it from `tab_labels` until there's real content, or — more useful — earmark it now as the home for Stage 3's inline `.asm` editor and syntax-error panel, so the assembler work has a UI destination from day one instead of bolting on a seventh tab later.

3. **Promote the existing telemetry into Stage 4's "Performance Summary Panel."** The live gauges in the Stage 2 dashboard (cycles/stalls/forwards/flushes/CPI) are per-session running counters, not a post-run summary. The smallest next step toward Stage 4 is capturing a snapshot of those counters on halt (CPU hits `StepResult::Halt`) and rendering it as a static end-of-run report, before tackling the heavier Instruction × Cycle Grid or Squashed Loops visualizations.

4. **Add a regression test for the tab/focus fix.** `processor_test.cpp` already runs both CPU backends through the same programs to catch behavioral divergence — there's no equivalent guard for UI wiring (e.g. asserting `Container::Tab`'s child count matches `tab_labels.size()` at construction, via a `static_assert`-style runtime check or a small UI smoke test). Worth a one-line guard so a future tab addition can't silently reintroduce the index-wraparound bug.