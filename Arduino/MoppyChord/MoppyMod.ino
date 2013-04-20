#include "../lib/TimerOne.cpp"
boolean firstRun = true; // Used for one-run-only stuffs;

//First pin being used for floppies, and the last pin.  Used for looping over all pins.
const byte FIRST_PIN = 2;
const byte PIN_MAX = 17;
#define RESOLUTION 40 //Microsecond resolution for notes
#define BAUD 38400

#define USE_ALL 1
#define DEBUG 0
#define ACTIVE_SENSE 0

/*An array of maximum track positions for each step-control pin.  Even pins
are used for control, so only even numbers need a value here.  3.5" Floppies have
80 tracks, 5.25" have 50.  These should be doubled, because each tick is now
half a position (use 158 and 98).
*/
byte MAX_POSITION[] = {
  0,0,158,0,158,0,158,0,158,0,158,0,158,0,158,0,158,0
};
  
//Array to track the current position of each floppy head.  (Only even indexes (i.e. 2,4,6...) are used)
byte currentPosition[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*Array to keep track of state of each pin.  Even indexes track the control-pins for toggle purposes.  Odd indexes
track direction-pins.  LOW = forward, HIGH=reverse
*/
uint8_t currentState[] = {
  0,0,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW
};
  
//Current period assigned to each pin.  0 = off.  Each period is of the length specified by the RESOLUTION
//variable above.  i.e. A period of 10 is (RESOLUTION x 10) microseconds long.
unsigned int currentPeriod[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

uint8_t currentKey[] = {
  0,0,0,0,0,0,0,0
};

//Current tick
unsigned int currentTick[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 
};

unsigned int microPeriods[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        382, 361, 341, 321, 303, 286, 270, 255, 241, 227, 215, 202, //C1 - B1
        191, 180, 170, 161, 152, 143, 135, 128, 120, 114, 107, 101, //C2 - B2
        96, 90, 85, 80, 76, 72, 68, 64, 60, 57, 54, 51, //C3 - B3
        48, 45, 43, 40, 38, 36, 34, 32, 30, 28, 27, 25, //C4 - B4
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void noteOn(uint8_t key)
{
	//get period
//#define RESOLUTION_2 RESOLUTION*2
	//uint8_t period = microPeriods[key] / (RESOLUTION_2);
	uint8_t period = microPeriods[key];
	uint8_t moppy; //0 - 8
	
#if USE_ALL
	//get free moppy based on finding currentKey zero.
	
	for(moppy = 0; moppy<9; moppy++)
	{
		if(moppy == 8) 
		{
			return;
		}
		if(currentKey[moppy] == 0) 
		{
			break;
		}
	}
	
#else
	moppy = 0;
#endif
	
	//set currentKey to key.
	currentKey[moppy] = key;
	
	
#if DEBUG
	Serial.print("NOTE ON MOPPY:");
	Serial.println(moppy,DEC);    
#endif

	//encode to output-specific array.
	moppy = (moppy + 1) * 2;
	
	//set currentPeriod to period.
	currentPeriod[moppy] = period;
	
	
}

void noteOff(uint8_t key)
{
	//get moppy based on finding key in currentKey
	uint8_t moppy;
#if USE_ALL
	for(moppy = 0; moppy < 9; moppy++)
	{
		if(moppy == 8)
		{
			return;
		}
		if(currentKey[moppy] == key)
		{
			break;
		}
	}
#else
	moppy = 0;
#endif
	
	//set currentKey to 0.
	currentKey[moppy] = 0;

#if DEBUG
	Serial.print("NOTE OFF MOPPY:");
	Serial.println(moppy,DEC);    
#endif
	
	//encode to output-specific array.
	moppy = (moppy + 1) * 2;
	
	//set currentPeriod to 0.
	currentPeriod[moppy] = 0;
	
}

void loop(){
  
  //The first loop, reset all the drives, and wait 2 seconds...
  if (firstRun)
  {
    firstRun = false;
    resetAll();
    delay(2000);
  }

  //Only read if we have 
  if (Serial.available() > 2)
  {
#if ACTIVE_SENSE
	if (Serial.peek() == 254) return; //AS message
#endif
    //Watch for special 255-message to reset the drives
    if (Serial.peek() == 255) {
      resetAll();
      //Flush any remaining messages.
      while(Serial.available() > 0){
        Serial.read();
      }
    } 
    else{
		//get command nibble, ignore channel nibble
		uint8_t command = Serial.read() >> 4;
		
		//get key byte (0-255) 
		uint8_t key = Serial.read();
		
		//get velocity
		uint8_t velocity = Serial.read();
		
#if DEBUG
    Serial.print(command);
    Serial.print("/");
    Serial.print(key);
    Serial.print("/");
    Serial.println(velocity);
#endif
		
		//first make sure it's either note on or note off.
		//NOTE OFF:
		if(command == 8 || (command == 9 && velocity == 0)) //last is volume. note off if volume=0.
		{
			noteOff(key);
		}
		//NOTE ON:
		else if (command == 9)
		{
			noteOn(key);
		}
    }
  }
}


/*
Called by the timer inturrupt at the specified resolution.
*/
void tick()
{
  /* 
  If there is a period set for control pin 2, count the number of
  ticks that pass, and toggle the pin if the current period is reached.
  */
  if (currentPeriod[2]>0){
    currentTick[2]++;
    if (currentTick[2] >= currentPeriod[2]){
      togglePin(2,3);
      currentTick[2]=0;
    }
  }
  
#if USE_ALL
  if (currentPeriod[4]>0){
    currentTick[4]++;
    if (currentTick[4] >= currentPeriod[4]){
      togglePin(4,5);
      currentTick[4]=0;
    }
  }
  if (currentPeriod[6]>0){
    currentTick[6]++;
    if (currentTick[6] >= currentPeriod[6]){
      togglePin(6,7);
      currentTick[6]=0;
    }
  }
  if (currentPeriod[8]>0){
    currentTick[8]++;
    if (currentTick[8] >= currentPeriod[8]){
      togglePin(8,9);
      currentTick[8]=0;
    }
  }
  if (currentPeriod[10]>0){
    currentTick[10]++;
    if (currentTick[10] >= currentPeriod[10]){
      togglePin(10,11);
      currentTick[10]=0;
    }
  }
  if (currentPeriod[12]>0){
    currentTick[12]++;
    if (currentTick[12] >= currentPeriod[12]){
      togglePin(12,13);
      currentTick[12]=0;
    }
  }
  if (currentPeriod[14]>0){
    currentTick[14]++;
    if (currentTick[14] >= currentPeriod[14]){
      togglePin(14,15);
      currentTick[14]=0;
    }
  }
  if (currentPeriod[16]>0){
    currentTick[16]++;
    if (currentTick[16] >= currentPeriod[16]){
      togglePin(16,17);
      currentTick[16]=0;
    }
  }
#endif
  
}

void togglePin(byte pin, byte direction_pin) {
  
  //Switch directions if end has been reached
  if (currentPosition[pin] >= MAX_POSITION[pin]) {
    currentState[direction_pin] = HIGH;
    digitalWrite(direction_pin,HIGH);
  } 
  else if (currentPosition[pin] <= 0) {
    currentState[direction_pin] = LOW;
    digitalWrite(direction_pin,LOW);
  }
  
    //Update currentPosition
  if (currentState[direction_pin] == HIGH){
    currentPosition[pin]--;
  } 
  else {
    currentPosition[pin]++;
  }
  
  //Pulse the control pin
  digitalWrite(pin,currentState[pin]);
  currentState[pin] = ~currentState[pin];
}


//For a given controller pin, runs the read-head all the way back to 0
void reset(byte pin)
{
  digitalWrite(pin+1,HIGH); // Go in reverse
  for (byte s=0;s<MAX_POSITION[pin];s+=2){ //Half max because we're stepping directly (no toggle)
    digitalWrite(pin,HIGH);
    digitalWrite(pin,LOW);
    delay(5);
  }
  currentPosition[pin] = 0; // We're reset.
  digitalWrite(pin+1,LOW);
  currentPosition[pin+1] = 0; // Ready to go forward.
}

//Resets all the pins
void resetAll(){
  
  // Old one-at-a-time reset
  //for (byte p=FIRST_PIN;p<=PIN_MAX;p+=2){
  //  reset(p);
  //}
  
  // New all-at-once reset
  for (byte s=0;s<80;s++){ // For max drive's position
    for (byte p=FIRST_PIN;p<=PIN_MAX;p+=2){
      digitalWrite(p+1,HIGH); // Go in reverse
      digitalWrite(p,HIGH);
      digitalWrite(p,LOW);
    }
    delay(5);
  }
  
  for (byte p=FIRST_PIN;p<=PIN_MAX;p+=2){
    currentPosition[p] = 0; // We're reset.
    digitalWrite(p+1,LOW);
    currentState[p+1] = 0; // Ready to go forward.
  }
  
}



//Setup pins (Even-odd pairs for step control and direction
void setup(){
  pinMode(13, OUTPUT);// Pin 13 has an LED connected on most Arduino boards
  pinMode(2, OUTPUT); // Step control 1
  pinMode(3, OUTPUT); // Direction 1
#if USE_ALL
  pinMode(4, OUTPUT); // Step control 2
  pinMode(5, OUTPUT); // Direction 2
  pinMode(6, OUTPUT); // Step control 3
  pinMode(7, OUTPUT); // Direction 3
  pinMode(8, OUTPUT); // Step control 4
  pinMode(9, OUTPUT); // Direction 4
  pinMode(10, OUTPUT); // Step control 5
  pinMode(11, OUTPUT); // Direction 5
  pinMode(12, OUTPUT); // Step control 6
  pinMode(13, OUTPUT); // Direction 6
  pinMode(14, OUTPUT); // Step control 7
  pinMode(15, OUTPUT); // Direction 7
  pinMode(16, OUTPUT); // Step control 8
  pinMode(17, OUTPUT); // Direction 8
#endif
  Timer1.initialize(RESOLUTION); // Set up a timer at the defined resolution
  Timer1.attachInterrupt(tick); // Attach the tick function

  Serial.begin(BAUD);
}

