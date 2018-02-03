// ConfigLib.h

#ifndef _CONFIGLIB_h
#define _CONFIGLIB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

/*****************************************************************************

Configurator - Allows sketch config to be stored in EEPROM and optionally modified 
and saved at runtime as part of sketch startup.

It's simple to hold your sketch config in code and compile it in but then how do you change 
it when the device is in-situ or what about if you have multiple devices? Do you modify the 
sketch for each device and recompile/reburn? 

You really want to be able to at runtime see the config and modify it before the main
part of the sketch runs and this class allows you to do that.

Simply provides implementations of the three callback functions, create an instance
and call initConfig and the class will try and load the config from EEPROM, display
the config and prompt the user to trigger the config process.
If triggered they can then display the config, modify it, save it to EEPROM etc.
Once complete they can exit and the sketch will continue loading. 

*****************************************************************************/

#define EPROM_TAG_SIZE 4

class Configurator 
{
    public:
        /* 
           Constructor
           params:
             stream: Stream to display onto and consume input from
             configSelectPeriod: time in msecs to wait for user to initiate config process
             logBufferSize: number of chars to reserve to use in logging to the stream
        */
        Configurator(Stream* stream, int configSelectPeriod, int logBufferSize);
        
        
        /*
            initConfig
            Call to trigger the config process. 
            Any config stored in flash will be read into the config parameter.
            The user may then, within a default period, trigger the config process allowing them to modify the config and store to flash.
            params:
                configTag: tag assigned to the block used to hold the config in flash
                config: pointer to the config structure which will be loaded / modified
                configLen: length of config structure
                printConfigItemHelp: pointer to callback function which displays help on what can be configured
                printConfig: pointer to callback function which displays the config.
                setConfigItem: pointer to callback function which is called in response to user trying to set a config item
        */    
        void initConfig(const char* configTag,
                    unsigned char* config,
                    int configLen,
                    void(*printConfigItemHelp)(),
                    void(*printConfig)(),
                    void(*setConfigItem)(const char*, const char*));    
                                        
    protected:
    
        void runConfigUI(const char* configTag,
                        unsigned char* config,
                        int configLen,
                        void(*printConfigItemHelp)(),
                        void(*printConfig)(),
                        void(*setConfigItem)(const char*, const char*));
        
    	Stream* m_stream = NULL;
        int m_logBufferSize;
        int m_configSelectPeriod;
    
        void log(const __FlashStringHelper * fsh, ...);
        void logToStream(const char* msg);
        
        void dumpBlocksToConsole(int startPos);
        void printConfigCommandHelp(void(*printConfigItemHelp)());

        void sprintf_vargs(char* buffer, int bufferlen, char * format, ...);
        String getField(String* msg, char fieldSep) ;
        String getFieldOfSize(String* msg, int numChars);
        int readLineFromSerial(int readch, char*buffer, int bufferLen);
        char* find_first_non_white_space(const char *line);

        boolean atBlockStart(int location);
        boolean checkBlockTagMatches(int location, const char* tag) ;
        int locateBlock(const char* tag, int startPos);
        int writeBytesToEEPROM(int location, const unsigned char* buffer, int bufferLen, unsigned char* crc);
        int writeByteToEEPROM(int location, int numBytes, char byte);
        int writeBlockToEEPROM(const char* tag, const unsigned char* buffer, int bufferLen, int& blockStartPos, int& blockLen);
        int readBytesFromEEPROM(int location, int numBytes, unsigned char* buffer, unsigned char* crc);
        int readBlockAtPosFromEEPROM(int blockLocation, unsigned char* buffer, int bufferLen, int& bytesRead, int& blockLen, char* tag);
        int readBlockFromEEPROM(const char* tag, unsigned char* buffer, int bufferLen, int& bytesRead, int& blockStartPos, int& blockLen);
        void writeConfigToEEPROM(const char* tag, const unsigned char* config, int configLen, int _blockStartPos);
        void loadConfigFromEEPROM(const char* tag, unsigned char* config, int configLen);
        void dumpBytesFromEEPROMToConsole(int location, int numBytes);
        
        void crc8(unsigned char *crc, unsigned char m);
        void crc8_buffer(unsigned char *crc, const unsigned char *buffer, int bufferLen);

};
                
#endif

