/*
 * util.c
 *
 *  Created on: Jan 13, 2017
 *      Author: Magu
 */

#include "util.h"

extern TIMEREC MYTIME;
extern DATEREC MYDATE;
extern AS1_TComData rxbuff[256];
extern const unsigned char sendtoN[] ;
extern resptype MYresp;
extern IOsema MYio;
extern word msCounter1;

extern uint32_t VERopCounter;
extern uint32_t VERregCounter;
extern bool CRDY;
extern bool FUNC;
extern bool RDY;
extern bool OK;
extern byte REGn;
extern byte REGc;
extern word pwruptimer;


void pswitch(SWtype *sw)
{
	 if(sw->state == 1){ sw->Timer = 0; sw->pshort = 0; sw->plong = 0; }
	 else if(sw->Timer > 200) sw->plong = 1;
	 else if(sw->Timer > 50) sw->pshort = 1;

	 if(sw->plong){ MYio.status = 1; sw->plong = 0;}

}


bool PWRseq(void)
{
	resptype *resp = &MYresp;

	word timeout = 1000;
	resp->RDY = 0; // clear flags
	resp->FUNC = 0;
	Bit14_ClrVal();

	while(timeout){  // wait 1 sec
		WAIT1_Waitms(1);
		timeout--;
	}

	Bit12_ClrVal();   // confirm with Red LED flash
	Bit14_SetVal(); // drive module power switch
	timeout = 1000;

	while(timeout){  // wait 1 sec
		WAIT1_Waitms(1);
		timeout--;
	}

	Bit14_ClrVal();
	Bit12_SetVal();
	//verify
	timeout = 4000;
	while(timeout)
	{
		SerialService(resp);
		WAIT1_Waitms(1);
		timeout--;
		if(resp->RDY == 1 || resp->FUNC == 1 ) return ERR_OK;
	}
	return ERR_NOTAVAIL;
}


bool VERop(void)
{
	resptype *resp = &MYresp;
	if(VERopCounter > 190000) // every 3 mins
	{
		word timeout = 1000;
		resp->OK = 0;
		VERopCounter = 0;
		sendATstring("AT\r"); // check module
		while(timeout)
		{
			SerialService(resp);
			WAIT1_Waitms(1);
			timeout--;
			if(resp->OK == 1) return ERR_OK;
		}
		return ERR_NOTAVAIL; // no response
	}
	return ERR_OK;
}

uint8_t VERreg(void)
{
	resptype *resp = &MYresp;

	if(VERregCounter > 380000) //
	{
		word timeout = 3000;
		resp->REGn = 0xFF; // clear for a new update
		VERregCounter = 0;
		sendATstring("AT+CREG?\r"); // check registration
		while(timeout)
		{
			SerialService(resp);
			WAIT1_Waitms(1);
			timeout--;
			if(resp->REGn == 1) {resp->REGn = 0; resp->REGc = 0; return ERR_OK;}
		}
		return ERR_NOTAVAIL;
	}

	return ERR_OK;
}

void IOinit(IOsema *io)
{
	io->Q1 = 0;
	io->Q2 = 0;
	io->Q3 = 0;
	io->RLY1 = 0;
	io->RLY2 = 0;
	io->dailyrpt = 0;
	io->status = 1;
	io->Heat = 0;
	io->Pump = 0;
	io->Tank = 0;
	io->Prime = 0;
};

void DriveOut(IOsema *io)
{
	Bit8_PutVal(io->Q1);
	Bit9_PutVal(io->Q2);
	Bit10_PutVal(io->Q3);
	Bit6_PutVal(io->RLY1);
	Bit7_PutVal(io->RLY2);
};

void DriveIn(IOsema *io)
{
	io->INPUT1 = Bit1_GetVal();
	io->INPUT2 = Bit2_GetVal();
	io->INPUT3 = Bit3_GetVal();
	io->INPUT4 = Bit4_GetVal();
	io->INPUT5 = Bit5_GetVal();
}


