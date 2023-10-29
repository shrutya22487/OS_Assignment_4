#include "loader.h"
#include <signal.h>
Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
//declaring global variables
int fd , i ,min_entrypoint;
Elf32_Addr entry_pt = 0 ;
void *virtual_mem = NULL;

void signal_handler( int signum ){
  //allocating memory
  if (signum == SIGSEGV) {
        // Determine the faulting address
        void *fault_addr = (void *)get_fault_address();

        // Find the corresponding segment based on fault_addr and load it lazily
        for (int i = 0; i < ehdr->e_phnum; i++) {
            Elf32_Phdr *segment = &phdr[i];
            if (fault_addr >= (void *)segment->p_vaddr &&
                fault_addr < (void *)(segment->p_vaddr + segment->p_memsz)) {
                // Calculate the offset within the segment
                Elf32_Addr offset = (Elf32_Addr)fault_addr - segment->p_vaddr;
                
                //allocate memory when segmentation fault occurs
                // Map a page-sized block of memory with read-write permissions
                void *mapped_addr = mmap((void *)(segment->p_vaddr & ~(PAGE_SIZE - 1)),
                                         PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                if (mapped_addr == MAP_FAILED) {
                    printf("Failed to allocate memory for lazy loading\n");
                    exit(1);
                }

                // Copy the segment data to the mapped memory
                memcpy(mapped_addr, (void *)(segment->p_offset + offset), PAGE_SIZE);

                // Change protection to read-only to catch further accesses
                mprotect(mapped_addr, PAGE_SIZE, PROT_READ);

                return; // Segment loaded, no longer a segmentation fault
            }
        }

        // If the faulting address is not within any segment, it's an error
        printf("Segmentation fault at address: %p\n", fault_addr);
        exit(1);
    }
}

void setup_signal_handler() {
    struct sigaction sh_sev;
    memset(&sh_sev, 0, sizeof(sh_sev));
    sh_sev.sa_handler = signal_handler; 
    if (sigaction(SIGSEGV, &sh_sev, NULL) == -1)
    {
        printf("Error in handling SIGSEV\n");
    }
    memset(&sh_sev, 0, sizeof(sh_sev));
}

void free_space(){
    free(ehdr);
    free(phdr);
}

// Unmap virtual memory and close the file descriptor
void unmapping_virtual_memory(){
    if (virtual_mem != NULL) {
        munmap(virtual_mem, phdr[i].p_memsz);
    }
    close(fd);
}

// Check if the ELF file can be opened for reading
void check_file_read(const char* exe){
  int fd = open(exe, O_RDONLY);
  if (fd < 0) {
    printf("Error opening ELF file\n");
    exit(1);
  }
}
// Check if offset seeking was successful
void check_offset( off_t new_position ){
  if ( new_position == -1 )
  {
    printf("Failed to seek offset\n");
    exit(1);
  }
}

// Load program headers into memory
void load_phdr( size_t size_of_phdr ){
  phdr = ( Elf32_Phdr* )malloc( size_of_phdr * ehdr->e_phnum); 
  
  if (phdr == NULL) {
        printf("Failed to allocate memory for program headers.\n");
        exit(1);
  }
  
  check_offset(lseek(fd, 0, SEEK_SET));
  check_offset( lseek(fd , ehdr -> e_phoff , SEEK_SET ) );
  
  // Read program headers into memory
  if ( read( fd , phdr , size_of_phdr * ehdr -> e_phnum) !=  size_of_phdr * ehdr -> e_phnum)
  {
    printf("Failed to load program headers properly\n");
    exit(1);
  }
  return;
}

// Load ELF header into memory and perform necessary checks
void load_ehdr( size_t size_of_ehdr ){
  ehdr = ( Elf32_Ehdr* )malloc(size_of_ehdr);
  
  if (ehdr == NULL) {
        printf("Failed to allocate memory for ELF header.\n");
        exit(1);
  }

  check_offset( lseek(fd, 0, SEEK_SET) ); 
  // Read ELF header into memory
  if (read(fd, ehdr, size_of_ehdr) != size_of_ehdr)
  {
    printf("Failed to load ELF header properly\n");
    exit(1);
  }
  // Check if the ELF file is 32-bit
  if (ehdr -> e_ident[EI_CLASS] != ELFCLASS32) {
    printf("Not a 32-bit ELF file\n");
    exit(1);
  }
  return;
}

/*
 * Load and run the ELF executable file
 */

// Find the appropriate entry point in the program headers corres to PT_LOAD
void find_entry_pt(){
  i = 0  ;
  min_entrypoint = 0;
  int min = 0xFFFFFFFF;
  for ( i = 0; i < ehdr -> e_phnum ; i++)
  {
    if ( phdr[i].p_flags == 0x5)
    {
      if (min > ehdr->e_entry - phdr[i].p_vaddr )
      {
        min = ehdr->e_entry - phdr[i].p_vaddr;
        min_entrypoint = i;
      }
    }
  }
  i = min_entrypoint;
  entry_pt = phdr[i].p_vaddr;
}

// Allocate virtual memory and load segment content
void Load_memory(){
  virtual_mem = mmap(NULL, phdr[i].p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE , 0, 0);  
  
  if (virtual_mem == MAP_FAILED) {
      printf("Failed to allocate virtual memory\n");
      exit(1);
  }
  
  check_offset(lseek(fd, 0, SEEK_SET));
  check_offset( lseek( fd , phdr[i].p_offset ,SEEK_SET ) );

  // Read segment content into virtual memory
  read(fd , virtual_mem ,phdr[i].p_memsz) ;
}

// Open the ELF file and validate the file descriptor
void open_elf( char* exe ){
  fd = open(exe, O_RDONLY);
  
  if (fd < 0) {
    printf("Failed to open ELF file\n");
    exit(1);
  }
}

// Load and execute the ELF executable
void load_and_run_elf(char* exe) {
  open_elf(exe);

  size_t size_of_phdr = sizeof( Elf32_Phdr ) ,size_of_ehdr = sizeof( Elf32_Ehdr); // size of one program header

  // 1. Load entire binary content into memory from the ELF file.
  load_ehdr( size_of_ehdr );
  load_phdr( size_of_phdr );

  // 2. Find the appropriate entry point in the program headers
  find_entry_pt();

  // 3. Allocate memory and load the segment content
  //Load_memory();

  size_t page_size = 4096;  // Set page size to 4KB
  Elf32_Addr total_memsz = phdr[i].p_memsz;
  Elf32_Addr offset = ehdr->e_entry - entry_pt;

// Iterate through the virtual memory addresses within the segment
  for (Elf32_Addr addr = entry_pt; addr < entry_pt + total_memsz; addr += page_size) {
    // Calculate the aligned address that is a multiple of the page size (page boundary)
    void *mapped_addr = mmap((void *)(addr & ~(page_size - 1)),page_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      // Check if the mmap call was successful
    if (mapped_addr == MAP_FAILED) {
        printf("Failed to allocate memory for segment\n");
        exit(1);
    }
  }


  // 4. Calculate the offset between entry point and segment starting address
  Elf32_Addr offset = ehdr->e_entry - entry_pt;
  // This should now be performed for the entire memory range
    void *entry_virtual = virtual_mem + offset;
    int (*_start)() = (int (*)())entry_virtual;
    int result = _start();
    printf("User _start return value = %d\n", result);
}
  // Get the actual memory address of the _start function
  void *entry_virtual = virtual_mem + offset;

  // 5. Typecast the address to a function pointer for "_start" method
  int (*_start)() = (int(*)())entry_virtual;

  // 6. Call the "_start" method and print the returned value
  int result = _start();

  // Cleanup and display result
  printf("User _start return value = %d\n", result);


int main(int argc, char** argv) 
{
// checking if we get 2 arguments into the main
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. Check the ELF file can be read
  check_file_read(argv[1]);

  // 2. Load and execute the ELF executable
  load_and_run_elf(argv[1]);

  // 3. Perform cleanup
  free_space();
  unmapping_virtual_memory();

  return 0;
}