/*
 * atc.c
 *
 *  Created on: Jan 4, 2017
 *      Author: Magu
 */

#include "atc.h"

//#define  NULL ((void*)0)

extern TIMEREC MYTIME;
extern DATEREC MYDATE;
extern AS1_TComData rxbuff[];
extern uint8_t rxbuffindex;
extern const unsigned char sendtoN[];
extern IOsema MYio;
extern VIBtype MYbLED;
extern VIBtype MYrLED;
extern VIBtype MYvib;
extern resptype MYresp;

extern const acttable actionarray[];

const static char *nmesstable[] = {"+CFUN:","+CPIN:","+PACSP:","Call Ready","RING","OK", NULL};





static void PARSEmess(const unsigned char *stream, acttable *actionarray);
static uint8_t Parse(const unsigned char **stream, const unsigned char *cmp );
static uint8_t findCMT(const unsigned char *stream);
static uint8_t findCLIP(const unsigned char *stream);
static uint8_t findCMGS(const unsigned char *stream);
static uint8_t findSTATUS(const unsigned char *stream, char *status);
static uint8_t findCREG(const unsigned char *stream);


void initrxbuff(void)
{
	rxbuff[0] = '\0'; /* initialize buffer */
	rxbuffindex = 0;
}


void SerialService(resptype *resp)
{
	AS1_TComData Chr = 0;


	while(AS1_GetCharsInRxBuf() > 0 )
	{

		if(AS1_RecvChar(&Chr) == ERR_OK)
		{
			rxbuff[rxbuffindex++] = Chr;
			rxbuff[rxbuffindex] = '\0'; // terminate buffer

			if(Chr == '\n')
			{
			 //parse one line at a time and reset buffer after parse

				if(resp->waitfmess) { PARSEmess(rxbuff, actionarray); resp->waitfmess = 0; }
				else if(findCMT(rxbuff) == 0) resp->waitfmess = 1;
				else if(findCLIP(rxbuff) == 0) resp->CLIP = 1;
				else if(findCMGS(rxbuff) == 0) resp->sent = 1;
				else if(findCREG(rxbuff) == 0) resp->REGc = 1;
				else if(findSTATUS(rxbuff, "OK") == 0) resp->OK = 1;
				else if(findSTATUS(rxbuff, "ERROR") == 0) resp->ERR = 1;
				else if(findSTATUS(rxbuff, "RING") == 0) resp->RING = 1;
				else if(findSTATUS(rxbuff, "Call Ready") == 0) resp->CRDY = 1;
				else if(findSTATUS(rxbuff, "+CFUN:")) resp->FUNC = 1;
				else if(findSTATUS(rxbuff, "RDY")) resp->RDY = 1;
				initrxbuff();
			}
		}
	}
}


void sendATstring(AS1_TComData *CH)
{
	while(*CH != '\0')
	{
		while( AS1_SendChar(*CH) != ERR_OK ){}
		CH++;
	}
}


static uint8_t Match(const unsigned char *stream, const unsigned char *cmp, uint8_t len)
{
	const unsigned char *p = stream;
	uint8_t i = 0;

	while(p[i] == cmp[i])
	{
		i++;
		if(--len == 0)  return 0;  // match
	}
	return 1;
}



static uint8_t Parse( const unsigned char **stream, const unsigned char *cmp )
{
	const unsigned char *p = *stream;
	uint8_t len = 0;

	while(cmp[len] != '\0') len++;  // find length

	while(*p != '\0')
	{
		if(*p == *cmp ){
			if(Match(p, cmp, len) == 0){
				*stream = p; // pointer advanced to start of cmp
				return 0;
			}
		}
		p++; // next location in stream
	}
	return 1;
}


