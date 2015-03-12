/*
 Copyright (c) 2014 NicoHood
 See the readme for credit to other people.

 USB-Serial

 Transferes from USB to HW Serial and vice versa.
 It also resets the main MCU on a DTR rise.
 */



/*

 Notes BY ARMAND @ Site3
 This code unfortunately breaks the passthrough programming of 328 unless the
 Raspberry PI fix is disabled. (it has something to do with the ras-pi overriding the 
 serial port speed (down to 9600 i think.) when you plug it into a pi.
  
*/

// Comment this out to enable aurdino/avr dude programming of the 328 chip  
#define RAS_PI_KEYBOARD_FIX


// define the reset pin to reset the destination MCU.
// this definition is made for HoodLoader2 (pin 20)
// but you still can use it with any other USB MCU or pin
const int resetPin = MAIN_MCU_RESET_PIN;

uint8_t inout; 
#define DELIM '|'

void setup() {
  // set main MCU by default active
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, HIGH);

  // Start USB Serial
  Serial.begin(115200);
  
 
  Keyboard.begin();
 
  // apparently this gets somehow ignored!
  Serial1.begin(57600);

    inout = 0;  

}

void loop() {

  #ifdef RAS_PI_KEYBOARD_FIX
 // Raspberry PI Keyboard hack fix thing to allow communication back with 328
//  it seems to override it to 9600 or what not. 
  Serial1.begin(57600);
#endif

  // USB -> Serial
  for (int i = 0; i < USB_EP_SIZE; i++) {
    // read maximum one EP_SIZE to not block
    if (Serial.available())
      Serial1.write(Serial.read());
    else break;
  }

  // Serial -> USB
  if (Serial1.available()) {
    // send maximum one EP_SIZE to give the usb some time to flush the buffer
    uint8_t buff[USB_EP_SIZE - 1];
    int i = 0;
    for (i = 0; i < sizeof(buff); i++) {
      if (Serial1.available())
        buff[i] = Serial1.read();
      else break;
    }
    
    Serial.write(buff, i);

   // send to keyboard
    int j=0;
    for (j=0; j < i; j++)
    {
      if (inout)
      {
        Keyboard.press(buff[j]);
        Keyboard.releaseAll();
      }
          // inside delimiter?
      inout = (DELIM == buff[j])? 1 : 0;
    }


  }
}

void CDC_LineEncodingEvent(void)
{
  // start HW Serial with new baud rate
  Serial1.end();
  Serial1.begin(Serial.baud());
}

void CDC_LineStateEvent(void) {
  // reset the main mcu if DTR goes HIGH
  if (Serial.dtr())
    digitalWrite(resetPin, LOW);
  else
    digitalWrite(resetPin, HIGH);
}
