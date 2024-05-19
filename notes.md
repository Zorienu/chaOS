## Install needed tools
- nasm: `brew install nasm`
- qemu: `brew install qemu`
- see guide to cross-compiler

## Commands
- compile: `make`
- run: `qemu-system-i386 -fda build/main_floppy.img`

## To take into account
- https://wiki.osdev.org/Segmentation: 
   - Information required to get protected mode working also 64 bit GDT is needed to enter long mode
   - If you want to be serious about OS development, we strongly recommend using flat memory model and paging as memory management technique
   - This leaves you with 2 descriptors per privilege level (Ring 0 and Ring 3 normally), one for code and one for data, which both describe precisely the same segment. The only difference being that the code descriptor is loaded into CS, and the data descriptor is used by all the other segment registers. The reason you need both a code and data descriptor is that the processor will not allow you to load CS with a data descriptor (This is to help with security when using a segmented memory model, and although useless in the flat-memory model it is still required because you can't turn off segmentation).
   - So, if you're going to use C, do what the rest of the C world does, which is set up a flat-memory model, use paging, and ignore the fact that segmentation even exists.

## x86 Architecture
### Registers
Small r/w pieces of memory
- General purpose registers: rax, rbx, rcx, rbp (ebp 32 bits), rsp (esp 32 bits)
- Status registers: rip (eip 32 bits), rflags (eflags 32 bits)
- Segment registers
- Protected mode registers
- Control registers
- Debug registers
- Floating point unit registers
- SIMD registers

#### [How EAX register (or any general purpose register) is mapped](https://stackoverflow.com/questions/15191178/how-do-ax-ah-al-map-onto-eax)
- EAX is the full 32-bit value
- AX is the lower 16-bits
- AL is the lower 8 bits
- AH is the bits 8 through 15 (zero-based), the top half of AX

### The PIC (Programmable Interrupt Controller) and the PIT (Programmable Interrupt Timer)
- http://www.brokenthorn.com/Resources/OSDev16.html
- http://www.brokenthorn.com/Resources/OSDevPic.html
- http://www.brokenthorn.com/Resources/OSDevPit.html

### C and C++
- [What is the effect of extern "C" in C++?](https://stackoverflow.com/questions/1041866/what-is-the-effect-of-extern-c-in-c)
- [Calling Global Constructors](https://wiki.osdev.org/Calling_Global_Constructors) Explanation for crti.asm and crtn.asm
- [Pass by value vs pass by rvalue reference](https://stackoverflow.com/questions/37935393/pass-by-value-vs-pass-by-rvalue-reference)
- [Treat h header files as C++ header files flag (-xc++-header)](https://stackoverflow.com/questions/75497627/clangd-lsp-shows-unknown-type-name-interface-in-dsound-h)

### File I/O
- [Overview: Serenety Kernel File I/O](https://www.youtube.com/watch?v=JJx7j4mR3CM)

### Git
- [GitHub - Repository state at specified time](https://stackoverflow.com/questions/21345787/github-repository-state-at-specified-time)

### Resources
- [serenetyOS](https://github.dev/SerenityOS/serenity/tree/HEAD@%7B2019-10-30%7D)
- [Linux device driver lecture 15 : Character driver](https://www.youtube.com/watch?v=R5qSTZA0PuY)

### TODO memory manager
Tener en cuenta el area de "Supervisor physical pages", las puedo usar para alocar cosas del kernel, como el lugar donde llegará lo del disco
    // Basic memory map:
    // 0      -> 512 kB         Kernel code. Root page directory & PDE 0.
    // (last page before 1MB)   Used by quickmap_page().
    // 1 MB   -> 3 MB           kmalloc_eternal() space.
    // 3 MB   -> 4 MB           kmalloc() space.
    // 4 MB   -> 5 MB           Supervisor physical pages (available for allocation!)
    // 5 MB   -> 0xc0000000     Userspace physical pages (available for allocation!)
    // 0xc0000000-0xffffffff    Kernel-only virtual address space

## TODO
[] Migrar memory managers a clases
[x] Implementar keyboardDevice based on serenetyOS
[x] Implementar TTY device based on serenetyOS
[x] Implementar VirtualConsole -> ConsoleImplementation -> Console -> CharacterDevice (read, write, etc)
[x] Implementar "K"printf usando printf_internal -> console_putch para apuntar a VirtualConsole (Console::the().set_implementation(s_consoles[s_active_console]))
[x] File system
[x] Implementar Backspace en VirtualConsole
[x] Implement CircularDeque for TTY
[x] Replace all calls to printf with kprintf
[x] Move interrupts and syscall folder from "include" to "kernel"
[] Implement file system - in progress
[] See how osakaOS uses <> to include files in C++
[] Figure out how to move everything to that file system
[] shell
[] Process 
[] VGA
[] fix: debo incluir kmalloc.h para cuando quiero usar CircularQueue
[] Improve MakeFile, listing all files to compile
[] Implementar delete word from console, en Serenety borran del \_input\_buffer y hacen "echo" de un char especial para indicar
a la shell que borre la palabra
[] Buscaminas
[] cowsay

# File system
### Fundamental operations of a file system
- Tracking the available storage space
- Tracking which block or blocks of data belong to which files
- Creating new files
- Reading data from existing files into memory
- Updating the data in the files
- Deleting existing files
Note that the last 4 points are CRUD

### Additional features
- Assigning human-readable names to files, and renaming files after creation
- Allowing files to be divided among non-contiguous blocks in storage, and tracking the parts of files even when they are fragmented across the medium
- Providing some form of hierarchical structure, allowing the files to be divided into directories or folders
- Buffering reading and writing to reduce the number of actual operation on the physical medium
- Caching frequently accessed files or parts of files to speed up access
- Allowing files to be marked as 'read-only' to prevent unintentional corruption of critical data
- Providing a mechanism for preventing unauthorized access to a user's files


