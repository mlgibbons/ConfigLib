
#include "ConfigLib.h"

// Arduino libraries referenced by this library - need to include these in your sketch too 
#include <EEPROM.h>

#include <stdio.h>
#include <stdarg.h>


//#!*******************************************************************************************
Configurator::Configurator(Stream* stream, int configSelectPeriod, int logBufferSize=128) 
{ 
    m_stream = stream; 
    m_logBufferSize = logBufferSize;
    m_configSelectPeriod = configSelectPeriod;
}
                
//#!*******************************************************************************************
void Configurator::sprintf_vargs (char* buffer, int bufferlen, char * format, ...)
{
    va_list args;
    va_start (args, format);
    vsnprintf (buffer, bufferlen-1, format, args);
    va_end (args);
}

//#!*******************************************************************************************
String Configurator::getField(String* msg, char fieldSep) 
{
    String result = "";
    int delimPos = msg->indexOf(fieldSep);
      
    // if no delim found then scan to end of the string 
    if (delimPos != -1) {
        result = msg->substring(0, delimPos);
        *msg = msg->substring(delimPos+1);
    } else {
        result = msg->substring(0, msg->length());
        *msg = "";
    }
      
    return result;
}  

//#!*******************************************************************************************
String Configurator::getFieldOfSize(String* msg, int numChars)
{
    String result = "";

    result = msg->substring(0, numChars);
    
    *msg = msg->substring(numChars);
    
    return result;     
}

//#!*******************************************************************************************
int Configurator::readLineFromSerial(int readch, char*buffer, int bufferLen)
{
    static int pos = 0;
    int rpos;

    if (readch > 0) {
        switch (readch) {
        case '\n': // Ignore new-lines
            break;
        case '\r': // Return on CR
            rpos = pos;
            pos = 0;  // Reset position index ready for next time
            return rpos;
        default:
            if (pos < bufferLen - 1) {
                buffer[pos++] = readch;
                buffer[pos] = 0;
            }
        }
    }
    // No end of line has been found, so return -1.
    return -1;
}


//#!********************************************************************************************
// 
// Config
//
//	EPROM format is a collection of blocks starting at EPROM_CONFIG_START
//  
//  Each block starts with a magic string "MGGG"
//  Next comes a block tag of 4 characters e.g. "MBT1"
//  Then a byte for the block length
//  Then the actual data 
//  Then a checksum
//
// *********************************************************************************************

#define EPROM_BLOCK_START_MAGIC_STRING "MGGG"
#define EPROM_BLOCK_START_MAGIC_STRING_LEN 4
#define EPROM_CONFIG_START 0
#define EPROM_CONFIG_END   1024

//#!*******************************************************************************************
boolean Configurator::atBlockStart(int location) {
	boolean rc = false;

	if (EEPROM.read(location + 0) == EPROM_BLOCK_START_MAGIC_STRING[0] &&
		EEPROM.read(location + 1) == EPROM_BLOCK_START_MAGIC_STRING[1] &&
		EEPROM.read(location + 2) == EPROM_BLOCK_START_MAGIC_STRING[2] &&
		EEPROM.read(location + 3) == EPROM_BLOCK_START_MAGIC_STRING[3]) 
	{
		rc = true;
	}

	return rc;
}

//#!*******************************************************************************************
boolean Configurator::checkBlockTagMatches(int location, const char* tag) 
{
	boolean rc = false;

	if (EEPROM.read(location + 0) == tag[0] &&
		EEPROM.read(location + 1) == tag[1] &&
		EEPROM.read(location + 2) == tag[2] &&
		EEPROM.read(location + 3) == tag[3])
	{
		rc = true;
	}

	return rc;
}

