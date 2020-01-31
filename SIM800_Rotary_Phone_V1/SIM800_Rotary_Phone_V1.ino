/*
Code by: Tal Ofer (talofer99@hotmail.com)

For more info - 
https://create.arduino.cc/projecthub/talofer99/taking-a-rotary-dial-phone-into-the-future-c974b2

*/

#define I2SEXISTS true

#include <wifiTool.h>
WifiTool wifiTool;

#ifdef I2SEXISTS
#include "I2S.h"
#endif

#include "SIM800.h"
#include "rotary.h"
#include "phoneSwitch.h"

#define SS_CHECK_HARDWARE 0
#define SS_CHECK_LINE 1
#define SS_IDLE 2
#define SS_INCOMING_CALL 3
#define SS_IN_CALL 4
#define SS_DAILING 5
#define SS_WIFITOOL 6

#define INTCALLCODE 972

String callerID;
String reply;
char dialedNumber[16];
byte dialedNumberCursor = 0;
String intFrmtNumber;
byte systemState;
unsigned long systemStateStartMillis;

void setup() {
  // start serial
  Serial.begin(115200);
  Serial.println("System started");

#ifdef I2SEXISTS
  // I2SEXISTS setup
  I2S_setup();
  say_sentence(SENTENCE_SYSTEMCHECK);
#endif
  wifiTool.begin(false);
  if (!wifiTool.wifiAutoConnect())
  {
    Serial.println("fail to connect to wifi!!!!");
  }

  // phone switch
  phoneSwitch_setup();

  // setup rotary
  rotary_setup();

  // setup sim 800
  SIM800_Setup();

  // set system state to ZERO
  setSystemState(SS_CHECK_HARDWARE);
}

void loop() {
  // process logic
  processBySystemState();

  // listen to the port for incoming data
  listenToComPort();

  // handle returned values
  if (sim900IncomingMessageReady) {

    // run the process
    processsim900IncomingMessage();

    // clear flag and string
    sim900IncomingMessageReady = false;
    sim900IncomingMessage = "";
  } //end if

}




