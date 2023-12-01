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

### The PIC (Programmable Interrupt Controller) and the PIT (Programmable Interrupt Timer)
- http://www.brokenthorn.com/Resources/OSDev16.html
- http://www.brokenthorn.com/Resources/OSDevPic.html
- http://www.brokenthorn.com/Resources/OSDevPit.html

### C and C++
- [What is the effect of extern "C" in C++?](https://stackoverflow.com/questions/1041866/what-is-the-effect-of-extern-c-in-c)
- [Calling Global Constructors](https://wiki.osdev.org/Calling_Global_Constructors) Explanation for crti.asm and crtn.asm

### File I/O
- [Overview: Serenety Kernel File I/O](https://www.youtube.com/watch?v=JJx7j4mR3CM)

### Git
- [GitHub - Repository state at specified time](https://stackoverflow.com/questions/21345787/github-repository-state-at-specified-time)
- [Linux device driver lecture 15 : Character driver](https://www.youtube.com/watch?v=R5qSTZA0PuY)
