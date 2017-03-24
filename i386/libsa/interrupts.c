#ifdef __i386__

#include "libsa.h"
#include "saio_internal.h"

#ifdef DEBUG_INTERRUPTS
#define DBG(x...) printf(x)
#else
#define DBG(x...)
#endif

#define CODE_SEGMENT_SELECTOR 0x28U
#define CODED_EXCEPTION_MASK 0x27D00U
#define IA32_APIC_BASE 27U
#define IA32_APIC_BASE_BSP 0x100U
#define IA32_APIC_BASE_EN  0x800U
#define LAPIC_WANTED_FLAGS (IA32_APIC_BASE_EN | IA32_APIC_BASE_BSP)
#define LAPIC_EOI_OFFSET 0xB0U
#define LAPIC_ISR_OFFSET 0x100U
#define PIC_READ_ISR_COMMAND 11U
#define PIC_EOI_COMMAND 0x20U
#define PIC_PORT0 0
#define PIC_PORT1 1

#define BDA_TICK_COUNT 0x46CU		// DWORD
#define BDA_MIDNIGHT_FLAG 0x470U		// BYTE
#define BDA_24HR_TURNOVER 0x1800B0U

enum InterruptSources
{
	IS_Unknown = 0,
	IS_APIC = 1,	// Covers both LAPIC and IOAPIC
	IS_PIC0 = 2,
	IS_PIC1 = 3,
	IS_Software = 4
};

struct InterruptFrame
{
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t index;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
};

struct ExceptionFrame
{
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t index;
	uint32_t exception_code;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
};

struct InterruptGate
{
	uint16_t offset_0_15;
	uint16_t selector;
	uint16_t flags;
	uint16_t offset_16_31;
};

#pragma mark -
#pragma mark Global Data
#pragma mark -

extern
uint16_t Idtr_prot[];

static
uint32_t* counters = NULL;

static
uint32_t lapic_base = 0U;

#pragma mark -
#pragma mark Assembly Stubs
#pragma mark -

static
__attribute__((naked, noreturn))
void InterruptStub(void)
{
	__asm__ volatile ("pushl %%eax\n\t"
					  "pushl %%ecx\n\t"
					  "pushl %%edx\n\t"
					  "pushl %%esp\n\t"
					  "calll _InterruptHandler\n\t"
					  "addl $4, %%esp\n\t"
					  "popl %%edx\n\t"
					  "popl %%ecx\n\t"
					  "popl %%eax\n\t"
					  "addl $4, %%esp\n\t"
					  "iretl"
					  :);
}

static
__attribute__((naked, noreturn))
void ExceptionWithCodeStub(void)
{
	__asm__ volatile("testl $-57, 8(%%esp)\n\t"
					 "je 0f\n\t"
					 "pushal\n\t"
					 "jmp 1f\n"
					 "_ExceptionNoCodeStub:\n"
					 "0:\tsub $4, %%esp\n\t"
					 "pushal\n\t"
					 "xorl %%eax, %%eax\n\t"
					 "xchgl %%eax, 36(%%esp)\n\t"
					 "movl %%eax, 32(%%esp)\n"
					 "1:\taddl $20, 12(%%esp)\n\t"
					 "pushl %%esp\n\t"
					 "calll _ExceptionHandler\n\t"
					 "addl $4, %%esp\n\t"
					 "popal\n\t"
					 "addl $8, %%esp\n\t"
					 "iretl"
					 :);
}

/*
 * Make _ExceptionNoCodeStub accessible to C
 */
static
__attribute__((naked, noreturn, weakref("ExceptionNoCodeStub")))
void ExceptionNoCodeStubAlias(void);

static
__attribute__((/* naked, */noinline, regparm(1), section("__INIT,__text")))
void DispatchBiosVector(uint8_t vector)
{
	__asm__ volatile ("movb %0, 0f + 1\n\t"
					  "calll __prot_to_real\n\t"
					  ".code16\n"
					  "0:\tint $0\n\t"
					  "calll __real_to_prot\n\t"
					  ".code32"
					  : : "r"(vector));
}

