# 🔢 Number System Converter → MIPS CPU Emulator

A modern C++20 project that began as a snappy terminal UI for converting numbers between **binary**, **hexadecimal**, and **decimal** — live, as you type — and is growing into a **single-cycle (and eventually pipelined) MIPS CPU emulator** with a live datapath visualizer. Built with [FTXUI](https://github.com/ArthurSonzogni/FTXUI).

![demo.png](assets/demo.png)
![demo-2.png](assets/demo-2.png)

## ✨ Features

- **Live two-way conversion** — edit any field and the others update instantly.
- **Input validation** — each field only accepts characters valid for its base (`0`/`1`, `0`-`9`, `0`-`9a`‑`fA`‑`F`).
- **Nibble-grouped bit view** — see the binary layout grouped into 4-bit chunks for readability.
- **Live MIPS decode** — enter a 32-bit value in hex and see its decoded MIPS mnemonic inline.
- **Keyboard-driven** — `Tab` to move between fields, `Esc`/`Ctrl`-`C` to quit.
- **64-bit range** — backed by a single `uint64_t` source of truth.

## 🚀 Quick start

### Build

The build is fully self-contained — CMake fetches FTXUI v7.0.0 for you.

```bash
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug --target number_system_converter
```

### Run

> ⚠️ **Run it in a real terminal.** FTXUI draws with ANSI escape codes and needs a
> TTY. Inside CLion's *Run* tool window you'll see garbled border fragments — either
> enable **"Emulate terminal in output console"** in the Run Configuration, or just
> run it from a terminal:

```bash
./cmake-build-debug/number_system_converter
```

Type `255` in the DEC field and watch HEX become `FF` and BIN become `11111111`. 🎉

### Test

```bash
cmake --build cmake-build-debug --target decoder_test nsc_tests
ctest --test-dir cmake-build-debug --output-on-failure
```

## 🧠 How it works

A single `uint64_t value` is the **source of truth**; each base is just a *view* of it.

![how-it-works.png](assets/how-it-works.png)

Editing a field parses it into `value`, then re-renders the *other* fields. A
`syncing` re-entrancy guard prevents the programmatic rewrites from cascading into
an infinite `on_change` loop.

## 📦 Project layout

The codebase is split into independently testable libraries with no UI code in the
core. This separation is what makes the emulator expansion tractable — each hardware
component is a pure library that can be unit-tested in isolation before it's ever
wired into the TUI.

```
include/
  nsc/      converter, parse, format    — number-system conversion
  mips/     decoder, alu, ...           — CPU emulator components
src/
  nsc/      conversion + FTXUI frontend
  mips/     CPU emulator implementation
tests/
  nsc/      convert_test
  mips/     decoder_test, ...
```

### nsc_core

| Module        | Responsibility                                                                          |
| ------------- | --------------------------------------------------------------------------------------- |
| **converter** | Orchestrator: manages conversion between bases, provides `as()` views and bit grouping. |
| **parse**     | String-to-uint64 parser with base support; validates inputs before committing to state. |
| **format**    | Serializer for binary (expand on zero), hex (`%X`), decimal, and nibble groups.         |

### mips_core

| Module        | Responsibility                                                                          |
| ------------- | --------------------------------------------------------------------------------------- |
| **decoder**   | Splits a 32-bit word into R/I/J fields; maps opcode + funct to mnemonics.               |
| **alu**       | ALU control block + execute: arithmetic, logic, shifts, comparisons, overflow flags.    |

## 🛠️ Built with

- **C++20**: `std::format`, `std::optional`, `std::variant`, `std::string_view`, designated initializers
- **FTXUI v7.0.0**: For the reactive UI layer (screen, dom, component)
- **CMake FetchContent**: Declarative dependency fetching

---

## 🗺️ Roadmap: MIPS CPU Emulator

The long-term goal is a **pipelined MIPS emulator with a live TUI datapath
visualizer** — register state, pipeline stages, hazard stalls, and instruction
encodings, all updating in real time. The work is staged so each milestone produces
something runnable.

References: Harris & Harris, *Digital Design and Computer Architecture* (MIPS edition),
and Patterson & Hennessy, *Computer Organization and Design*.

### ✅ Stage 0 — Foundations *(in progress)*

- [x] Instruction `decoder` — R/I/J formats, opcode + funct → mnemonic
- [x] `alu` — control block and execute with overflow/zero/negative flags
- [x] Live MIPS decode in the converter UI
- [ ] `registers.hpp` — 32 × `uint32_t` register file with `$zero` hardwired
- [ ] `memory.hpp` — byte-addressable RAM with word/half/byte access helpers

### Stage 1 — Single-cycle datapath

Run a real (if small) program end to end.

- [ ] `cpu` — fetch → decode → execute → memory → writeback in one tick
- [ ] Control unit deriving `RegWrite`, `MemRead`, `MemWrite`, `Branch`, `ALUSrc`
- [ ] ISA subset: `add`, `sub`, `and`, `or`, `slt`, `addi`, `lw`, `sw`, `beq`, `j`
- [ ] Program loader for a flat array of instruction words
- [ ] `cpu_test` — verify known programs reach expected register/memory state

### Stage 2 — TUI execution visualizer

The differentiator. Most emulators are a CLI loop; this one *shows* the machine.

- [ ] Register file panel — all 32 registers, last-changed highlighted
- [ ] Memory panel — hex dump with the PC-pointed address highlighted
- [ ] Instruction panel — current instruction disassembled, fields shown
  (`opcode | rs | rt | rd | shamt | funct`) reusing the nibble grouper
- [ ] Step / Run / Reset controls, keyboard-driven
- [ ] Adjustable run speed for watching execution unfold

### Stage 3 — Assembler

Stop hand-encoding hex. Write MIPS assembly, run it.

- [ ] Two-pass assembler: build symbol table, then encode
- [ ] Label resolution for branches and jumps
- [ ] Pseudo-instructions (`li`, `move`, `la`, `nop`)
- [ ] Inline editor or file loader in the TUI

### Stage 4 — 5-stage pipeline

The Digital Design payoff: model the pipeline registers as state carried forward each
cycle, and show all five stages at once.

- [ ] Pipeline registers: `IF/ID`, `ID/EX`, `EX/MEM`, `MEM/WB`
- [ ] Visualize which instruction occupies each stage per cycle
- [ ] Cycle-accurate stepping

### Stage 5 — Hazards & forwarding

Where it becomes a genuine learning tool.

- [ ] Data hazard detection
- [ ] Forwarding (EX/MEM and MEM/WB bypass paths)
- [ ] Load-use stall insertion
- [ ] Control hazards with a branch-resolution strategy
- [ ] Visual stall/bubble indicators in the pipeline view

### Stretch goals

- [ ] Cycle counters and a simple CPI readout
- [ ] Memory-mapped I/O (console output)
- [ ] RISC-V backend behind the same datapath visualizer
- [ ] Breakpoints and watchpoints

## 🧱 Design conventions

C++ conventions this project follows, to keep the emulator maintainable as it grows:

- **Headers are `.hpp`, sources are `.cpp`**, one component per pair.
- **Strong enums** (`enum class`) for every hardware field — no bare integer opcodes.
- **`std::variant`** for instruction fields so an invalid access throws rather than
  reading garbage.
- **`std::optional`** for fallible operations (decode, ALU control) instead of
  sentinel values.
- **`[[nodiscard]]`** on every pure query so dropped results are caught at compile time.
- **Core libraries never include UI headers** — `mips_core` and `nsc_core` are pure
  logic, unit-tested without a terminal.
- **`constexpr` where the computation allows**, e.g. bit-field extraction.

## 📄 License

see [License.md](LICENSE) file