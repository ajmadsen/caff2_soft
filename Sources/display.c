#include <stdint.h>

#include "LPC11xx.h"                    // Device header
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

#include "display.h"

#define D_ENABLE(en) do { \
	LPC_GPIO1->MASKED_ACCESS[1UL << 2] = ~((unsigned)(en)) << 2; \
} while (0)

#define D_SHF_RESET(rst) do { \
	LPC_GPIO0->MASKED_ACCESS[1UL << 7] = ~((unsigned)(rst)) << 7; \
} while (0)

#define D_SHF_DATA(data) do { \
	LPC_GPIO0->MASKED_ACCESS[1UL << 2] = (data) << 2; \
} while (0)

#define D_SHF_CLK(clk) do { \
	LPC_GPIO0->MASKED_ACCESS[1UL << 3] = (clk) << 3; \
} while (0)

#define D_REG_SEL(reg) do { \
	LPC_GPIO1->MASKED_ACCESS[1UL << 0] = (reg); \
} while (0)

#define D_DIR_SEL(read) do { \
	if (read) LPC_GPIO1->DIR &= ~(0x0FUL << 4); \
	else      LPC_GPIO1->DIR |= 0x0FUL << 4; \
	LPC_GPIO1->MASKED_ACCESS[1UL << 1] = (read) << 1; \
	LPC_GPIO1->MASKED_ACCESS[1UL << 8] = !(read) << 8; \
} while (0)

#define D_WRITE(data) do { \
	LPC_GPIO1->MASKED_ACCESS[0x0FUL << 4] = (data) << 4; \
} while (0)

#define D_DATA ((LPC_GPIO1->MASKED_ACCESS[0x0FUL << 4]) >> 4)

#define D_DELAY(len) do { \
	uint32_t tick_ = osKernelSysTick(); \
	do { osThreadYield(); } while ((osKernelSysTick() - tick_) < osKernelSysTickMicroSec(len)); \
} while (0)

static void Nybble(void);
static void d_write_half(char cmd);
static void d_command(char cmd, int wait);
static void d_data(char data, int wait);
static int32_t Display_GetAddr_int(void);

int32_t Display_Initialize()
{
	LPC_SYSCON->SYSAHBCLKCTRL |= ((1UL <<  6) |		/* GPIO CLK Enable */
																(1UL << 16) );  /* IOCON CLK Enable */
	
	LPC_IOCON->R_PIO1_0 = ((1UL << 0) |						/* Select GPIO function */
												 (2UL << 3) |						/* Select pull-up */
												 (2UL << 6) );					/* Keep reserved values */
	
	LPC_IOCON->R_PIO1_1 = ((1UL << 0) |						/* Select GPIO function */
												 (2UL << 3) |						/* Select pull-up */
												 (2UL << 6) );					/* Keep reserved values */
	
	LPC_IOCON->R_PIO1_2 = ((1UL << 0) |						/* Select GPIO function */
												 (2UL << 3) |						/* Select pull-up */
												 (2UL << 6) );					/* Keep reserved values */
	
	LPC_GPIO1->DIR  |= 0x1F7UL; 									/* Pins 0-2, 4-8 configured as outputs */
	LPC_GPIO1->DATA &= ~0x1F7UL; 									/* Clear outputs */
	LPC_GPIO0->DIR  |= 0x08CUL;									  /* Pins 3, 4, 7 configured as outputs */
	LPC_GPIO0->DATA &= ~0x08CUL;									/* Clear outputs */
	
	D_ENABLE(0);
	D_REG_SEL(0);
	D_DIR_SEL(0);
	
	return 0;
};

int32_t Display_Uninitialize()
{
	return 0;
};

int32_t Display_ChipSel(int8_t chip)
{
	D_ENABLE(0);
	
	// reset shift register
	D_SHF_RESET(1);
	D_DELAY(1);
	D_SHF_RESET(0);
	D_DELAY(1);
	
	// shift in first bit
	D_SHF_DATA(1);
	D_SHF_CLK(1);
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	D_SHF_CLK(0);
	D_SHF_DATA(0);
	
	// shift bit over as necessary
	while (chip) {
		chip--;
		__NOP();
		__NOP();
		__NOP();
		__NOP();
		D_SHF_CLK(1);
		__NOP();
		__NOP();
		__NOP();
		__NOP();
		D_SHF_CLK(0);
	}
	
	return 0;
}

int32_t Display_Reset()
{
	D_ENABLE(0);
	D_REG_SEL(0);
	D_DIR_SEL(0);
	
	d_write_half(0x03); // Wakeup
	D_DELAY(37);

	d_command(0x28, 0);	// Function set
	D_DELAY(37);
	d_command(0x28, 0);	// Function set (try 2)
	D_DELAY(37);
	
	Display_EntryModeSet(1, 0);
	Display_DispCtl(1, 1, 0);
	
	return 0;
}

