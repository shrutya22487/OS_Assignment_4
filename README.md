# Smart Loader 🛠️

The Smart Loader is a program designed to dynamically load and execute ELF (Executable and Linkable Format) binaries, handling memory mapping and signal handling for segmentation faults.

## Features 🌟

- **Dynamic Memory Mapping:** Utilizes `mmap` to map virtual memory dynamically when a segmentation fault occurs during execution.
  
- **Signal Handling:** Implements a custom signal handler (`SIGSEGV_handler`) to catch segmentation faults (`SIGSEGV`) and dynamically allocate memory for the faulting address.

- **Memory Management:** Efficiently manages memory using page-sized allocations (`PAGE_SIZE = 4096`) and calculates internal fragmentation.

## Functions 📝

### 1. `load_ehdr(size_t size_of_ehdr)`
In this function, memory is allocated to store the ELF header, and the ELF header is read from the file into memory. The function also checks whether the opened file is a 32-bit ELF file. It's a critical step to load the necessary information about the ELF file's structure.

### 2. `load_phdr(size_t size_of_phdr)`
Similar to `load_ehdr`, this function allocates memory for program headers and reads them from the file into memory. Program headers contain essential information about various segments in the ELF file, which is needed to load and execute the program.

### 3. `setup_signal_handler()`
This function sets up a signal handler, specifically a segmentation fault (SIGSEGV) handler. The signal handler detects and handles page faults (e.g., missing segments) during the program's execution. It efficiently manages page allocation and fault handling.

### 4. `segfault_handler(int signum, siginfo_t *sig, void *context)`
The segmentation fault handler is the core of this program. When a page fault occurs, it checks which segment the fault belongs to, allocates memory for the missing segment, and continues the program's execution. It keeps track of the number of page faults and the allocation status of each page within a segment, which is crucial for memory efficiency.

## Usage 🚀

1. **Compilation:**
   ```bash
   gcc -o smart_loader loader.c
2. **Execution**
   ```bash
    ./smart_loader <ELF Executable>
Replace `<ELF Executable>` with the path to your 32-bit ELF binary.

### Requirements 📋
- Linux environment (as it uses mmap and signal handling specific to POSIX systems).
- GCC compiler (`gcc`).

### Notes 
- Ensure the ELF binary is a 32-bit executable (ELFCLASS32).
- The loader assumes the ELF file can be opened for reading - (O_RDONLY).
- Clean up resources using free_space() and unmapping_virtual_memory() after execution.