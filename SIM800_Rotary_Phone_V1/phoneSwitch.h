#define phoneSwitchPin 4
#define debounceDelay 50

byte buttonState = HIGH;
byte lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
byte flagphoneSwitchChange = 0;


byte isOffTheHook() {
  return !buttonState;
}

void phoneSwitch_setup() {
  pinMode(phoneSwitchPin, INPUT_PULLUP);
}

void process_phoneSwitch() {
  // reset flag
  flagphoneSwitchChange = 0;
  
  // read the state of the switch into a local variable:
  byte currentButtonState = digitalRead(phoneSwitchPin);

  // If the switch changed, due to noise or pressing:
  if (currentButtonState != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  } //end if 

  if ((millis() - lastDebounceTime) > debounceDelay) {

    // if the button state has changed:
    if (currentButtonState != buttonState) {
      buttonState = currentButtonState;
      flagphoneSwitchChange = 1;
    } //end if 
  } //end if 

  lastButtonState = currentButtonState;
}