static
__attribute__((noreturn, noinline, section("__INIT,__text")))
void DisplayErrorAndStop(void)
{
	__asm__ volatile ("calll __prot_to_real\n\t"
					  ".code16\n\t"
					  "movw $2, %%ax\n\t"
					  "int $0x10\n\t"
					  "xorw %%ax, %%ax\n\t"
					  "movw %%ax, %%ds\n\t"
					  "movw $0x6000, %%si\n\t"
					  "cld\n"
					  "0:\tlodsb\n\t"
					  "testb %%al, %%al\n\t"
					  "je 1f\n\t"
					  "movb $0xE, %%ah\n\t"
					  "movw $0xFF, %%bx\n\t"
					  "int $0x10\n\t"
					  "jmp 0b\n"
					  "1:\thlt\n\t"
					  "jmp 1b\n\t"
					  ".code32"
					  :);
	__builtin_unreachable();
}

#pragma mark -
#pragma mark Other Inline Assembly
#pragma mark -

static inline
uint32_t ReadLapic(uint32_t offset)
{
	return *(uint32_t const volatile*) (lapic_base + offset);
}

static inline
void WriteLapic(uint32_t offset, uint32_t value)
{
	*(uint32_t volatile*) (lapic_base + offset) = value;
}

#define ChoosePicPort(pic, index) (pic == 0 ? (index == 0 ? 0x20U : 0x21U) : (index == 0 ? 0xA0U : 0xA1U))

#define ReadPic(pic, index) ({ \
uint8_t value; \
__asm__ volatile ("inb %1, %0" : "=a"(value) : "N"(ChoosePicPort(pic, index))); \
value; \
})

#define WritePic(pic, index, value) __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)value), "N"(ChoosePicPort(pic, index)))

#pragma mark -
#pragma mark Main Code
#pragma mark -

static
int IdentifyInterruptSource(uint8_t vector, uint32_t eip)
{
	if (lapic_base)
	{
		uint32_t value = ReadLapic(LAPIC_ISR_OFFSET + ((vector & 0xE0U) >> 1));
		if (value & (1U << (vector & 31U)))
			return IS_APIC;
	}
	if (vector >= 8U && vector < 16U)
	{
		uint8_t value;
		WritePic(0, PIC_PORT0, PIC_READ_ISR_COMMAND);
		value = ReadPic(0, PIC_PORT0);
		if (value & (1U << (vector & 7U)))
			return IS_PIC0;
	}
	if (vector >= 0x70U && vector < 0x78U)
	{
		uint8_t value;
		WritePic(1, PIC_PORT0, PIC_READ_ISR_COMMAND);
		value = ReadPic(1, PIC_PORT0);
		if (value & (1U << (vector & 7U)))
			return IS_PIC1;
	}
	if (eip)
	{
		uint8_t const volatile* pInstruction = (uint8_t const volatile*) (eip - 2U);
		if ((*pInstruction) == 0xCDU && pInstruction[1] == vector)
			return IS_Software;
		/*
		 * There are other software interrupt opcodes
		 *   debug breakpoint 0xCC
		 *   interrupt on overflow 0xCE
		 *   bound instruction 0x62
		 * but those all trigger specific vectors, so are handled as exceptions.
		 */
	}
	return IS_Unknown;
}

static
void SignalEOI(int source)
{
	switch (source)
	{
		case IS_APIC:
			if (lapic_base)
				WriteLapic(LAPIC_EOI_OFFSET, 0U);
			break;
		case IS_PIC1:
			WritePic(1, PIC_PORT0, PIC_EOI_COMMAND);
		case IS_PIC0:
			WritePic(0, PIC_PORT0, PIC_EOI_COMMAND);
		default:
			break;
	}
}

static
void HandleIRQ(int source, uint8_t vector)
{
	if (source == IS_PIC0 && vector == 8U)
	{
		uint32_t* pTickCount = (uint32_t*) BDA_TICK_COUNT;
		if (++(*pTickCount) == BDA_24HR_TURNOVER)
		{
			*pTickCount = 0U;
			++(*(uint8_t*)BDA_MIDNIGHT_FLAG);
		}
		SignalEOI(source);
		return;
	}
	/*
	 * Default Approach: send to bios
	 */
	DispatchBiosVector(vector);
}