// PROCESS RETURN DATA
void processsim900IncomingMessage() {
  Serial.print(F("sim900IncomingMessage - "));
  String stringTwo = sim900IncomingMessage;
  stringTwo.replace("0522339145", "0522******");
  Serial.println(stringTwo);

  // ATH -- > Hang Up Call
  if (sim900IncomingMessage.indexOf("ATH") >= 0) {
    setSystemState(SS_IDLE);
  } //end if (before else if)

  // ATD -- > MAKE CALL
  else if (sim900IncomingMessage.indexOf("ATD") >= 0) {
    //set in DAILING
    setSystemState(SS_DAILING);
  } //end if (before else if)

  // NO CARRIER -- > hang up (other side on RING)
  else if (sim900IncomingMessage.indexOf("NO CARRIER") >= 0) {
    setSystemState(SS_IDLE);
  } //end if (before else if)

  // ATA -- > Anser Call
  else if (sim900IncomingMessage.indexOf("ATA") >= 0) {
    setSystemState(SS_IN_CALL);
  } //end if (before else if)


  // BUSY -- > HANG ON OTHER SIDE
  else if (sim900IncomingMessage.indexOf("BUSY") >= 0) {
    //set to phone ready
    printSecondLine("BUSY........");
    delay(1000);
    setSystemState(SS_IDLE);
  } //end if (before else if)

  // COLP -- > CALL ANSWERED BY DIAL
  else if (sim900IncomingMessage.indexOf("COLP") >= 0) {
    setSystemState(SS_IN_CALL); //set in call
  } //end if (before else if)

  // CLIP incoming caller ID
  else if (sim900IncomingMessage.indexOf("+CLIP") > 0) {
    // clear the caller ID
    callerID = "";
    // look for clip location
    int clipPoistion = sim900IncomingMessage.indexOf("+CLIP:");
    // if we found clip as well
    if (clipPoistion > 0) {
      //adject the postion to trim the number
      clipPoistion += 8;
      // look for nd dbl-qoute
      int secDblQuotePosition = sim900IncomingMessage.indexOf("\"", clipPoistion);
      if (secDblQuotePosition > 0) {
        callerID = sim900IncomingMessage.substring(clipPoistion, secDblQuotePosition);
        displayCallerID();
      } //end if
    }//end if
  } //end if (before else if)

  // incoming call RING
  else if (sim900IncomingMessage.indexOf("RING") > 0) {
    //set system state to call
    if (systemState != SS_INCOMING_CALL && (systemState != SS_IN_CALL && systemStateStartMillis + 500 < millis())) {
      // set system to 3 - incoming call
      setSystemState(SS_INCOMING_CALL);
    } //end if
  } //end if (before else if)

  // AT COMMAND
  else if (sim900IncomingMessage.indexOf("AT\r\r\n") >= 0) {
    // set to system state 1 - check line
    setSystemState(SS_CHECK_LINE);
  } //end if (before else if)

  // CCALR: command
  else if (sim900IncomingMessage.indexOf("CCALR:") >= 0) {
    char reply = sim900IncomingMessage[sim900IncomingMessage.indexOf("CCALR:") + 7];
    if (reply == '1') {
      setSystemState(SS_IDLE);
#ifdef I2SEXISTS
      say_sentence(SENTENCE_PHONEREADY);
#endif
      sendCommand("AT+CRSL=0"); //turn off ring on the handset
    } else {
      printSecondLine("NOT READY ...");
      delay(3000);
      // retry
      setSystemState(SS_CHECK_LINE);
    } //end if
  } //end if (before else if)

  // NORMAL POWER DOWN -- > SIM900 IS SHUTTING DOWN
  else if (sim900IncomingMessage.indexOf("NORMAL POWER DOWN") >= 0) {
    setSystemState(SS_CHECK_HARDWARE); //set to check module
  } //end if (before else if)

  // NORMAL POWER DOWN -- > SIM900 IS SHUTTING DOWN
  else if (sim900IncomingMessage.indexOf("NO DIALTONE") >= 0) {
    printSecondLine("NO DIALTONE...");
    delay(1000);
    setSystemState(SS_CHECK_LINE); //check line is ready
  } //end if (before else if)

  // DTMF
  else if (sim900IncomingMessage.indexOf("DTMF") >= 0) {
    char reply = sim900IncomingMessage[sim900IncomingMessage.indexOf("DTMF") + 5];
    // echo out
    Serial.print(reply);
    Serial2.print(reply);
    switch (reply) {
      case '2': //2
        //setChannelOnlyOn(0);
        break;
      case '4': //4
        //setChannelOnlyOn(1);
        break;
      case '6': //6
        //setChannelOnlyOn(2);
        break;
      case '8': //8
        //setChannelOnlyOn(3);
        break;
      default:
        //setChannelOnlyOn(4); // do not exists - will clear all
        break;
    }
    // 1 = char(49)
    //if (reply >= 49 && reply < 49 + totalChannels) {
    //flipChannelState(reply-49);
    //
    //} //end if
  } //end if (before else if)

  // CPBR: 1 -- > GET FIRST RECORD OF SOMTHING
  else if (sim900IncomingMessage.indexOf("CPBR: 1,\"+") >= 0) {
    // calc postion of number
    byte startPos = sim900IncomingMessage.indexOf("CPBR: 1,\"+") + 10;
    byte secDblQuotePosition = sim900IncomingMessage.indexOf("\"", startPos);
    //get number
    String phoneNumer = sim900IncomingMessage.substring(startPos, secDblQuotePosition);

    // add to dial
    for (byte i = 0; i < phoneNumer.length(); i++) {
      addDigitToDialNumber(phoneNumer[i]);
    } //end for


  } else {
    //Serial.print(F("UNMANGED RETURN DATA:"));
    //Serial.println(sim900IncomingMessage);
  } //end if

} //end processsim900IncomingMessage


// ************************************************************
// PROCESS BY STATE
// ************************************************************

void processBySystemState() {
  switch (systemState) {
    case SS_CHECK_HARDWARE:
      // if its been more then 1000 millis
      if (systemStateStartMillis + 1000 < millis()) {
        // show fail message
        printSecondLine("FAIL.....");
        // delay
        delay(1000);
        // try again
        setSystemState(SS_CHECK_HARDWARE);
      } //end if
      break;

    case SS_CHECK_LINE:
      // if its been more then 5000 millis
      if (systemStateStartMillis + 5000 < millis()) {
        // show fail message
        printSecondLine("FAIL.....");
        // delay
        delay(1000);
        // try again
        setSystemState(SS_CHECK_LINE);
      } //end if
      break;

    case SS_IDLE: {
        // IDLE STATE

        // procees the phone switch state
        process_phoneSwitch();
        // if change in the switch
        if (flagphoneSwitchChange) {
          // if low - means its picked up
          if (isOffTheHook()) {
            // reset the dialed number
            resetDialedNumber();
          } //end if
        } //end if

        // get any dialed number
        byte dialed_digit = process_rotary();

        // only dial if off the hook
        if (isOffTheHook()) {

          if (dialed_digit != 99) {
            addDigitToDialNumber(dialed_digit + 48);
#ifdef I2SEXISTS
            say_digit(dialed_digit);
#endif
          } //end if
          // if we dialed a full 10 digit number
          if ((dialedNumberCursor == 10 && dialedNumber[1] == '5') || (dialedNumberCursor == 9 && dialedNumber[1] != '5') ) {
            // announce
#ifdef I2SEXISTS
            say_sentence(SENTENCE_DAILINGNUMBER);
#endif
            // dial the number
            dialNumber();
            // reset dialed number
            resetDialedNumber();
          } //end if

        } else {
          // if digit that was dialed was NINE we acticvate the AP
          if (dialed_digit == 9) {
            setSystemState(SS_WIFITOOL);
          } //end if
        } //end if

      } //end case
      break;
    case SS_INCOMING_CALL:
      {
        // INCOMING CALL (RING)
        // process song
#ifdef I2SEXISTS
        process_ringer();
#endif
        // procees the phone switch state
        process_phoneSwitch();
        // if change in the switch
        if (flagphoneSwitchChange) {
          if (isOffTheHook()) {
            Serial.println("Answering .... ");
            // send answer command
            sendCommand("ATA");
          } else {
            // send hangup  command
            sendCommand("ATH");
          } //end if
        } //end if
      } //end case
      break;
    case SS_IN_CALL:
      // IN CALL
      // procees the phone switch state
      process_phoneSwitch();
      // if change in the switch
      if (flagphoneSwitchChange) {
        if (!isOffTheHook()) {
          sendCommand("ATH");
        } //end if
      } //end if
      break;
    case SS_DAILING:
      // IN DAILING
      // procees the phone switch state
      process_phoneSwitch();
      // if change in the switch
      if (flagphoneSwitchChange) {
        if (!isOffTheHook()) {
          sendCommand("ATH");
        } //end if
      } //end if
      break;
    case SS_WIFITOOL:
      //do nothing
      break;
    default:
      Serial.println(F("UN-HANDELDED SYSTEM STATE ON PROCESS"));
      break;

  } //end switch
} //end processBySystemState

