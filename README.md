<b># NTHU-courses</b>
The homework and projects in NTHU (National Tsing Hua University)

<b>====================================================</b></p>
<b>Parallel Programming: (Graduate Program)</b>

Testing System: On Quanta cluster 11 nodes, each one has 12 processes, 96GB memory with 4TB storage space.
Each project must <b>measure and analyze the performance and scalability of implemented programs</b>.

<b>HW_1. Basic odd-even sort and advanced odd-even sort :</b> implementing basic and advanced odd-even sort with MPI and <b>parallelizing I/O actions with collective MPI-I/O</b>.

<b>HW_2. Single Roller Coaster Car Problem And N-body Problem</b></p>
      <blockquote><blockquote><blockquote><b>Single Roller Coaster Car Problem  :</b></blockquote></blockquote></blockquote></p>
  <blockquote><blockquote><blockquote>2.1 Simulate Single Roller Coaster Car action and prevent synchronization by using conditional variable or mutex lock.</p></blockquote></blockquote></blockquote>
  <blockquote><blockquote><blockquote><b>N-body Problem:</b></blockquote></blockquote></blockquote></p>
  <blockquote><blockquote><blockquote>2.2 Parallel N-body's sequential code by using Pthread and OpenMP.</blockquote></blockquote></blockquote></p>
  <blockquote><blockquote><blockquote>2.3 Implement <b>Barnes-Hut Algorithm</b> by Pthread: Must <b>parallelizing building tree phase</b> and simulation phase</blockquote></blockquote></blockquote></p>

<b>HW_3. Manderlbrot Set Problem:</b> Implementing both the static and dynamic scheduling versions to evaluate its loading balance status.</p>
   <blockquote><blockquote><blockquote>3.1. Distributed memory in MPI</blockquote></blockquote></blockquote></p> 
   <blockquote><blockquote><blockquote>3.2. Shared memory in OpenMP</blockquote></blockquote></blockquote></p>
   <blockquote><blockquote><blockquote>3.3. Hybrid (distributed-shared) memory ( MPI + OpenMP)</blockquote></blockquote></blockquote></p>

<b>HW_4. Blocked All-Pairs Shortest Paths Algorithm in CUDA:</b></p>
   <blockquote><blockquote><blockquote>4.1. Single GPU:implement Blocked All-Pairs Shortest Paths Algorithm in CUDA.</blockquote></blockquote></blockquote></p>
   <blockquote><blockquote><blockquote>4.2. Multi-GPU in MPI version:implement program using multiple GPUs on multi nodes by MPI.</blockquote></blockquote></blockquote></p>
   <blockquote><blockquote><blockquote>4.3. Multi-GPU in OpenMP version:mplement program using multiple GPUs on multi nodes by OpenMP.</blockquote></blockquote></blockquote></p>
  
<b>====================================================</b></p>
<b>Advance Programming Techniques and Implementation: Solve ACM & UVA problem.</b>

<b>01_Binary Search:</b>　Uva_10341_Solve_It, Uva_714_Copying_book

<b>02_Sorting:</b>        Uva_812_Trade_On_Verweggistan,   Uva_10125_Sumsets

<b>03_Big_Number:</b>     Uva_623_500!, Uva_10023_Square_Root 

<b>04_Prime_Number:</b>   Uva_294_Divisors, Uva_10140_Prime_Distance

<b>05_Simulation:</b>     Uva_246_10-20-30. Uva_305_Joseph

<b>06_Graph_DFS:</b>　    Uva_10608_Friends, Uva_315_Network

<b>07_EulerPath:</b>      Uva_302_John's_Trip, Uva_10248_The_Integer_All-Time_Champ　

<b>08_SingleSource_ShortestPath:</b>   Uva_318_Domino_Effect,   Uva_10537_Toll!_Revisited

<b>09_BFS:</b>                         Uva_816_Abbott_Revenge,  Uva_10603_Fill

<b>10_All-pairs_Shortest_Path:</b>　   Uva_247_Calling_Circles, Uva_10269_Advaneture_Of_Super_Mario

<b>11_Branch_And_Bound_1:</b>          Uva_10318_Security_Panel, Uva_818_Cutting_Chains

<b>12_Branch_And_Bound_2:</b>          Uva_10422_Knights_In_FEN, Uva_704_Colour_Hash

<b>====================================================</b></p>
<b>Computer Architecture:</b>

<b>HW_1. MIPS Single Cycle Simulator:</b> Implement a single-cycle, functional processor simulator according to the reduced MIPS R3000 ISA. 

<b>HW_2. MIPS Pipelined Simulator:</b> Implement a pipelined, functional processor simulator with forwarding unit and data-hazard dectection.

<b>HW_3. MIPS Cache Memory Pagetable Simulator:</b> a simulator with memory hierarchy, Translation-Lookaside Buffer (TLB), virtual page table and cache mechanism with write back and write through policy. 

<b>HW_Tools:</b> Assembler and disassembler for Simulator.

<b>====================================================</b></p>
<b>Operation System:</b>

<b>HW_1. User-Mode and Kernel-Mode Processor Monitor:</b> a monitor oversees it own child’ process status and delivers information between different processers.

<b>HW_2. River and Frog Arcade Game:</b> multi-thread programming with XLib

<b>HW3. Virtual Memory Management in CUDA:</b> implementing GPU virtual-memory system with LRU replacement policy.

<b>HW_4. File System Management in CUDA:</b> Implement a simple file system in a kernel function of GPU that have single thread, limit global memory as volume.

<b>HW_5. I/O System DMA (Kernel-mode) Device Simulator:</b> keyboard device simulator in Kernel-Mode with blocking and non-blocking I/O operation.

<b>====================================================</b></p>