void supervisor(IOsema *io, IOsema *mem, char *rxbuff)
{
	bool send = 0;
	char mess[140] = {0};
	mess[0] = '\0'; //intialize buffer


	if(io->INPUT1 ^ mem->INPUT1)   // on change do something!
	{
		UTIL1_strcat(mess, sizeof(mess), "INPUT1=");
		UTIL1_strcatNum8u(mess, sizeof(mess), io->INPUT1 );
		UTIL1_strcat(mess, sizeof(mess), "\r\n");
		mem->INPUT1 = io->INPUT1;
		send = 1;
	}

	if(io->INPUT2 ^ mem->INPUT2)
	{
		UTIL1_strcat(mess, sizeof(mess), "INPUT2=");
		UTIL1_strcatNum8u(mess, sizeof(mess), io->INPUT2 );
		UTIL1_strcat(mess, sizeof(mess), "\r\n");
		mem->INPUT2 = io->INPUT2;
		send = 1;
	}

	if(io->INPUT3 ^ mem->INPUT3)
	{
		UTIL1_strcat(mess, sizeof(mess), "INPUT3=");
		UTIL1_strcatNum8u(mess, sizeof(mess), io->INPUT3 );
		UTIL1_strcat(mess, sizeof(mess), "\r\n");
		mem->INPUT3 = io->INPUT3;
		send = 1;
	}

	if(io->INPUT4 ^ mem->INPUT4)
	{
		UTIL1_strcat(mess, sizeof(mess), "INPUT4=");
		UTIL1_strcatNum8u(mess, sizeof(mess), io->INPUT4 );
		UTIL1_strcat(mess, sizeof(mess), "\r\n");
		mem->INPUT4 = io->INPUT4;
		send = 1;
	}

	if(io->INPUT5 ^ mem->INPUT5)
	{
		UTIL1_strcat(mess, sizeof(mess), "INPUT5=");
		UTIL1_strcatNum8u(mess, sizeof(mess), io->INPUT5 );
		UTIL1_strcat(mess, sizeof(mess), "\r\n");
		mem->INPUT5 = io->INPUT5;
		send = 1;
	}

	if(send) sendMessage(sendtoN, mess, rxbuff, &MYresp);

	if(io->status){
		sendSTATUS(io, rxbuff);
	}

	if(io->config){
		sendCONFIG(io, rxbuff);
	}


}

void sendSTATUS(IOsema *io, char *rxbuff)
{
	char mess[140] = {0};
	mess[0] = '\0'; //intialize buffer
	word adcvalue;
	int8_t Temp = 0xFF;

	while (AD1_Measure(1)){};
	(void)AD1_GetChanValue16(1, &adcvalue);

	io->messcnt++;  // message counter
	UTIL1_strcat(mess, sizeof(mess), "MESSAGE# ");
	UTIL1_strcatNum16u(mess, sizeof(mess), io->messcnt );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "INPUT1=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->INPUT1 );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "INPUT2=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->INPUT2 );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "INPUT3=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->INPUT3 );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "INPUT4=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->INPUT4 );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "INPUT5=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->INPUT5 );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");

	UTIL1_strcat(mess, sizeof(mess), "RLY1=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->RLY1 );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "RLY2=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->RLY2 );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "Q1=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->Q1 );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "Q2=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->Q2 );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "Q3=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->Q3 );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	Temp = (int8_t)25 - ((adcvalue - 14219)/35);
	UTIL1_strcat(mess, sizeof(mess), "Temperature=");
	UTIL1_strcatNum8s(mess, sizeof(mess), Temp);
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "Set Point=");
	UTIL1_strcatNum8s(mess, sizeof(mess), io->setPoint);

	if(sendMessage(sendtoN, mess, rxbuff, &MYresp) == ERR_OK) io->status = 0;
}


