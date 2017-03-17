/*
 * cont.c
 *
 *  Created on: Feb 17, 2017
 *      Author: Magu
 */


#include "cont.h"
#include "UTIL1.h"

extern AS1_TComData rxbuff[];
extern IOsema MYio;

void RLY1(byte b)
{
	MYio.RLY1 = b;
}

void RLY2(byte b)
{
	MYio.RLY2 = b;
}

void Q1(byte b)
{
	MYio.Q1 = b;
}

void Q2(byte b)
{
	MYio.Q2 = b;
}

void Q3(byte b)
{
	MYio.Q3 = b;
}

void StatusUp(byte b)
{
	MYio.status = b;
}

void ConfigUp(byte b)
{
	MYio.config = b;
}

void DLY(byte b)
{
	MYio.dailyrpt = b;
}

void HEATinit(byte b)
{
	const unsigned char *ptr = rxbuff;
	MYio.Heat = b;
	ptr += 6;      // advance pointer to where data will be
	UTIL1_ScanDecimal8sNumber(&ptr, &MYio.setPoint);
}

void HEATdeinit(byte b)
{
	MYio.Heat = b;
	MYio.RLY1 = 0;
}


void PUMPinit(byte b)
{
	const unsigned char *ptr = rxbuff;
	MYio.Pump = b;
	ptr += 6;      // advance pointer to where data will be
	UTIL1_ScanDecimal8sNumber(&ptr, &MYio.pumpPoint);
}

void PUMPdeinit(byte b)
{
	MYio.Pump = b;
	MYio.Q1 = 0;
}

void TANKinit(byte b)
{
	MYio.Tank = b;
}

void TANKdeinit(byte b)
{
	MYio.Tank = 0;
	MYio.Q2 = 0;
}

void PRIMEinit(byte b)
{
	MYio.Prime = b;
}

void PRIMEdeinit(byte b)
{
	MYio.Prime = b;
	MYio.Q1 = 0;
}

void BYPASS(byte b)
{
	if(b)
	{
		MYio.Heat = 0;
		MYio.RLY1 = 1;
		MYio.Pump = 0;
		MYio.Q1 = 1;
		MYio.Tank = 0;
		MYio.Q2 = 1;
		MYio.Prime = 0;
	}
	else
	{
		MYio.RLY1 = 0;
		MYio.Q1 = 0;
		MYio.Q2 = 0;
	}
}

void SysReset(byte b)
{
	Cpu_SystemReset();
}

const acttable actionarray[] = {

		"Relay1on", RLY1, 1,
		"Relay1off", RLY1, 0,
		"Relay2on", RLY2, 1,
		"Relay2off", RLY2, 0,
		"Trans1on", Q1, 1,
		"Trans1off", Q1, 0,
		"Trans2on", Q2, 1,
		"Trans2off", Q2, 0,
		"Trans3on", Q3, 1,
		"Trans3off", Q3, 0,
		"Status?", StatusUp, 1,
		"Config?", ConfigUp, 1,
		"Reset", SysReset,1,
		"Dailyon", DLY, 1,
		"Dailyoff", DLY, 0,
		"Heaton", HEATinit, 1,
		"Heatoff", HEATdeinit, 0,
		"Pumpon", PUMPinit, 1,
		"Pumpoff", PUMPdeinit, 0,
		"Tankon", TANKinit, 1,
		"Tankoff", TANKdeinit, 0,
		"Primeon", PRIMEinit, 1,
		"Primeoff", PRIMEdeinit, 0,
		"Bypasson", BYPASS, 1,
		"Bypassoff", BYPASS, 0,
		NULL, ((void*)0), 0    // sentinel

};






