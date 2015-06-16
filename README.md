# NTHU-courses
The homework and projects in NTHU

========================================================================================
Parallel Programming: (Graduate Program)

Testing System: On Quanta cluster 11 nodes, each one has 12 processes, 96GB memory with 4TB storage space.
Each project must measure and analyze the performance and scalability of your program.

HW_1. Basic odd-even sort and advanced odd-even sort : implementing basic and advanced odd-even sort with MPI.

HW_2. Single Roller Coaster Car Problem And N-body Problem</p>
      <b>Single Roller Coaster Car Problem  :</b> </p>
  2.1 Simulate Single Roller Coaster Car action and prevent synchronization by using conditional variable or mutex lock.</p>
  N-body Problem:</p>
  2.2 Parallel N-body's sequential code by using Pthread and OpenMP.</p>
  2.3 Implement Barnes-Hut Algorithm by Pthread: Must parallelizing building tree phase and simulation phase</p>

HW_3. Manderlbrot Set Problem: Implementing both the static and dynamic scheduling versions to evaluate its loading balance status.</p>
   3.1. distributed memory in MPI</p> 
   3.2. shared memory in OpenMP</p>
   3.3. hybrid (distributed-shared) memory ( MPI + OpenMP)</p>

HW_4. Blocked All-Pairs Shortest Paths Algorithm in CUDA:</p>
   4.1. Single GPU:</p> 
        implement Blocked All-Pairs Shortest Paths Algorithm in CUDA.</p>
   4.2. Multi-GPU in MPI version:</p> 
        implement program using multiple GPUs on multi nodes by MPI.</p>
   4.3. Multi-GPU in OpenMP version:</p> 
        implement program using multiple GPUs on multi nodes by OpenMP.</p>
  
========================================================================================
Computer Architecture:

HW_1. MPIS Single Cycle Simulator: Implement a single-cycle, functional processor simulator according to the reduced MIPS R3000 ISA. 

HW_2. MIPS Pipeline Simulator: Implement a pipelined, functional processor simulator 

HW_3. MIPS Cache Memory Pagetable Simulator: a simulator with memory hierarchy, Translation-Lookaside Buffer (TLB), virtual page table and cache mechanism with write back and write through policy. 

HW_Tools. Assembler and disassembler for Simulator.

========================================================================================
Operation System:

HW_1. User-Mode and Kernel-Mode Processor Monitor: a monitor oversees it own child’ process status and delivers information between different processers.

HW_2. River and frog arcade game: multi-thread programming with XLib

HW3. Virtual Memory Management in CUDA: implementing GPU virtual-memory system with LRU replacement policy.

HW_4. File System Management in CUDA: Implement a simple file system in a kernel function of GPU that have single thread, limit global memory as volume.

HW_5. I/O System DMA (Kernel-mode) device simulator: keyboard device simulator in Kernel-Mode with blocking and non-blocking I/O operation.

========================================================================================
