
## Summary
A simple library which allows for the storing, reading and modification by end users of sketch config at runtime.

## Why Is This Needed?
How often have you written an Arduino sketch and hard coded the config into it?
Maybe you put into EEPROM so that you could keep it out of the sketch but then you had run 
burn an EEPROM sketch to change the config and the reburn your actual sketch.
Maybe you wrote some config routines but they were specific to the sketch that you put them in.
Also, how did you know what the config was then your sketch ran? How could you see it?

This small library aims to help ease the pain of configuring Arduino sketches by storing the config 
in EEPROM and providing a means by which the user can view, change and store the config before the 
sketch runs. It also allows the user to temporarily change the config for just this "session" by
modifying it but not saving it to EEPROM.

The library requires four things to be able to do its magic:
 - a pointer to the config and a tag for it
 - three callback functions that interface to the config (as the library knows nothing about what is in the config)
   - printConfig
   - printConfigItemHelp
   - setConfigItem

## The Process
At sketch startup the library will try and find the config in EEPROM and if found will 
load it into the memory pointed to by the passed in config pointer. If none is found then 
it will do nothing and the defaults values which the config already contains will be used.

The config will then be printed out to the stream and the user will be prompted to enter config mode.
They have N seconds to do so by entering "C" after which, if not selected, the sketch will simply carry on using the current config.
Alternatively they can enter "Q" to quit the config startup period immediately.

However, if they do select config mode they will be presented with a menu which allows them to interact with the config.
They can do things like display the config, modify an item in it and write it to EEPROM.

Once they are done they quit the config process through entering "Q" and the sketch continues.
 
## An Example Usage

This is an example of how the configurator lib works

```
Setup start
Starting up in [10000] ms
Using config
Reading config from EEPROM.
Failed to read config from EEPROM. Using default config.
Current config
  RFM_NODE_ID     = [100]
  RFM_NETWORK_ID  = [199]
  NODE_ID         = [AAA]
Press 'C' and 'Enter' to enter config mode or 'Q' to continue immediately
.
.
.

C\n   <<<<<<<<<<<<<<<<<<<<<<<< Enter config mode
Entering manual config mode
Config mode entered
Commands are
S:K,V = Set item K to value V
---------------------------------------------
Item Id : Item Name : Value
RFM_NODE_ID       RFM_NODE_ID     int
RFM_NETWORK_ID    RFM_NETWORK_ID  int
NODE_ID           NODE_ID         char[4]
---------------------------------------------
P       = Print config
W:P     = Write config to EEPROM Optional (P=Pos) 
R       = Read config from EEPROM
H       = Print this help text
E       = Erase all config in EEPROM
C       = Dump all config blocks to console
D:P,N   = Dump N bytes from EEPROM at pos P to console
Q       = Quit
---------------------------------------------

P\n      <<<<<<<<<<<<<<<<<<<<<<<< Print out config
Current config
  RFM_NODE_ID     = [100]
  RFM_NETWORK_ID  = [199]
  NODE_ID         = [AAA]

S:RFM_NODE_ID,122      <<<<<<<<<<<<<<<<<<<<<<<< Change a config item
Setting item [RFM_NODE_ID] to [122]

W\n   <<<<<<<<<<<<<<<<<<<<<<<< Store new config to EEPROM
Writing config to EEPROM
Writing block at [0]
Successfully wrote config to EEPROM

R\n   <<<<<<<<<<<<<<<<<<<<<<<< Read config from EEPROM
Reading config from EEPROM.
Successfully read config from EEPROM.
Current config
  RFM_NODE_ID     = [122]
  RFM_NETWORK_ID  = [199]
  NODE_ID         = [AAA]

Q\n   <<<<<<<<<<<<<<<<<<<<<<<< Quit config mode
Exiting interactive config mode
Continuing startup
Setup complete
Looping
```
