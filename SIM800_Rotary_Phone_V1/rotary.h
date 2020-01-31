#define INDIALPIN 2
#define PULSEPIN 15 //2

boolean inDialPinLastState = HIGH;
boolean inDialPinState = HIGH;
unsigned long lastinDialPinStateChange;

boolean pulsPinLastState;
boolean pulsPinState;
unsigned long lastPulsPinStateChange;
byte counter = 0;
int debounceDelay = 10;


void rotary_setup() {
  pinMode(INDIALPIN, INPUT_PULLUP);
  pinMode(PULSEPIN, INPUT_PULLUP);
  inDialPinLastState = digitalRead(INDIALPIN);
  pulsPinLastState = digitalRead(PULSEPIN);
}


byte process_rotary() {
  byte rotaryDialedDigit = 99;

  // IN DIAL
  boolean inDialCurrentPinState = digitalRead(INDIALPIN);

  // if we got change
  if (inDialCurrentPinState != inDialPinLastState) {
    // reset the debouncing timer
    lastinDialPinStateChange = millis();
  }

  // if debounce time is over
  if ((millis() - lastinDialPinStateChange) > debounceDelay) {
    // if the button state has changed:
    if (inDialCurrentPinState != inDialPinState) {
      // set current
      inDialPinState = inDialCurrentPinState;

      if (!inDialPinState) {
        Serial.println("Start of dial");
        counter = 0;
      } else {
        Serial.println("End of dial");
        if (counter) {
          if (counter == 10)
            counter = 0;
          rotaryDialedDigit = counter;
        }//end if
      } //end if
    } //endf if
  } //end if
  inDialPinLastState = inDialCurrentPinState;

  // pulse state
  boolean pulsPinCurrentState = digitalRead(PULSEPIN);

  // if we got change
  if (pulsPinCurrentState != pulsPinLastState) {
    // reset the debouncing timer
    lastPulsPinStateChange = millis();
  }

  // if debounce time is over
  if ((millis() - lastPulsPinStateChange) > debounceDelay) {
    // if the button state has changed:
    if (pulsPinCurrentState != pulsPinState) {
      // set current
      pulsPinState = pulsPinCurrentState;
      // only add on LOW
      if (pulsPinState == LOW) {
        counter++;
      } //end if
    } //end if
  } //endf if

  pulsPinLastState = pulsPinCurrentState;

  // return value
  return rotaryDialedDigit;
}
