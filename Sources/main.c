#include "LPC11xx.h"                    // Device header
#include "display.h"

#define osObjectsPublic
#include "osObjects.h"

#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

osMailQId qid_display_msg;
osThreadId tid_Thread_uart;
osThreadId tid_Thread_Display;

void sys_err() { while(1) __WFE(); }

int main()
{
	osKernelInitialize();
	osDelay(200); // wait before display initialization
	
	Init_Thread_Display();
	Init_Thread_uart();
	
	osKernelStart();	
}
