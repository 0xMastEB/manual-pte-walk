# Manual PTE Walk

A Windows kernel driver that manually walks the x64 page table hierarchy (CR3 → PML4 → PDPT → PD → PT) to translate a virtual address into its corresponding physical address in RAM.

The driver translates its own `DriverEntry` virtual address by reading physical memory at each level using `MmCopyMemory`, reimplementing what the CPU does in hardware on every memory access.

## Features

- Reads CR3 to locate the PML4 base
- Walks all 4 page table levels readi