void setSystemState(byte newState) {
  switch (newState) {
    case SS_CHECK_HARDWARE:
      printNew("HARDWARE CHECK..");
      sendCommand("AT");
      break;
    case SS_CHECK_LINE:
      printNew("LINE CHECK..");
      sendCommand("AT+CCALR?");
      break;
    case SS_IDLE:
      // reset dialed number
      resetDialedNumber();
      // in case the rtttl is playing
#ifdef I2SEXISTS
      stop_ringer();
#endif
      printNew("PHONE READY");
      //sendCommand("AT+DDET=1"); // created error - to check data sheet for AT command on the 800 vs 900
      break;
    case SS_INCOMING_CALL:
      printNew("INCOMING CALL");
#ifdef I2SEXISTS
      activate_ringer();
#endif
      break;
    case SS_IN_CALL:
      // in case the rtttl is playing
#ifdef I2SEXISTS
      stop_ringer();
#endif
      //adjuest display
      printNew("IN CALL");
      displayCallerID();
      break;
    case SS_DAILING:
      printNew("DIALING");
      // for diplay later
      callerID = intFrmtNumber;
      // print number
      printSecondLine(intFrmtNumber);
      break;
    case SS_WIFITOOL:
      Serial.println(F("Running wifi AP for configuration"));
#ifdef I2SEXISTS
      say_sentence(SENTENCE_RUNWIFIPORTAL);
#endif
      if (!wifiTool.wifiAutoConnect())
      {
        Serial.println("fail to connect to wifi!!!!");
        wifiTool.runApPortal();
      }
      break;
    default:
      Serial.println(F("UN-HANDELED NEW SYSTEM STATE"));
      break;
  } //end switch

  systemState = newState;
  systemStateStartMillis = millis();
} //end setSystemState


// set the dialedNumberCursor to zero
void resetDialedNumber() {
  memset(dialedNumber, 0, 16);
  dialedNumberCursor = 0;
}

// add digit to dialed number
void addDigitToDialNumber(char getKeyPadInput) {
  //printSecondLineCol(getKeyPadInput, dialedNumberCursor);
  Serial.println(getKeyPadInput);
  dialedNumber[dialedNumberCursor] = getKeyPadInput;
  dialedNumberCursor++;
} //end addDigitToDialNumber


// dial number
void dialNumber() {
  // clear
  intFrmtNumber = "";
  //define starting point
  byte startingPoint = 0;
  // if number start with ZERO we remove it
  if (dialedNumber[0] == '0') {
    intFrmtNumber = INTCALLCODE;
    startingPoint = 1;
  } //end if

  for (byte i = startingPoint; i < dialedNumberCursor; i++) {
    intFrmtNumber += dialedNumber[i];
  } //end for
  // MAKE CALL
  sendCommand("ATD + +" + intFrmtNumber + ";");
} //end dial number

// TO BE ADJUSTED TO PLACE

void printNew(String line) {
  Serial.println("printNew - " + line);
}

void printSecondLine(String line) {
  Serial.println("printSecondLine - " + line);
}
void displayCallerID() {
  Serial.println("displayCallerID");
}


void printSecondLineCol(char message, byte col) {
  Serial.println("printSecondLineCol");
}

//void process_rtttl() {
//  //Serial.println("process_rtttl");
//}
//
//void stop_rtttl() {
//  Serial.println("stop_rtttl");
//}
