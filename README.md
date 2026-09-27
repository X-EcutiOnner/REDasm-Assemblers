# REDasm Processor Plugins
This repository hosts the official native CPU architecture modules for the **[REDasm Core Engine](https://github.com/redasm-dev/core)**.

Every processor is written in pure C as a hot-pluggable shared library.

## Supported Architectures
*   **16/32/64-bit**: Intel x86, x86_64 (Real, Protected, and Long modes).
*   **RISC & Embedded**: MIPS (MIPS32), ARM (ARM32, ARM64, THUMB).
*   **IoT & Maker**: Xtensa (currently disabled).
*   **8-bit & Retro**: Zilog Z80 (with register tracking), MOS Technology 6502 (NES).
