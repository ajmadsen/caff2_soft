#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

int32_t Display_Initialize(void);
int32_t Display_Uninitialize(void);
int32_t Display_ChipSel(int8_t);
int32_t Display_Reset(void);
int32_t Display_Puts(const char*);
int32_t Display_Putc(const char);
int32_t Display_Clear(void);
int32_t Display_SetCurPos(int8_t);
int32_t Display_ClrCurPos(void);
int32_t Display_IncCurPos(void);
int32_t Display_DecCurPos(void);
int32_t Display_ShfCurPos(int8_t);
int32_t Display_EntryModeSet(int8_t, int8_t);
int32_t Display_IncDispPos(void);
int32_t Display_DecDispPos(void);
int32_t Display_ShfDispPos(int8_t);
int32_t Display_FuncSet(int8_t, int8_t);
int32_t Display_DispCtl(int8_t, int8_t, int8_t);
int32_t Display_SetCgramAddr(int8_t);
int32_t Display_SetDdramAddr(int8_t);
int32_t Display_GetAddr(void);
int32_t Display_IsBusy(void);
int32_t Display_PutData(int8_t);
int32_t Display_GetData(void);
int32_t Display_WaitBusy(void);

#endif // DISPLAY_H
