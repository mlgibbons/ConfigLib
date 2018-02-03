
#include "types.h"
#include <ConfigLib.h>
#include <EEPROM.h>

//:TODO:Move utility function into Configurator class

// Utility function
void print(const __FlashStringHelper * fmt, ...)
{
    char msgBuffer[128];
    va_list args;
    va_start(args, fmt);
#ifdef __AVR__
    vsnprintf_P(msgBuffer, sizeof(msgBuffer), (const char *)fmt, args); // progmem for AVR
#else
    vsnprintf(msgBuffer, sizeof(msgBuffer), (const char *)fmt, args); // for the rest of the world
#endif
    va_end(args);
    Serial.println(msgBuffer);
}


// Config structure which will be read from EEPROM
// with some default values in case we can't read any config from EEPROM
Config config = { 100, 199, "AAA" };

// Unique tag use to locate config in EEPROM
#define CONFIG_TAG "ESWC"

// Callback function which prints out information on the config items and values
void printConfigItemHelp() {
	print(F("RFM_NODE_ID       RFM_NODE_ID     int"));
	print(F("RFM_NETWORK_ID    RFM_NETWORK_ID  int"));
	print(F("NODE_ID           NODE_ID         char[%d]"), sizeof(config.nodeId));
}

// Callback function which prints out the config
void printConfig() {
	print(F("Current config"));
	print(F("  RFM_NODE_ID     = [%d]"), config.rfmNodeId);
	print(F("  RFM_NETWORK_ID  = [%d]"), config.rfmNetworkId);
	print(F("  NODE_ID         = [%s]"), config.nodeId);
}


// Callback function which sets the config item key to the value val
void setConfigItem(const char* key, const char* val) {
	if (strcmp(key, "RFM_NODE_ID")==0) {
		config.rfmNodeId = atoi(val);
	}
	else if (strcmp(key, "RFM_NETWORK_ID")==0) {
		config.rfmNetworkId = atoi(val);
	}
	else if (strcmp(key, "NODE_ID") == 0) {
		int _len = strlcpy(config.nodeId, val, sizeof(config.nodeId));
	}
	else {
		print(F("Unknown key type"));
	}
}

void setup()
{
	Serial.begin(57600);
    Serial.print(String(F("Setup start\n")).c_str());

    // Create Configurator instance
    // Allows user 10s to trigger config process
    // Reserve 128 bytes for output buffer
    Configurator configurator(&Serial, 10000, 128);
    
    // Init the config 
    // Tries to read config from block in EEPROM which is tagged with CONFIG_TAG
    // Will then use the callback functions to display the config and allow the user to modify it and save it if triggered during the 10s wait
    configurator.initConfig(CONFIG_TAG, (unsigned char *) &config, sizeof(Config), printConfigItemHelp, printConfig, setConfigItem);
  
	Serial.print(String(F("Setup complete\n")).c_str());
}

void loop()
{
	// do your stuff here
	Serial.print("Looping\n");
	delay(10000);
}

