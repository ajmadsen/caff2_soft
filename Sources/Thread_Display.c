#include "types.h"
#include "display.h"

#undef osObjectsPublic
#define osObjectsExternal
#include <cmsis_os.h>                                           // CMSIS RTOS header file
#include "osObjects.h"


/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/


int Init_Thread_Display (void) {
	
	qid_display_msg = osMailCreate(osMailQ(display_msg), NULL);

  tid_Thread_Display = osThreadCreate (osThread(Thread_Display), NULL);
  if(!tid_Thread_Display) return(-1);
	
	Display_Initialize();
	
	Display_ChipSel(0);
	Display_Reset();
	Display_Clear();
	Display_Puts("Awaiting input...");
	Display_ChipSel(1);
	Display_Reset();
	Display_Clear();
	Display_Puts("Awaiting input...");
  
  return(0);
}

void Thread_Display (void const *argument) {
	Display_Msg *msg = 0;
	osEvent evt;
	
  while (1) {
    evt = osMailGet(qid_display_msg, osWaitForever);
		if (evt.status == osEventMail) {
			msg = evt.value.p;
			
			Display_ChipSel(msg->scr);
			if (msg->clr) Display_Clear();
			
			Display_Puts(msg->msg);
			
			osMailFree(qid_display_msg, msg);
			msg = 0;
		} else if (evt.status == osOK) {
			__breakpoint(0);
		}
  }
}