void sendCONFIG(IOsema *io, char *rxbuff)
{
	char mess[140] = {0};
	mess[0] = '\0'; //intialize buffer

	io->messcnt++;  // message counter
	UTIL1_strcat(mess, sizeof(mess), "MESSAGE# ");
	UTIL1_strcatNum16u(mess, sizeof(mess), io->messcnt );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "HEAT=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->Heat );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "PUMP=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->Pump );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "TANK=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->Tank );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "PRIME=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->Prime );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "FEAT5=");
	UTIL1_strcatNum8u(mess, sizeof(mess), io->Feat5 );
	UTIL1_strcat(mess, sizeof(mess), "\r\n");

	UTIL1_strcat(mess, sizeof(mess), "Set Point=");
	UTIL1_strcatNum8s(mess, sizeof(mess), io->setPoint);
	UTIL1_strcat(mess, sizeof(mess), "\r\n");
	UTIL1_strcat(mess, sizeof(mess), "Pump Point=");
	UTIL1_strcatNum8s(mess, sizeof(mess), io->pumpPoint);


	if(sendMessage(sendtoN, mess, rxbuff, &MYresp) == ERR_OK) io->config = 0;
}

void Feature(IOsema *io)
{

	if(io->Heat && io->Heattime > 60000){ // every 60 secs

		word adcvalue;
		int8_t Temp = 0xFF;
		uint8_t day = 1;

		TmDt1_GetDate(&MYDATE);
		TmDt1_GetTime(&MYTIME);
		TmDt1_GetDay(&day);
		if(MYDATE.Year > 2008 && (MYTIME.Hour < 7 || MYTIME.Hour >= 19 || day == 0 || day == 6)) // check to see if it's valid date > 2008
		{                                                                                         // before 7hrs after 19hrs sat and sun OK
			io->Heattime = 0;
			while (AD1_Measure(1)){};
			(void)AD1_GetChanValue16(1, &adcvalue);
			Temp = (int8_t)25 - ((adcvalue - 14219)/35);
			if(Temp < io->setPoint) io->RLY1 = 1;
			else io->RLY1 = 0;

		}
	}

	if(io->Pump && io->Pumptime > 3600000){  // every 60 mins

		word adcvalue;
		int8_t Temp = 0xFF;

		while (AD1_Measure(1)){};
		(void)AD1_GetChanValue16(1, &adcvalue);
		Temp = (int8_t)25 - ((adcvalue - 14219)/35);
		if(Temp < io->pumpPoint)
		{
			io->Pumptime = 0;
			io->Pumpa = 1;
			io->Pumpatime = 0;
			io->Q1 = 1;
		}
	}

	if(io->Pumpa &&io->Pumpatime > 90000){  // for 90 seconds

		io->Pumpa = 0;
		io->Q1 = 0;

	}

	if(io->Tank && io->Tanktime > 7200000){ // every 120 mins

		uint8_t day = 1;

		TmDt1_GetDate(&MYDATE);
		TmDt1_GetTime(&MYTIME);
		TmDt1_GetDay(&day);

		if(MYDATE.Year > 2008 && (MYTIME.Hour < 7 || MYTIME.Hour >= 19 || day == 0 || day == 6))
		{
			io->Tanktime = 0; // clear timer
			io->Tanka = 1;
			io->Tankatime = 0;
			io->Q2 = 1;
		}

	}

	if(io->Tanka && io->Tankatime > 1200000){

		io->Tanka = 0;
		io->Q2 = 0;

	}

	if(io->Prime && io->Primetime > 43200000){ // every 12 hours

		io->Prime = 0;
		io->Primea = 1;
		io->Primeatime = 0;
		io->Q1 = 1;
	}

	if(io->Primea && io->Primeatime > 180000){ // for 3 mins

		io->Primea = 0;
		io->Q1 = 0;
	}
}



void VIBdeinit(VIBtype *vib)
{
	vib->onState = 0;
	vib->active = 0;
}

void LEDdeinit(VIBtype *vib)
{
	vib->onState = 1; // inverted for LED
	vib->active = 0;
}

