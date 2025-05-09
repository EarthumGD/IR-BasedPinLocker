==============  DESCRIPTION  ==============
This code contains the main file and header files for the IR-Based Pin Locker.

==============  FILES  ==============
IRRemotePassword.ino - Main file
DefinePins.h - Defines the pin input/outputs 
HexCodeDecoder.h - Decodes the IR signal to its corresponding IR Remote output  
Lock.h - Controls the servo motor (SG90) 
UltrasonicSensor.h - Contains the function to get the distance sensed 

==============  UPDATES  ==============
Update v1.1
- Resetting PIN code changed when you have access granted
- Added buzzer on equal with duration of 60 seconds when you entered PIN code three times unsuccessfully
- Original: Buzzer goes on when password is correct
  New: Buzzer goes on when password is incorrect
- Changed actual digits to asterisks (*) when inputting PIN code