//#!*******************************************************************************************
int Configurator::locateBlock(const char* tag, int startPos=0)
{
	int rc;
	boolean blockFound = false;
	int blockLocation = 0;

	int currLocation = startPos;

	while ((blockFound!=true) && (currLocation<EPROM_CONFIG_END)) {		
		
		// are we at the start of a block
		if (atBlockStart(currLocation) == true) {
			
			// check block tag matches if one was passed
			if (tag!=NULL) {
				int tagPos = currLocation + EPROM_BLOCK_START_MAGIC_STRING_LEN;		
				if (checkBlockTagMatches(tagPos, tag) == true) {
					blockFound = true;
				}
			}
			else {
				blockFound = true;
			}

			if (blockFound==true) {
				blockLocation = currLocation;
				break;
			}
		}
		
		currLocation++;		
	}	

	rc = blockFound ? blockLocation: -1;

	return rc;
}

//#!*******************************************************************************************
int Configurator::writeBytesToEEPROM(int location, const unsigned char* buffer, int bufferLen, unsigned char* crc)
{
	for (unsigned int t = 0; t<bufferLen; t++) {
		EEPROM.write(location + t, buffer[t]);
	}

	if (crc!=NULL) {
		crc8_buffer(crc, buffer, bufferLen);
	}
	
	return location + bufferLen;
}

//#!*******************************************************************************************
int Configurator::writeByteToEEPROM(int location, int numBytes, char byte)
{
	for (unsigned int t = 0; t<numBytes; t++) {
		EEPROM.write(location + t, byte);
	}
}

//#!*******************************************************************************************
int Configurator::writeBlockToEEPROM(const char* tag, const unsigned char* buffer, int bufferLen, int& blockStartPos, int& blockLen)
{
	if (strlen(tag) < EPROM_TAG_SIZE) {
		log(F("ERROR - Write aborted: tag size incorrect"));
		return -1;
	}

	int _blockStart = 0;

	// find the block if writePos was not set
	if (blockStartPos==-1)  {
		_blockStart = locateBlock(tag);

		if (_blockStart < 0) {
			log(F("ERROR - Block not found"));
			return -1;
		}
		else {
		    String logMsg = String(F("Block found at ["));
			logMsg += _blockStart;
			logMsg += "]";
			log(F("%s"), logMsg.c_str());
		}
	}
	else {
		_blockStart = blockStartPos;
		String logMsg = String(F("Writing block at ["));
		logMsg += _blockStart;
		logMsg += "]";
	    log(F("%s"), logMsg.c_str());
	}

	int currWritePos = _blockStart;

	// now write out the block
	unsigned char crc = 0;

	// magic string
	currWritePos = writeBytesToEEPROM(currWritePos, (unsigned  char *) EPROM_BLOCK_START_MAGIC_STRING, strlen(EPROM_BLOCK_START_MAGIC_STRING), NULL);
	
	// tag
	currWritePos = writeBytesToEEPROM(currWritePos, (unsigned char*) tag, EPROM_TAG_SIZE, &crc);
	
	// data len
	unsigned char blockDataLenChar = (char) bufferLen;
	currWritePos = writeBytesToEEPROM(currWritePos, &blockDataLenChar, 1, &crc);

	// data
	currWritePos = writeBytesToEEPROM(currWritePos, buffer, bufferLen, &crc);

	// checksum
	unsigned char blockChecksumChar = (unsigned char) crc;
	currWritePos = writeBytesToEEPROM(currWritePos, &blockChecksumChar, 1, NULL);

	blockStartPos = _blockStart;
	blockLen = currWritePos - _blockStart;

	return 0;
}

//#!*******************************************************************************************
int Configurator::readBytesFromEEPROM(int location, int numBytes, unsigned char* buffer, unsigned char* crc)
{
	for (unsigned int t = 0; t<numBytes; t++) {
		*(buffer + t) = EEPROM.read(location + t);
		if (crc != NULL) {
			crc8(crc, *((char*)buffer + t));
		}
	}

	return location + numBytes;
}

