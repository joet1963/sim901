/*
 * atc.h
 *
 *  Created on: Jan 4, 2017
 *      Author: Magu
 */

#ifndef SOURCES_ATC_H_
#define SOURCES_ATC_H_

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "Events.h"
#include "IO_Map.h"
#include "WAIT1.h"
#include "UTIL1.h"
#include "util.h"
#include "as1.h"
#include "TmDt1.h"
#include "cont.h"



typedef struct {

	bool waitfmess;
	uint8_t result;
	bool CLIP;
	bool sent;
	bool OK;
	bool ERR;
	bool RING;
	bool CRDY;
	bool FUNC;
	bool RDY;
	byte REGn;
	byte REGc;
	bool date;
	bool time;

}resptype;

uint8_t sendMessage(const unsigned char *number, char *mess, char *stream, resptype *resp);
void sendATstring(AS1_TComData *CH);
void driveway(void);
void driveout(void);
void initrxbuff(void);
void checkio(void);
void SerialService(resptype *resp);
//void testMessages(uint8_t n);

#endif /* SOURCES_ATC_H_ */