int32_t Display_Puts(const char *msg)
{
	while (*msg) {
		d_data(*msg++, 1);
	}
	return 0;
}

int32_t Display_Putc(const char chr)
{
	d_data(chr, 1);
	return 0;
}

int32_t Display_Clear()
{
	d_command(0x01, 1);
	//D_DELAY(1520);
	return 0;
}

int32_t Display_SetCurPos(int8_t pos)
{
	d_command(0x80 | pos, 1);
	//D_DELAY(37);
	return 0;
}

int32_t Display_ClrCurPos()
{
	d_command(0x02, 1);
	//D_DELAY(1520);
	return 0;
}

int32_t Display_IncCurPos()
{
	d_command(0x10 | 0x04, 1);
	//D_DELAY(37);
	return 0;
}

int32_t Display_DecCurPos()
{
	d_command(0x10, 1);
	//D_DELAY(37);
	return 0;
}

int32_t Display_ShfCurPos(int8_t cnt)
{
	if (cnt < 0) {
		while (cnt++) {
			Display_IncCurPos();
		}
	}
	else {
		while (cnt--) {
			Display_DecCurPos();
		}
	}
	return 0;
}

int32_t Display_EntryModeSet(int8_t incr, int8_t shift)
{
	d_command(0x04 | (!!incr << 1) | !!shift, 1);
	//D_DELAY(37);
	return 0;
}

int32_t Display_IncDispPos()
{
	d_command(0x10 | 0x08, 1);
	//D_DELAY(37);
	return 0;
}

int32_t Display_DecDispPos()
{
	d_command(0x10 | 0x08 | 0x04, 1);
	//D_DELAY(37);
	return 0;
}

int32_t Display_ShfDispPos(int8_t cnt)
{
	if (cnt < 0) {
		while (cnt++) {
			Display_IncDispPos();
		}
	}
	else {
		while (cnt--) {
			Display_DecDispPos();
		}
	}
	return 0;
}

int32_t Display_FuncSet(int8_t nlines, int8_t font)
{
	d_command(0x20 | (!!nlines << 3) | (!!font << 2), 1);
	//D_DELAY(37);
	return 0;
}


int32_t Display_DispCtl(int8_t disp_on, int8_t cursor_on, int8_t cursor_blink)
{
	d_command(0x08 | (!!disp_on << 2) | (!!cursor_on << 1) | !!cursor_blink, 1);
	//D_DELAY(37);
	return 0;
}

int32_t Display_SetCgramAddr(int8_t addr)
{
	d_command(0x40 | (addr & 0x3F), 1);
	//D_DELAY(37);
	return 0;
}

int32_t Display_SetDdramAddr(int8_t addr)
{
	d_command(0x80 | (addr & 0x7F), 1);
	//D_DELAY(37);
	return 0;
}

int32_t Display_GetAddr()
{
	return Display_GetAddr_int() & 0x7F;
}


int32_t Display_IsBusy()
{
	return !!(Display_GetAddr_int() & 0x80);
}

static int32_t Display_GetAddr_int()
{
	int32_t data;
	
	D_WRITE(0x00);
	D_REG_SEL(0);
	D_DIR_SEL(1);
	
	D_ENABLE(1);
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	data = D_DATA;
	D_ENABLE(0);
	
	D_WRITE(0x00);
	D_REG_SEL(0);
	D_DIR_SEL(1);
	
	D_ENABLE(1);
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	data = (data << 4) | D_DATA;
	D_ENABLE(0);
	
	return data;
}

int32_t Display_PutData(int8_t data)
{
	d_data(data, 1);
	//D_DELAY(37);
	return 0;
}

int32_t Display_GetData()
{
	int32_t data;
	
	D_WRITE(0xFF);
	D_REG_SEL(1);
	D_DIR_SEL(1);
	
	D_ENABLE(1);
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	data = D_DATA;
	D_ENABLE(0);
	
	D_WRITE(0xFF);
	D_REG_SEL(1);
	D_DIR_SEL(1);
	
	D_ENABLE(1);
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	data = (data << 4) | D_DATA;
	D_ENABLE(0);
	
	//D_DELAY(37);
	
	return data;
}

int32_t Display_WaitBusy()
{
	while (Display_IsBusy()) osThreadYield();
	return 0;
}

static void Nybble()
{
	D_ENABLE(1);
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	D_ENABLE(0);
	__NOP();
	__NOP();
	__NOP();
	__NOP();
}

static void d_command(char cmd, int wait)
{
	if (wait) Display_WaitBusy();
	D_DIR_SEL(0);
	D_REG_SEL(0);
	d_write_half(cmd >> 4);
	d_write_half(cmd);
}

static void d_write_half(char cmd)
{
	D_WRITE(cmd);
	Nybble();
}

static void d_data(char data, int wait)
{
	if (wait) Display_WaitBusy();
	D_DIR_SEL(0);
	D_REG_SEL(1);
	d_write_half(data >> 4);
	d_write_half(data);
}
