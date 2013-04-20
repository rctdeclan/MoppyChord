   __  ___                   
  /  |/  /__  ___  ___  __ __
 / /|_/ / _ \/ _ \/ _ \/ // /
/_/  /_/\___/ .__/ .__/\_, / 
           /_/  /_/   /___/  

by Sammy1Am. Initial modification by rctdeclan

Moppy is a M_usical Fl_oppy controller program built for the Arduino UNO.

This version of Moppy, centers all the code into the Arduino. Therefore you do not have to use the Java code.

The arduino code takes Serial MIDI ON and MIDI OFF messages, and turns them into audible notes.

The big difference between MoppyChord and the original Moppy, is that notes are allocated to floppy drives independly of MIDI channels. 

Therefore you can send 3 MIDI ON messages to form a chord, and 3 floppydrives will play the chord.

--DESCRIPTION OF PREPROCESSOR FLAGS--





--INSTALLATION--
The Arduino code requires the TimeOne library available here: http://www.arduino.cc/playground/Code/Timer1

Upload the included Arduino code to the Arduino of your choice.

--HARDWARE-- (original text)

I built Moppy using an Arduino UNO, though it should work just fine on most Arduinos.  The pins are connected in pairs to floppy drives as follows: Even pins (2,4,6...) are connected to each drive's STEP pin, the matching odd pins (3,5,7...) are connected to the each drive's DIRECTION control pin.  So the first floppy would be connected to pin 2 & 3, the second floppy to 4 & 5, and so on.

Some pinout information can be found here: http://pinouts.ru/Storage/InternalDisk_pinout.shtml

Make sure you ground the correct drive-select pin, or the drive won't respond to any input (just connect the drive-select pin on the floppy to the pin directly below it).  You can tell when you have the right drive selected, because the light on the front of the drive will come on.  

Also, it's VERY IMPORTANT that your Arduino is grounded with the drives, or the drives will not register the pulses correctly.  To do this, make sure that the GND pin on the Arduino is connected to the odd-numbered pin below the STEP pin on the floppy (i.e. if the STEP pin is 20, connect the Audnio's GND pin to Floppy-pin 19).  You might need to do this for the DIRECTION pin as well (I did it for both, but I don't know if it's required).


--CONFIGURATION / USE--

Simply connect the Arduino serial to the MIDI OUT of your MIDI device.
To use your computer as the MIDI device, I recommend using "Hairless MIDI to Serial bridge":
http://projectgus.github.io/hairless-midiserial/


--MIDI FILE / MIDI STREAM INFORMATION / GUIDELINES--

- Each drive can only play a single note at a time.
- The amount of notes that can be played is dependant on the amount of drives. Currently, the program is set up with a maximum of 8.
- The software will only attempt to play notes between C1 and B4.  Floppy drives don't seem to respond well to notes outside of this range (especially higher).
- Generally shorter notes tend to sound better, as longer notes are marred by the read-heads changing directions repeatedly.




Cross your fingers, and enjoy!

--HELP/CONTRIBUTIONS--
https://github.com/SammyIAm/Moppy
https://github.com/rctdeclan/MoppyChord