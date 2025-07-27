
/*
 *  Macros
 */


/* Functional macros
 * 
 * ZEROVISOR_USE_SHUTDOWN: For support safe shut-down. 
 * 
 * 
 * 
 */
#define ZEROVISOR_USE_SHUTDOWN                  1

/* Utility macros. */
#define CEIL(X, Y)						        (((X) + (Y) - 1) / (Y))
#define MASK_GDT_ACCESS					        ((u64)0x03)

/* Static symbol table */
#define SYMBOL_MAX_COUNT                        32

/* Error codes */
#define ERROR_NOT_START						 	1
#define ERROR_HW_NOT_SUPPORT					2
#define ERROR_LAUNCH_FAIL						3
#define ERROR_KERNEL_MODIFICATION				4
#define ERROR_KERNEL_VERSION_MISMATCH			5
#define ERROR_SHUTDOWN_TIME_OUT					6
#define ERROR_MEMORY_ALLOC_FAIL					7
#define ERROR_TASK_OVERFLOW						8
#define ERROR_MODULE_OVERFLOW					9

/* Define allocated memory type. */
#define ALLOC_KMALLOC							0
#define ALLOC_VMALLOC							1

/* CPUID flags */
#define CPUID_1_ECX_VMX							((u64)0x01 << 5)

#define MAX_PROCESSOR_COUNT 				    256
#define MAX_STACK_SIZE						    0x800000

#define VMCS_SIZE							    0x2000
#define IO_BITMAP_SIZE						    0x1000
#define VIRT_APIC_PAGE_SIZE 				    0x1000