void VIBinit(VIBtype *vib, byte mode)
{
	if(mode == 0)
	{
		vib->active = 1;
		vib->offTime = 1975;
		vib->onTime = 25;
		vib->reps = 0xFF; // never turn off
	}
	else if(mode == 1)
	{
		vib->active = 1;
		vib->offTime = 300;
		vib->onTime = 700;
		vib->reps = 1;
	}
	else if(mode == 2)
	{
		vib->active = 1;
		vib->offTime = 250;
		vib->onTime = 400;
		vib->reps = 2;
	}
	else if(mode >= 3)
	{
		vib->active = 1;
		vib->offTime = 900;
		vib->onTime = 100;
		vib->reps = 180;
	}

	vib->Timer = 0;
	vib->onState = 1;
}

void LEDinit(VIBtype *vib, byte mode)
{
	if(mode == 0)
	{
		vib->active = 1;
		vib->offTime = 1975;
		vib->onTime = 25;
		vib->reps = 0xFF; // never turn off
	}
	else if(mode == 1)
	{
		vib->active = 1;
		vib->offTime = 200;
		vib->onTime = 175;
		vib->reps = 2;
	}
	else if(mode == 2)
	{
		vib->active = 1;
		vib->offTime = 200;
		vib->onTime = 175;
		vib->reps = 10;
	}
	else if(mode >= 3)
	{
		vib->active = 1;
		vib->offTime = 900;
		vib->onTime = 100;
		vib->reps = 180;
	}

	vib->Timer = 0;
	vib->onState = 1;
}

void VIBcont(VIBtype *vib)
{
	if(vib->active && vib->onState == 1 && vib->Timer > vib->onTime)
	{
		vib->Timer = 0;
		vib->onState = 0;
		if(vib->reps != 0xFF){ if(--vib->reps == 0) vib->active = 0; }
	}

	else if(vib->active && vib->onState == 0 && vib->Timer > vib->offTime)
	{
		vib->Timer = 0;
		vib->onState = 1;
	}
}

void LEDcont(VIBtype *vib)
{
	if(vib->active && vib->onState == 0 && vib->Timer > vib->onTime)
	{
		vib->Timer = 0;
		vib->onState = 1; // turn off
		if(vib->reps != 0xFF){ if(--vib->reps == 0) vib->active = 0; }
	}

	else if(vib->active && vib->onState == 1 && vib->Timer > vib->offTime)
	{
		vib->Timer = 0;
		vib->onState = 0;  // turn on
	}
}


/*
** ===================================================================
**     Method      :  ScanDate
**     Description :
**         Scans a date in the format "yy/mm/dd"
**
**     Parameters  :
**         NAME            - DESCRIPTION
**       * str             - Pointer to the string to be scanned. The
**                           function advances the pointer.
**       * day             - Pointer to where to store the day value
**       * month           - Pointer to where to store the month
**                           value
**       * year            - Pointer to where to store the year value
**     Returns     :
**         ---             - Error code
** ===================================================================
*/
uint8_t ScanDate(const unsigned char **str, uint8_t *day, uint8_t *month, uint8_t *year)
{
  /* precondition: string points to starting of date, e.g. "01.01.10" or "12.5.2010", and date is in format dd.mm.yy or dd.mm.yyyy */
   const unsigned char *p;

  p = *str;
  while(*p==' ' || *p=='\"' || *p == ',') p++; /* skip leading spaces */

  if( UTIL1_ScanDecimal8uNumber(&p, year)==ERR_OK  && *year > 0 && *year <= 99 && (*p=='/'))
  {
	  p++;
	  if( UTIL1_ScanDecimal8uNumber(&p, month)==ERR_OK && *month > 0 && *month <= 12 && (*p=='/'))
	  {
		  p++;
		  if( UTIL1_ScanDecimal8uNumber(&p, day)==ERR_OK && *day > 0 && *day <= 31)
		  {
			  p++; // skip over comma between date and time
			  *str = p; /* advance pointer for caller */
			  return ERR_OK;
		  }
	  }
  }
  return ERR_FAILED; /* wrong format */
}



