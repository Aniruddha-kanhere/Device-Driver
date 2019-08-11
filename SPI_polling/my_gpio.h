#ifndef __GPIO_H
#define __GPIO_H

#define BITS(x) (((u32)1)<<x)

#define GPIO_0_BASE 0x44E07000
#define GPIO_0_END  0x44E07FFF

#define GPIO_1_BASE 0x4804C000
#define GPIO_1_END  0x4804CFFF

#define GPIO_2_BASE 0x481AC000
#define GPIO_2_END  0x481ACFFF

#define GPIO_3_BASE 0x481AE000
#define GPIO_3_END  0x481AEFFF



#define GPIO0_START_ADDR GPIO_0_BASE
#define GPIO0_END_ADDR   GPIO_0_END
#define GPIO0_SIZE 	(GPIO0_END_ADDR - GPIO0_START_ADDR)

#define GPIO1_START_ADDR GPIO_1_BASE
#define GPIO1_END_ADDR   GPIO_1_END
#define GPIO1_SIZE 	(GPIO1_END_ADDR - GPIO1_START_ADDR)

#define GPIO2_START_ADDR GPIO_2_BASE
#define GPIO2_END_ADDR   GPIO_2_END
#define GPIO2_SIZE 	(GPIO2_END_ADDR - GPIO2_START_ADDR)

#define GPIO3_START_ADDR GPIO_3_BASE
#define GPIO3_END_ADDR   GPIO_3_END
#define GPIO3_SIZE 	(GPIO3_END_ADDR - GPIO3_START_ADDR)



//FIX ME: Don't actually need this. Was using this as test
#define Offset_define(str)			 	           \
volatile unsigned long str##_REVISION		= 0x0	;          \
volatile unsigned long str##_SSYCONFIG		= 0x10	;       \
volatile unsigned long str##_EOI	        = 0x20	;            \
volatile unsigned long str##_IRQSTATUS_RAW_0	= 0x24	;  \
volatile unsigned long str##_IRQSTATUS_RAW_1	= 0x28	;  \
volatile unsigned long str##_IRQSTATUS_0	= 0x2C	;      \
volatile unsigned long str##_IRQSTATUS_1	= 0x30	;      \
volatile unsigned long str##_IRQSTATUS_SET_0	= 0x34	;  \
volatile unsigned long str##_IRQSTATUS_SET_1	= 0x38	;  \
volatile unsigned long str##_IRQSTATUS_CLR_0	= 0x3C	;  \
volatile unsigned long str##_IRQSTATUS_CLR_1	= 0x40	;  \
volatile unsigned long str##_IRQWAKEN_0		= 0x44	;      \
volatile unsigned long str##_IRQWAKEN_1		= 0x48	;      \
volatile unsigned long str##_SSYSTATUS		= 0x114	;      \
volatile unsigned long str##_CTRL		= 0x130	;            \
volatile unsigned long str##_OE			= 0x134	;            \
volatile unsigned long str##_DATAIN		= 0x138	;          \
volatile unsigned long str##_DATAOUT		= 0x13C	;        \
volatile unsigned long str##_LEVELDETECT0	= 0x140	;      \
volatile unsigned long str##_LEVELDETECT1	= 0x144	;      \
volatile unsigned long str##_RISINGDETECT	= 0x148	;      \
volatile unsigned long str##_FALLINGDETECT	= 0x14C	;    \
volatile unsigned long str##_DEBOUNCABLE	= 0x150	;      \
volatile unsigned long str##_DEBOUNCINGTIME	= 0x154	;    \
volatile unsigned long str##_CLEARDATAOUT	= 0x190	;      \
volatile unsigned long str##_SETDATAOUT		= 0x194;


#endif