//#!*******************************************************************************************
int Configurator::readBlockAtPosFromEEPROM(int blockLocation, unsigned char* buffer, int bufferLen, int& bytesRead, int& blockLen, char* tag=NULL)
{
	int currReadPos = blockLocation;

    // read in the block

	// magic string
	char blockStartStr[EPROM_BLOCK_START_MAGIC_STRING_LEN];

	currReadPos = readBytesFromEEPROM(currReadPos, EPROM_BLOCK_START_MAGIC_STRING_LEN, (unsigned char*)&blockStartStr[0], NULL);

    unsigned char crc = 0;

	// tag
	char _tag[EPROM_TAG_SIZE];
	currReadPos = readBytesFromEEPROM(currReadPos, EPROM_TAG_SIZE, (unsigned char*) &_tag[0], &crc) ;

	if (tag != NULL) {
		memcpy(tag, _tag, 4);
	}

	// data len
	unsigned char blockDataLenChar = (unsigned char) bufferLen;
	currReadPos = readBytesFromEEPROM(currReadPos, 1, (unsigned char*)&blockDataLenChar, &crc);
	int blockDataLen = (int) blockDataLenChar;

	// data
	currReadPos = readBytesFromEEPROM(currReadPos, blockDataLen, buffer, &crc);

	// checksum
	unsigned char blockChecksumChar;
	currReadPos = readBytesFromEEPROM(currReadPos, 1, (unsigned char*) &blockChecksumChar, NULL);

	if (blockChecksumChar != crc) {
		log(F("ERROR - Block read errro: checksum mismatch"));
		return -1;
	}

	blockLen = currReadPos - blockLocation;
	bytesRead = blockDataLen;

	return 0;
}

//#!*******************************************************************************************
int Configurator::readBlockFromEEPROM(const char* tag, unsigned char* buffer, int bufferLen, int& bytesRead, int& blockStartPos, int& blockLen)
{
	// find the block
	blockStartPos = locateBlock(tag);
	if (blockStartPos<0)  return -1;

	return readBlockAtPosFromEEPROM(blockStartPos, buffer, bufferLen, bytesRead, blockLen);
}

//#!*******************************************************************************************
void Configurator::dumpBytesFromEEPROMToConsole(int location, int numBytes)
{
	for (unsigned int t = 0; t<numBytes; t++) {
		int curr = (int) EEPROM.read(location+t);
		String currStr = String( curr, HEX);
		log(F("%s"), currStr.c_str());
	}

}

//#!*******************************************************************************************
void Configurator::dumpBlocksToConsole(int startPos)
{
	int currPos = startPos;
	while (currPos<EPROM_CONFIG_END) {
		int blockFoundPos = locateBlock(NULL, currPos);

		if (blockFoundPos>=0) {
			unsigned char buffer[64];
			int blockLen;
			char tag[4];
			int numBytesRead;
			int rc = readBlockAtPosFromEEPROM(blockFoundPos, (unsigned char*)&buffer, 64, numBytesRead, blockLen, &tag[0]);

			String logMsg = "Block with tag [";
			for (int i=0; i<4;i++) {
				logMsg += tag[i];
			}
			
			logMsg += "] found at location [";
			logMsg += blockFoundPos;

			logMsg += "] length [";
			logMsg += blockLen;

			logMsg += "] with contents [";
			for (int i = 0; i<numBytesRead;i++)
				logMsg += String(buffer[i], HEX);
			logMsg += "]";

			Serial.println(logMsg);

			currPos+=blockLen+1;
		} else {
			currPos++;
		}
	}
}

//#!*******************************************************************************************
void Configurator::printConfigCommandHelp(void(*printConfigItemHelp)()) 
{
	log(F("Commands are"));
	log(F("S:K,V = Set item K to value V"));
	log(F("---------------------------------------------"));

    if (printConfigItemHelp != NULL) {
        log(F("Item Id : Item Name : Value"));
        printConfigItemHelp();
    }

	log(F("---------------------------------------------"));

	log(F("P       = Print config"));
	log(F("W:P     = Write config to EEPROM Optional (P=Pos) "));
	log(F("R       = Read config from EEPROM"));
	log(F("H       = Print this help text"));
	log(F("E       = Erase all config in EEPROM"));
	log(F("C       = Dump all config blocks to console"));
	log(F("D:P,N   = Dump N bytes from EEPROM at pos P to console"));
	log(F("Q       = Quit"));

	log(F("---------------------------------------------"));
}

