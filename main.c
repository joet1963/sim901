/* ###################################################################
**     Filename    : main.c
**     Project     : SIM901
**     Processor   : MKL25Z128VLK4
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2017-02-06, 09:47, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.01
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "UTIL1.h"
#include "WAIT1.h"
#include "AS1.h"
#include "ASerialLdd1.h"
#include "TI1.h"
#include "TimerIntLdd1.h"
#include "TU1.h"
#include "TmDt1.h"
#include "TimeDateLdd1.h"
#include "TU2.h"
#include "AD1.h"
#include "AdcLdd1.h"
#include "Bit1.h"
#include "BitIoLdd1.h"
#include "Bit2.h"
#include "BitIoLdd2.h"
#include "Bit3.h"
#include "BitIoLdd3.h"
#include "Bit4.h"
#include "BitIoLdd4.h"
#include "Bit5.h"
#include "BitIoLdd5.h"
#include "Bit6.h"
#include "BitIoLdd6.h"
#include "Bit7.h"
#include "BitIoLdd7.h"
#include "Bit8.h"
#include "BitIoLdd8.h"
#include "Bit9.h"
#include "BitIoLdd9.h"
#include "Bit10.h"
#include "BitIoLdd10.h"
#include "Bit11.h"
#include "BitIoLdd11.h"
#include "Bit12.h"
#include "BitIoLdd12.h"
#include "Bit13.h"
#include "BitIoLdd13.h"
#include "Bit14.h"
#include "BitIoLdd14.h"
#include "Bit15.h"
#include "BitIoLdd15.h"
#include "Bit16.h"
#include "BitIoLdd16.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* User includes (#include below this line is not maintained by Processor Expert) */

#include "atc.h"
#include "util.h"

const unsigned char sendtoN[] = {"+16134511711"};
AS1_TComData rxbuff[256] = {};
uint8_t rxbuffindex = 0;
TIMEREC MYTIME;
DATEREC MYDATE;
uint8_t k = 0;
word pwruptimer = 0;
uint32_t VERopCounter = 0;
uint32_t VERregCounter = 0;
//uint32_t txLOCK = 0;

int8_t tempo = -127;
word adcvalued = 0;
uint8_t serialtime = 0;
IOsema MYio;
IOsema MYmem;
VIBtype MYvib;
VIBtype MYrLED;
VIBtype MYgLED;
VIBtype MYbLED;
SWtype MYswitch;
resptype MYresp = {0};

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  /* For example: for(;;) { } */
  Bit11_SetVal(); // blue
  Bit12_SetVal();  // red
  Bit13_SetVal();  // green
  LEDdeinit(&MYrLED);
  LEDdeinit(&MYbLED);
  VIBinit(&MYvib, 1);
  LEDinit(&MYgLED, 0);

  //TmDt1_SetDate(2016, 2, 7); // intial date is wrong until update from CMT year > 2008 is valid
  //TmDt1_SetTime(12, 0, 0, 0);
  TmDt1_SetAlarm(14, 0, 0, 0);
  initrxbuff();
  IOinit(&MYio);
  MYmem = MYio;
  while (PWRseq()){}; // wait for module to power up
  pwruptimer = 0;
  while(pwruptimer < 3000){};
#ifdef PDU
  sendATstring("AT+CMGF=0\r"); //  PDU mode
#else
  sendATstring("AT+CMGF=1\r"); // text mode
#endif
  pwruptimer = 0;
  while(pwruptimer < 3000){};

  MYio.status = 1;

  for(;;)
    {

  	  if(VERop() != ERR_OK )(void)PWRseq();
  	  if(VERreg() != ERR_OK)(void)PWRseq();
  	  SerialService(&MYresp);
  	  supervisor(&MYio, &MYmem, rxbuff);
  	  pswitch(&MYswitch);
  	  Feature(&MYio);

    }




  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
