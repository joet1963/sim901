/* ###################################################################
**     Filename    : Events.c
**     Project     : SIM901
**     Processor   : MKL25Z128VLK4
**     Component   : Events
**     Version     : Driver 01.00
**     Compiler    : GNU C Compiler
**     Date/Time   : 2017-02-06, 09:47, # CodeGen: 0
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Contents    :
**         Cpu_OnNMIINT - void Cpu_OnNMIINT(void);
**
** ###################################################################*/
/*!
** @file Events.c
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         
/* MODULE Events */

#include "Cpu.h"
#include "Events.h"

#ifdef __cplusplus
extern "C" {
#endif 


/* User includes (#include below this line is not maintained by Processor Expert) */

#include "util.h"

extern IOsema MYio;
extern VIBtype MYvib;
extern SWtype MYswitch;
extern VIBtype MYrLED;
extern VIBtype MYgLED;
extern VIBtype MYbLED;
extern uint32_t VERopCounter;
extern uint32_t VERregCounter;
extern word pwruptimer;
extern uint8_t serialtime;

/*
** ===================================================================
**     Event       :  Cpu_OnNMIINT (module Events)
**
**     Component   :  Cpu [MKL25Z128LK4]
*/
/*!
**     @brief
**         This event is called when the Non maskable interrupt had
**         occurred. This event is automatically enabled when the [NMI
**         interrupt] property is set to 'Enabled'.
*/
/* ===================================================================*/
void Cpu_OnNMIINT(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  TmDt1_OnAlarm (module Events)
**
**     Component   :  TmDt1 [TimeDate]
**     Description :
**         This event is called whenever an actual time is equal to the
**         alarm time (set by <SetAlarm> method).
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void TmDt1_OnAlarm(void)
{
  /* Write your code here ... */
	MYio.status = 1;
}

/*
** ===================================================================
**     Event       :  TI1_OnInterrupt (module Events)
**
**     Component   :  TI1 [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void TI1_OnInterrupt(void)
{
  /* Write your code here ... */

	VERopCounter++;
	VERregCounter++;
	pwruptimer++;

	MYswitch.Timer++;
	MYvib.Timer++;
	MYrLED.Timer++;
	MYgLED.Timer++;
	MYbLED.Timer++;

	MYio.Heattime++;
	MYio.Pumptime++;
	MYio.Pumpatime++;
	MYio.Tanktime++;
	MYio.Tankatime++;
	MYio.Primetime++;
	MYio.Primeatime++;

	VIBcont(&MYvib);
	Bit16_PutVal(MYvib.onState);
	LEDcont(&MYrLED);
	Bit12_PutVal(MYrLED.onState);
	LEDcont(&MYgLED);
	Bit13_PutVal(MYgLED.onState);
	LEDcont(&MYbLED);
	Bit11_PutVal(MYbLED.onState);
	MYswitch.state = Bit15_GetVal();  //read push button
	pswitch(&MYswitch);
	DriveIn(&MYio);    // inputs
	DriveOut(&MYio);  // outputs

}

/*
** ===================================================================
**     Event       :  AS1_OnRxChar (module Events)
**
**     Component   :  AS1 [AsynchroSerial]
**     Description :
**         This event is called after a correct character is received.
**         The event is available only when the <Interrupt
**         service/event> property is enabled and either the <Receiver>
**         property is enabled or the <SCI output mode> property (if
**         supported) is set to Single-wire mode.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void AS1_OnRxChar(void)
{
  /* Write your code here ... */

}

/* END Events */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

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
