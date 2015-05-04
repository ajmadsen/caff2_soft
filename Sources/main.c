#include "LPC11xx.h"                    // Device header
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "display.h"

void sys_err() { while(1) __WFE(); }

int main()
{
	int i;
	int disp = 0;
	
	osKernelInitialize();
	osKernelStart();
	
	osDelay(200); // wait before display initialization
	
	Display_Initialize();
	Display_ChipSel(0);
	Display_Reset();
	Display_Clear();
	Display_ChipSel(1);
	Display_Reset();
	Display_Clear();
	
	for (;;) {
		Display_ChipSel(disp);
		
		Display_Clear();
		Display_DispCtl(1, 0, 0);
		Display_Puts("Hello, world!");
		osDelay(1000);
		Display_Clear();
		Display_SetCurPos(8);
		Display_Puts("Hello");
		osDelay(1000);
		Display_Clear();
		Display_SetCurPos(0x40);
		Display_Puts("World!");
		osDelay(1000);
		Display_Clear();
		osDelay(500);
		
		for (i = 0; i < 5; ++i) {
			Display_Putc('0'+i);
			Display_IncCurPos();
			osDelay(500);
		}
		
		Display_EntryModeSet(1, 1);
		
		for (; i < 10; ++i) {
			Display_Putc('0'+i);
			Display_IncCurPos();
			Display_IncDispPos();
			osDelay(500);
		}
		
		Display_IncCurPos();
		Display_Puts("Hi");
		
		osDelay(500);
		Display_DispCtl(1, 1, 0);
		osDelay(500);
		Display_DispCtl(1, 0, 0);
		osDelay(500);
		Display_DispCtl(0, 0, 0);
		Display_EntryModeSet(1, 0);
		osDelay(1000);
		
		disp = !disp;
	}
	
}