//#!*******************************************************************************************
void Configurator::writeConfigToEEPROM(const char* tag, const unsigned char* config, int configLen, int _blockStartPos = -1) {
	log(F("Writing config to EEPROM"));

	int blockStartPos = _blockStartPos;
	int blockLen;
	int rc = writeBlockToEEPROM(tag, (const unsigned char*) config, configLen, blockStartPos, blockLen);

	if (rc<0) {
		log(F("Failed to write config to EEPROM"));
	}
	else {
		log(F("Successfully wrote config to EEPROM"));
	}
}

//#!*******************************************************************************************
void Configurator::loadConfigFromEEPROM(const char* tag, unsigned char* config, int configLen) {
	log(F("Reading config from EEPROM."));

	int numBytesRead, blockStartPos, blockLen;

	if (readBlockFromEEPROM(tag, config, configLen, numBytesRead, blockStartPos, blockLen) == 0) {
		log(F("Successfully read config from EEPROM."));
	}
	else {
		log(F("Failed to read config from EEPROM. Using default config."));
	};
}

//#!*******************************************************************************************
void Configurator::runConfigUI(const char* configTag,
	unsigned char* config,
	int configLen,
	void(*printConfigItemHelp)(),
	void(*printConfig)(),
	void(*setConfigItem)(const char*, const char*))
{
	char lineBuffer[32];

	// allow user to modify config
	log(F("Config mode entered"));
	printConfigCommandHelp(printConfigItemHelp);

	while (1 == 1) {
		if (readLineFromSerial(Serial.read(), lineBuffer, sizeof(lineBuffer)) > 0) {

            // log(F("Line = [%s]"), lineBuffer);

			// ** HELP ******************************************************	
			if (lineBuffer[0] == 'H') {
				printConfigCommandHelp(printConfigItemHelp);
				strcpy(lineBuffer, "");
			}

			// ** QUIT **************************************************************	
			else if (strcmp(lineBuffer,"Q")==0) {
				log(F("Exiting interactive config mode"));
                strcpy(lineBuffer, "");
                break;
			}

			// ** BLOCKS **************************************************************	
			else if (strcmp(lineBuffer,"C")==0) {
				log(F("Dumping config blocks"));
				dumpBlocksToConsole(0);
				log(F("Done"));
                strcpy(lineBuffer, "");
            }

			// ** DUMP **************************************************************	
			else if (lineBuffer[0] == 'D') {
				log(F("Dumping EEPROM contents"));
                char* cmd = strtok(lineBuffer, ":");
                char* posStr = strtok(NULL, ",");
                char* numStr = strtok(NULL, ",");
                int pos = atoi(posStr);
                int num = atoi(numStr);
				dumpBytesFromEEPROMToConsole(pos, num);
				log(F("Done"));
                strcpy(lineBuffer, "");
            }

			// ** ERASE **************************************************************	
			else if (strcmp(lineBuffer,"E") == 0) {
				log(F("Erasing all config"));
				writeByteToEEPROM(0, EPROM_CONFIG_END, 'X');
				log(F("Done"));
                strcpy(lineBuffer, "");
            }

			// ** PRINT *************************************************************	
			else if (strcmp(lineBuffer,"P") == 0) {
				printConfig();
                strcpy(lineBuffer, "");
            }

			// ** WRITE *************************************************************	
			else if (lineBuffer[0] == 'W') {
				char *posStr = strtok(lineBuffer, ",");

				if (strcmp(posStr,"")==0) {
					writeConfigToEEPROM(configTag, config, configLen, -1);
				}
				else {
                    int pos;
                    pos = atoi(posStr);
					writeConfigToEEPROM(configTag, config, configLen, pos);
				}
                strcpy(lineBuffer, "");
            }

			// ** READ *************************************************************	
			else if (lineBuffer[0] == 'R') {
				loadConfigFromEEPROM(configTag, config, configLen);
                strcpy(lineBuffer, "");
            }

			// ** SET ***************************************************************	
			else if (lineBuffer[0] == 'S') {
                char* cmd = strtok(lineBuffer, ":");
                char* key = strtok(NULL, ",");
                char* val = strtok(NULL, ",");

				log(F("Setting item [%s] to [%s]"), key, val);

				setConfigItem(key, val);

                strcpy(lineBuffer, "");
            }

			else {
				log(F("Unknown command [%s]"), lineBuffer);
                strcpy(lineBuffer, "");
            }
		}
	}

}

