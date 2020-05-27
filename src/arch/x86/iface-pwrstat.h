#ifndef __PALACIOS_PWRSTAT_H__
#define __PALACIOS_PWRSTAT_H__

/* THESE ARE INTEL SPECIFIC (Sandy Bridge and up) */

#define SANDY_BRIDGE_E3_MODEL_NO 0x2A
#define SANDY_BRIDGE_E5_MODEL_NO 0x2D
#define IVY_BRIDGE_MODEL_NO 0x3A
/* WARNING WARNING: this is speculation... */
#define HASWELL_MODEL_NO 0x4A

#ifdef MSR_RAPL_POWER_UNIT
// assume the rest are also defined by the kernel's msr include
// except for special ones here

#else
// assume none are defined by the kernel's msr include
#define MSR_RAPL_POWER_UNIT		0x606
#define MSR_PKG_POWER_LIMIT	        0x610
#define MSR_PKG_ENERGY_STATUS		0x611
#define MSR_PKG_PERF_STATUS		0x613
#define MSR_PKG_POWER_INFO		0x614

/* PP0 RAPL Domain */
#define MSR_PP0_POWER_LIMIT		0x638
#define MSR_PP0_ENERGY_STATUS		0x639
#define MSR_PP0_POLICY			0x63A
#define MSR_PP0_PERF_STATUS		0x63B

/* PP1 RAPL Domain, may reflect to uncore devices */
#define MSR_PP1_POWER_LIMIT		0x640
#define MSR_PP1_ENERGY_STATUS		0x641
#define MSR_PP1_POLICY			0x642

/* DRAM RAPL Domain */
#define MSR_DRAM_POWER_LIMIT		0x618
#define MSR_DRAM_ENERGY_STATUS		0x619
#define MSR_DRAM_PERF_STATUS		0x61B
#define MSR_DRAM_POWER_INFO		0x61C

#endif

/* RAPL UNIT BITMASK */
#define POWER_UNIT_OFFSET	0
#define POWER_UNIT_MASK		0x0F

#define ENERGY_UNIT_OFFSET	0x08
#define ENERGY_UNIT_MASK	0x1F00

#define TIME_UNIT_OFFSET	0x10
#define TIME_UNIT_MASK		0xF0000

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int 
#define uint64_t unsigned long long
#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define NULL ((void*)0)

#define my_cpu_id() per_cpu_get(id)

typedef enum {
	V3_PWRSTAT_PKG_ENERGY,
	V3_PWRSTAT_CORE_ENERGY,
	V3_PWRSTAT_EXT_ENERGY, /* "Power plane 1", e.g. graphics peripheral */
	V3_PWRSTAT_DRAM_ENERGY,
} v3_pwrstat_ctr_t;

extern struct tracepoint __tracepoint_read_msr;
static inline void do_trace_read_msr(unsigned int msr, u64 val, int failed) {}

static inline unsigned long long notrace __rdmsr(unsigned int msr)
{
	DECLARE_ARGS(val, low, high);

	asm volatile("1: rdmsr\n"
		     "2:\n"
		     _ASM_EXTABLE_HANDLE(1b, 2b, ex_handler_rdmsr_unsafe)
		     : EAX_EDX_RET(val, low, high) : "c" (msr));

	return EAX_EDX_VAL(val, low, high);
}

static inline unsigned long long native_read_msr(unsigned int msr)
{
	unsigned long long val;

	val = __rdmsr(msr);

	if (msr_tracepoint_active(__tracepoint_read_msr))
		do_trace_read_msr(msr, val, 0);

	return val;
}

#define rdmsrl(msr, val)			\
	((val) = native_read_msr((msr)))

static int pwrstat_init (void);

#endif


