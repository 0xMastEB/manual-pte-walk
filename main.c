#include <ntddk.h>
#include <intrin.h>
#define PML4_INDEX(va) (((ULONG64)(va) >> 39) & 0x1FF)  // 0x1FF = 511 in decimal 111111111.
#define PDPT_INDEX(va) (((ULONG64)(va) >> 30) & 0x1FF)
#define PD_INDEX(va)   (((ULONG64)(va) >> 21) & 0x1FF)
#define PT_INDEX(va)   (((ULONG64)(va) >> 12) & 0X1FF) 

ULONG64 ReadPhysical(ULONG64 PhysAddr)
{
    ULONG64 value = 0;
    MM_COPY_ADDRESS addr;
    SIZE_T bytesRead = 0;
    addr.PhysicalAddress.QuadPart = PhysAddr;
    MmCopyMemory(&value, addr, sizeof(ULONG64), MM_COPY_MEMORY_PHYSICAL, &bytesRead);
    return value;
}


VOID WalkPageTables(PVOID VirtualAddress)
{
    ULONG64 cr3 = __readcr3();
    ULONG64 pml4Phys = cr3 & 0xFFFFFFFFF000;

    ULONG64 pml4e = ReadPhysical(pml4Phys + PML4_INDEX(VirtualAddress) * 8);
    DbgPrint("[Walk] PML4E: 0x%llX (index %llu)\n", pml4e, PML4_INDEX(VirtualAddress));
    if (!(pml4e & 1)) { DbgPrint("[Walk] PML4E Not Present!\n"); return; }

    ULONG64 pdpte = ReadPhysical((pml4e & 0xFFFFFFFFF000) + PDPT_INDEX(VirtualAddress) * 8);
    DbgPrint("[Walk] PDPTE: 0x%llX (index %llu)\n", pdpte, PDPT_INDEX(VirtualAddress));
    if (!(pdpte & 1)) { DbgPrint("[Walk] PDPTE not present!\n"); return; }

    ULONG64 pde = ReadPhysical((pdpte & 0xFFFFFFFFF000) + PD_INDEX(VirtualAddress) * 8);
    DbgPrint("[Walk] PDE: 0x%llX (index %llu)\n", pde, PD_INDEX(VirtualAddress));
    if (!(pde & 1)) { DbgPrint("[Walk] PDE not present!\n"); return; }

    if (pde & 0x80) {
        ULONG64 physAddr = (pde & 0xFFFFFE00000) | ((ULONG64)VirtualAddress & 0x1FFFFF);
        DbgPrint("[Walk] 2MB Large Page! Physical: 0x%llX\n", physAddr);
        return;
    }

    ULONG64 pte = ReadPhysical((pde & 0xFFFFFFFFF000) + PT_INDEX(VirtualAddress) * 8);
    DbgPrint("[Walk] PTE: 0x%llX (index %llu)\n", pte, PT_INDEX(VirtualAddress));
    if (!(pte & 1)) { DbgPrint("[Walk] PTE not present!\n"); return; }

    ULONG64 physAddr = (pte & 0xFFFFFFFFF000) | ((ULONG64)VirtualAddress & 0xFFF);
    DbgPrint("[Walk] Physical Address: 0x%llX\n", physAddr);
}


VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    DbgPrint("[HelloKernel] Driver Unloaded\n");
}
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    DriverObject->DriverUnload = DriverUnload;

    DbgPrint("[HelloKernel] Walking page tables  for DriverEnry itself...\n");
    WalkPageTables((PVOID)DriverEntry);
    return STATUS_SUCCESS;
}