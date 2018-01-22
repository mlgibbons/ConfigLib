// ConfigLib.h

#ifndef _CONFIGLIB_h
#define _CONFIGLIB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

// *****************************************************************************
// Config - functions to allow sketch config to be stored in EPROM
// *****************************************************************************

#define EPROM_TAG_SIZE 4

class Configurator 
{
    public:
        Configurator(Stream* stream, int bufferSize);
        
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
        int m_bufferSize;
    
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

