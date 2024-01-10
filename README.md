# Dynamic ELF Loader with Memory Fault Handling

This project introduces a dynamic ELF loader designed for loading and executing 32-bit ELF executables. The loader includes a custom signal handler to intelligently manage memory faults, dynamically allocating memory and reading file segments during execution. It tracks the number of faults, pages used, and calculates the total internal fragmentation within the loaded ELF executable. This project serves as a valuable tool for gaining insights into ELF file loading, memory management, and fault handling.

## Features

- Dynamic loading of 32-bit ELF executables
- Intelligent memory fault handling with a custom signal handler
- Monitoring of faults, pages used, and internal fragmentation
- Insights into ELF file execution and memory management