static
__attribute__((used))
void ExceptionHandler(struct ExceptionFrame* pFrame)
{
	uint8_t vector;
	int interruptSource;
	char* errorString;

	/*
	 * FIXME: Should check if 0x10000U <= ESP <= 0x1FFFF0 here and switch stacks if not.
	 */
	if (!pFrame)
	{
		return;
	}
	vector = (uint8_t) pFrame->index;
	if (counters)
		++counters[vector];
	interruptSource = IdentifyInterruptSource(vector, pFrame->eip);
	switch (interruptSource)
	{
		case IS_APIC:
		case IS_PIC0:
		case IS_PIC1:
			HandleIRQ(interruptSource, vector);
		case IS_Software:
			return;
		default:
			break;
	}
	errorString = (char*) 0x6000U;
	switch (vector)
	{
		case 0U:
			strcpy(errorString, "Division By Zero Exception");
			break;
		case 1U:
			strcpy(errorString, "Debug Exception");
			break;
		case 2U:
			strcpy(errorString, "NMI Interrupt");
			break;
		case 3U:
			strcpy(errorString, "Debug Breakpoint");
			break;
		case 4U:
			strcpy(errorString, "Overflow Exception");
			break;
		case 5U:
			strcpy(errorString, "BOUND Range Exception");
			break;
		case 6U:
			strcpy(errorString, "Invalid Opcode Exception");
			break;
		case 7U:
			strcpy(errorString, "Math Coprocessor Unavailable Exception");
			break;
		case 8U:
			strcpy(errorString, "Double Fault");
			break;
		case 9U:
			strcpy(errorString, "Coprocessor Segment Overrun Exception");
			break;
		case 10U:
			strcpy(errorString, "Invalid TSS Exception");
			break;
		case 11U:
			strcpy(errorString, "Segment Not Present Exception");
			break;
		case 12U:
			strcpy(errorString, "Stack-Segment Fault");
			break;
		case 13U:
			strcpy(errorString, "General Protection Fault");
			break;
		case 14U:
			strcpy(errorString, "Page Fault");
			break;
		case 16U:
			strcpy(errorString, "x87 FPU Floating-Point Error");
			break;
		case 17U:
			strcpy(errorString, "Alignment Check Exception");
			break;
		case 18U:
			strcpy(errorString, "Machine Check Exception");
			break;
		case 19U:
			strcpy(errorString, "SIMD Floating-Point Exception");
			break;
		case 20U:
			strcpy(errorString, "Virtualization Exception");
			break;
		default:
			sprintf(errorString, "Unknown Exception Vector %d", (int) vector);
			break;
	}
	errorString += strlen(errorString);

	errorString += sprintf(errorString, "\r\nEDI 0x%x, ESI 0x%x, EBP 0x%x, ESP 0x%x",
						   pFrame->edi, pFrame->esi, pFrame->ebp, pFrame->esp);
	errorString += sprintf(errorString, "\r\nEBX 0x%x, EDX 0x%x, ECX 0x%x, EAX 0x%x",
						   pFrame->ebx, pFrame->edx, pFrame->ecx, pFrame->eax);
	errorString += sprintf(errorString, "\r\nException Code 0x%x, EIP 0x%x, CS 0x%x, EFLAGS 0x%x\r\nSystem Halted\r\n",
						   pFrame->exception_code, pFrame->eip, pFrame->cs, pFrame->eflags);
	DisplayErrorAndStop();
}

static
__attribute__((used))
void InterruptHandler(struct InterruptFrame* pFrame)
{
	uint8_t vector;
	int interruptSource;

	if (!pFrame)
	{
		return;
	}
	vector = (uint8_t) pFrame->index;
	if (counters)
		++counters[vector];
	interruptSource = IdentifyInterruptSource(vector, pFrame->eip);
	switch (interruptSource)
	{
		case IS_APIC:
		case IS_PIC0:
		case IS_PIC1:
			HandleIRQ(interruptSource, vector);
		default:
			break;
	}
}

