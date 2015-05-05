#include "types.h"
#include "display.h"

#include "osObjects.h"
#include <cmsis_os.h>
#include <string.h>

int Init_Thread_Display (void) {
  int i;
  
  qid_display_msg = osMailCreate(osMailQ(display_msg), NULL);

  tid_Thread_Display = osThreadCreate (osThread(Thread_Display), NULL);
  if(!tid_Thread_Display) return(-1);
  
  Display_Initialize();
  
  for (i = 0; i < 4; ++i) {
    Display_ChipSel(i);
    Display_Reset();
    Display_Clear();
    Display_Puts("Awaiting input...");
  }
  
  return(0);
}

void Thread_Display (void const *argument) {
  Display_Msg *msg = 0;
  char *c;
  int scr;
  osEvent evt;
  
  while (1) {
    evt = osMailGet(qid_display_msg, osWaitForever);
    if (evt.status == osEventMail) {
      msg = evt.value.p;
      
      c = msg->buf;
      
      // command: s <display> <message>
      //   writes a message to a display
      if (!strncmp(c, "s ", 2)) {
        c += 2;
        
        scr = *c - '0';
        if (scr > 9 || scr < 0) continue;
        c += 2;
        
        Display_ChipSel(scr);
        Display_Clear();
        
        Display_Puts(c);
      }
      
      // command: c <display>
      //    clears a display
      else if (!strncmp(c, "c ", 2)) {
        c += 2;
        
        scr = *c - '0';
        if (scr > 9 || scr < 0) continue;
        
        Display_ChipSel(scr);
        Display_Clear();
      }
      
      osMailFree(qid_display_msg, msg);
      msg = 0;
    }
  }
}


