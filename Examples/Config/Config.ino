
#include "types.h"
#include <ConfigLib.h>
#include <EEPROM.h>


// Config structure which will be read from EEPROM
// with some default values in case we can't read any config from EEPROM
Config config = { 100, 199, "AAA" };

// Unique tag use to locate config in EEPROM
#define CONFIG_TAG "ESWC"

// Callback function which prints out information on the config items and values
void printConfigItemHelp(Configurator* configurator) 
{
	configurator->log(F("RFM_NODE_ID       RFM_NODE_ID     int"));
	configurator->log(F("RFM_NETWORK_ID    RFM_NETWORK_ID  int"));
	configurator->log(F("NODE_ID           NODE_ID         char[%d]"), sizeof(config.nodeId));
}

// Callback function which prints out the config
void printConfig(Configurator* configurator) 
{
	configurator->log(F("Current config"));
	configurator->log(F("  RFM_NODE_ID     = [%d]"), config.rfmNodeId);
	configurator->log(F("  RFM_NETWORK_ID  = [%d]"), config.rfmNetworkId);
	configurator->log(F("  NODE_ID         = [%s]"), config.nodeId);
}


// Callback function which sets the config item key to the value val
void setConfigItem(Configurator* configurator, const char* key, const char* val) 
{
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
		configurator->log(F("Unknown key type"));
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

