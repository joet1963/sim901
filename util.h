/*
 * util.h
 *
 *  Created on: Jan 13, 2017
 *      Author: Magu
 */

#ifndef SOURCES_UTIL_H_
#define SOURCES_UTIL_H_

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "atc.h"
#include "Events.h"



typedef struct {

	byte RLY1;
	byte RLY2;
	byte Q1;
	byte Q2;
	byte Q3;
	byte Heat;
	byte Pump;
	byte Pumpa;
	byte Tank;
	byte Tanka;
	byte Prime;
	byte Primea;
	byte Feat5;
	int8_t setPoint;
	int8_t pumpPoint;
	byte INPUT1;
	byte INPUT2;
	byte INPUT3;
	byte INPUT4;
	byte INPUT5;
	byte status;
	byte config;
	byte dailyrpt;
	uint32_t Heattime;
	uint32_t Pumptime;
	uint32_t Pumpatime;
	uint32_t Tanktime;
	uint32_t Tankatime;
	uint32_t Primetime;
	uint32_t Primeatime;
	word messcnt;

}IOsema;


typedef struct  {

	bool active;
	word onTime;
	word offTime;
	word Timer;
	byte reps;
	byte onState;

}VIBtype;

typedef struct {

	bool state;
	bool plong;
	bool pshort;
	word Timer;

}SWtype;



bool VERop(void);
uint8_t VERreg(void);
void IOinit(IOsema *io);
void DriveIn(IOsema *io);
void DriveOut(IOsema *io);
uint8_t ScanDate( const unsigned char **str, uint8_t *day, uint8_t *month, uint8_t *year);
void supervisor(IOsema *io, IOsema *mem, char *rxbuff);
void sendSTATUS(IOsema *io, char *rxbuff);
void sendCONFIG(IOsema *io, char *rxbuff);
void Feature(IOsema *io);
void pswitch(SWtype *sw);
void VIBinit(VIBtype *vib, byte mode);
void VIBdeinit(VIBtype *vib);
void VIBcont(VIBtype *vib);
void LEDcont(VIBtype *vib);
void LEDinit(VIBtype *vib, byte mode);
void LEDdeinit(VIBtype *vib);
bool PWRseq(void);




#endif /* SOURCES_UTIL_H_ */