//#!*******************************************************************************************
void Configurator::initConfig(  const char* configTag,
                                unsigned char* config,
                                int configLen,
                                void(*printConfigItemHelp)(),
                                void(*printConfig)(),
                                void(*setConfigItem)(const char*, const char*))
{
	int  initCycleDelay = 500;   // cycle time in ms
	long initPeriodCountMax = m_configSelectPeriod / initCycleDelay;
	long initPeriodCountCur = 0;

	char lineBuffer[32];

	log(F("Starting up in [%d] ms"), m_configSelectPeriod);
	log(F("Using config"));

	loadConfigFromEEPROM(configTag, config, configLen);
	printConfig();

	log(F("Press 'C' and 'Enter' to enter config mode or 'Q' to continue immediately"));

	int configModeSelected = 0;

	while ((initPeriodCountCur < initPeriodCountMax) && (configModeSelected == 0)) {

		initPeriodCountCur++;
		log(F("."));
		delay(initCycleDelay);

        if (readLineFromSerial(Serial.read(), lineBuffer, sizeof(lineBuffer)) > 0) {
			if (strcmp(lineBuffer,"C")==0) {
				configModeSelected = 1;
				strcpy(lineBuffer, "");
			}
			else if (strcmp(lineBuffer,"Q")==0) {
				break;
			}
		}
	}

	if (configModeSelected == 1) {
		log(F("Entering manual config mode"));
		runConfigUI(configTag, config, configLen, printConfigItemHelp, printConfig, setConfigItem);
	}

	log(F("Continuing startup"));
}


//#!*******************************************************************************************
void Configurator::logToStream(const char * fsh)
{
    if ( (m_stream==NULL) ) { return; }
    m_stream->println(fsh);
	m_stream->flush();
}

//#!*******************************************************************************************
void Configurator::log(const __FlashStringHelper * fmt, ...) 
{
    char logMsgBuffer[m_logBufferSize];
    va_list args;
    va_start(args, fmt);
#ifdef __AVR__
    vsnprintf_P(logMsgBuffer, sizeof(logMsgBuffer), (const char *)fmt, args); // progmem for AVR
#else
    vsnprintf(logMsgBuffer, sizeof(logMsgBuffer), (const char *)fmt, args); // for the rest of the world
#endif
    va_end(args);
    logToStream(logMsgBuffer);    
}


//#!*******************************************************************************************
char* Configurator::find_first_non_white_space(const char *line)
{
    char * rc = (char *) line;
    while (isspace((unsigned char)*rc))
        rc++;

    return rc;
}

//#!**********************************************************************************
// Computes a 8-bit CRC
// Dummy implementation - replace as needed but take care with memory usage
// ***********************************************************************************
void Configurator::crc8(unsigned char *crc, unsigned char m)
{
    *crc = 'X';
}

void Configurator::crc8_buffer(unsigned char *crc, const unsigned char *buffer, int bufferLen)
{
    *crc = 'X';
}

