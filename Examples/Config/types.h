// types.h

#ifndef _TYPES_h
#define _TYPES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

struct Config {
	int rfmNodeId;
	int rfmNetworkId;
	char nodeId[4];
};


#endif