static uint8_t findCMT(const unsigned char *stream)
{
	const unsigned char *rptr, *ptr = stream;
	uint8_t day, month, year, hour, minute, second, hSecond, TDFlg = 0;

	if(Parse( &ptr, "+CMT:" ) == 0){

		if(Parse( &ptr, sendtoN ) == 0){

			while( *ptr !='\0' && TDFlg == 0)
			{
				rptr = ptr;	// transfer pointer because it will be advanced by ScanDate function
				if( ScanDate(&rptr, &day, &month, &year) == ERR_OK )
				{
					TDFlg = 1;   // look for date
					TmDt1_SetDate((word)(year+2000), month, day);
				}
				ptr++;
			}

			if(TDFlg == 1)
			{
				TDFlg = 0;
				ptr = rptr; // advance pointer
				while( *ptr !='\0' && TDFlg == 0)
				{
					rptr = ptr;	// transfer pointer because it will be advanced by ScanTime function
					if( UTIL1_ScanTime(&rptr, &hour, &minute, &second, &hSecond ) == ERR_OK)
					{
						TDFlg = 1;
						TmDt1_SetTime(hour, minute, second, hSecond);
						LEDinit(&MYbLED, 3);  // flash Blue 3 mins
						VIBinit(&MYvib, 2);  // vibrate
						TmDt1_GetDate(&MYDATE);
						TmDt1_GetTime(&MYTIME);
						return 0;
					}
					ptr++;
				}
			}
		}
	}
	return 1;
}



static uint8_t findCREG(const unsigned char *stream)
{
	const unsigned char *ptr = stream;

	if(Parse( &ptr, "+CREG:" ) == 0){
		ptr += 9; // advance to status
		if(UTIL1_ScanDecimal8uNumber(&ptr, &MYresp.REGn) == ERR_OK){
			return 0;
		}
	}
	return 1;
}


static uint8_t findCLIP(const unsigned char *stream)
{
	const unsigned char *ptr = stream;

	if(Parse( &ptr, "+CLIP:" ) == 0){

		if(Parse( &ptr, sendtoN ) == 0){
			LEDinit(&MYrLED, 2);  // flash RED
			VIBinit(&MYvib, 1);  // vibrate
			return 0;
		}
	}
	return 1;
}


static uint8_t findCMGS(const unsigned char *stream)
{
	const unsigned char *ptr = stream;

	if(Parse( &ptr, "+CMGS" ) == 0){

		if(Parse( &ptr, "\r\n") == 0) return 0;
	}
	return 1;
}


static uint8_t findSTATUS(const unsigned char *stream, char *status)
{
	const unsigned char *ptr = stream;
	return Parse( &ptr, status );
}


static void PARSEmess(const unsigned char *stream, acttable *actionarray)
{
	const unsigned char *ptr = stream;
	byte i = 0;

	while(actionarray[i].cmdstring != NULL)
	{
		ptr = stream;  // always begin at start of stream
		if(Parse(&ptr, actionarray[i].cmdstring) == 0) actionarray[i].funcptr(actionarray[i].bit);
		i++;
	}
	//MYresp.waitfmess = 0;
}

void driveway(void)
{

}



uint8_t sendMessage(const unsigned char *number, char *mess, char *stream, resptype *resp)
{
	word timeout = 5000;

	resp->sent = 0;
	resp->REGn = 0;

	sendATstring("AT+CREG?\r"); // check registration
	while(timeout){
		WAIT1_Waitms(1);
		if(--timeout == 0) return ERR_NOTAVAIL;
		SerialService(resp);
		if(resp->REGn == 1) break;
	}

	sendATstring("AT+CMGS=\"");
	sendATstring(number);
	sendATstring("\"\r");

	timeout = 1000;
	while(timeout){
		WAIT1_Waitms(1);
		timeout--;
	}
	sendATstring(mess);
	sendATstring("\x1A"); // CTRL Z

	timeout = 5000;
	while(timeout){
		WAIT1_Waitms(1);
		timeout--;
		SerialService(resp);
		if(resp->sent) return ERR_OK;
	}
	return ERR_FAILED;
}