#if UNUSED
void dumpMasks(void)
{
	int idx;
	uint8_t port_val;
	uint8_t volatile* apic_index;
	uint32_t const volatile* apic_data;

	port_val = ReadPic(0, 1);
	DBG("pic0 Masks 0x%x\n", port_val);
	port_val = ReadPic(1, 1);
	DBG("pic1 Masks 0x%x\n", port_val);
	getchar();
	DBG("IOAPIC vectors\n");
	apic_index = (uint8_t volatile*) 0xFEC00000U;
	apic_data = (uint32_t const volatile*) 0xFEC00010U;
	for (idx = 0; idx != 24; ++idx)
	{
		uint32_t v1, v2;
		*apic_index = (uint8_t) (16U + 2U * (unsigned) idx);
		v1 = *apic_data;
		if (v1 & 0x10000U)
			continue;
		*apic_index = (uint8_t) (16U + 2U * (unsigned) idx + 1U);
		v2 = *apic_data;
		DBG("index %d vector 0x%x%08x\n", idx, v2, v1);
	}
	getchar();
	if (!lapic_base)
		return;
	DBG("LAPIC vectors\n");
	for (idx = 0; idx != 7; ++idx)
	{
		uint32_t offs, v;
		if (!idx)
			offs = 0x2F0U;
		else
			offs = 0x320U + 16U * (unsigned) (idx - 1);
		v = ReadLapic(offs);
		if (v & 0x10000U)
			continue;
		DBG("index %d vector 0x%x\n", idx, v);
	}
}
#endif

void ShowInterruptCounters(void)
{
	int j;

	if (!counters)
		return;
	msglog("Interrupt Counters\n");
	for (j = 0; j != 256; ++j)
		if (counters[j])
			msglog("counters[%d] == %d\n", j, counters[j]);
}

int SetupInterrupts(void)
{
	int idx;
	uint32_t stub_address;
	uint64_t ia32_apic_base;
	size_t const total = 2048U + 2048U + 1024U;
	uint8_t* workArea = (uint8_t*) malloc(total);
	if (!workArea)
	{
		msglog("%s: Memory Allocation Failed\n", __FUNCTION__);
		return 0;
	}
	counters = (uint32_t*) (workArea + 4096);
	bzero(counters, 1024U);
	for (idx = 0; idx != 256; ++idx)
	{
		struct InterruptGate* gate = (struct InterruptGate*) (workArea + idx * sizeof(struct InterruptGate));
		uint8_t* thunk = workArea + 2048 + idx * 8;
		gate->offset_0_15 = ((uint32_t) thunk) & 0xFFFFU;
		gate->selector = CODE_SEGMENT_SELECTOR;
		gate->flags = 0x8E00U;  // Interrupt Gate, Present, DPL 0, 32-bit
		gate->offset_16_31 = (((uint32_t) thunk) >> 16) & 0xFFFFU;
		thunk[0] = 0x6AU;	// push byte
		thunk[1] = (uint8_t) idx;
		thunk[2] = 0xE9U;	// jmp rel32
		if (idx >= 32)
			stub_address = (uint32_t) &InterruptStub;
		else if ((1U << idx) & CODED_EXCEPTION_MASK)
			stub_address = (uint32_t) &ExceptionWithCodeStub;
		else
			stub_address = (uint32_t) &ExceptionNoCodeStubAlias;
		*(uint32_t*) (&thunk[3]) = stub_address - (uint32_t) &thunk[7];
		thunk[7] = 0x90U;	// nop
	}
	Idtr_prot[0] = 0x7FFU;
	Idtr_prot[1] = ((uint32_t) workArea) & 0xFFFFU;
	Idtr_prot[2] = (((uint32_t) workArea) >> 16) & 0xFFFFU;
	__asm__ volatile ("lidt %0" : : "m"(Idtr_prot[0]));
	__asm__ volatile ("rdmsr" : "=A"(ia32_apic_base) : "c"((uint32_t) IA32_APIC_BASE));
	if ((ia32_apic_base & LAPIC_WANTED_FLAGS) == LAPIC_WANTED_FLAGS &&
		!((ia32_apic_base >> 32) & 255U))
		lapic_base = ((uint32_t) ia32_apic_base) & ~0xFFFU;
	DBG("%s: Work Area  0x%x, lapic_base 0x%x\n", __FUNCTION__, (uint32_t) workArea, lapic_base);
	return 1;
}
#endif
