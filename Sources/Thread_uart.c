#include "LPC11xx.h"                    // Device header
#include "types.h"

#undef osObjectsPublic
#define osObjectsExternal
#include <cmsis_os.h>                                           // CMSIS RTOS header file
#include "osObjects.h"


int getchar(void);
void putchar(int);
void puts(char const *);

int Init_Thread_uart (void) {
	
	LPC_SYSCON->SYSAHBCLKCTRL |= ((1UL << 16) | 	/* Enable IOCON */
	                              (1UL << 6));    /* Enable GPIO */
	LPC_IOCON->PIO1_6 = ((1UL << 0) |							/* Enable RXD */
										   (2UL << 3) |							/* Enable pull-up bits */
	                     (3UL << 6) );						/* Reserved */
	LPC_IOCON->PIO1_7 = ((1UL << 0) |							/* Enable TXD */
										   (2UL << 3) |							/* Enable pull-up bits */
	                     (3UL << 6) );						/* Reserved */
	
	/* configure UART0 */
  LPC_SYSCON->SYSAHBCLKCTRL |=  (1UL << 12);    /* Enable clock to UART       */
  LPC_SYSCON->UARTCLKDIV     =  (4UL <<  0);    /* UART clock =  CCLK / 4     */
	
	LPC_UART->LCR = 0x83;								/* 8 bits, no parity, 1 stop bit, DLAB=1 */
	LPC_UART->DLL = 0x34;       				/* 9600 baud @ 12MHz */
	LPC_UART->DLM = 0x00;
	LPC_UART->FDR = (2UL << 4) | 1UL; 	/* FR = 1.5, DL = 52, DIV = 1, MUL = 2 */
	LPC_UART->LCR = 0x03;

  tid_Thread_uart = osThreadCreate (osThread(Thread_uart), NULL);
  if(!tid_Thread_uart) return(-1);
	
	puts("Hello world!\r\n");
  
  return(0);
}

void Thread_uart (void const *argument) {
	Display_Msg *msg;
	osStatus st;
	int idx = 0;
	char c;
	
	while (1) {
		idx = 0;
		msg = osMailAlloc(qid_display_msg, osWaitForever);
		if (!msg) continue;
		
		msg->scr = getchar() - '0';
		if ((0 > msg->scr) || (msg->scr > 9)) {
			osMailFree(qid_display_msg, msg);
			continue;
		}
		
		getchar();
		
		msg->clr = !!(getchar() - '0');
		
		getchar();
		
		while ((c = getchar()) != '\r' && c != '\n' && (idx + 1 < sizeof(msg->msg))) {
			msg->msg[idx++] = c;
		}
		
		msg->msg[idx] = 0;
		msg->length = idx;
		puts("Writing message: ");
		puts(msg->msg);
		puts("\r\n");
		
		st = osMailPut(qid_display_msg, msg);
		if (st != osOK)
			puts("Error putting message\r\n");
		msg = 0;
	}
}

int getchar() {
	int c;
	while (!(LPC_UART->LSR & 0x01));
	c = LPC_UART->RBR;
	putchar(c);
	if (c == '\r')
		putchar('\n');
	return c;
}

void putchar(int c) {
	while (!(LPC_UART->LSR & 0x20));
  LPC_UART->THR = c;
}


void puts(char const *c)
{
	while (*c) putchar(*c++);
}
