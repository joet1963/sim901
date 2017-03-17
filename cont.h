/*
 * cont.h
 *
 *  Created on: Feb 17, 2017
 *      Author: Magu
 */

#ifndef SOURCES_CONT_H_
#define SOURCES_CONT_H_

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "atc.h"
#include "util.h"
#include "Events.h"





typedef const struct {

	char *cmdstring;
	void(*funcptr)(byte b);
	byte bit;

}acttable;


#endif /* SOURCES_CONT_H_ */
