String sim900IncomingMessage = "";
int sim900IncomingMessageHandler = -1;
boolean sim900IncomingMessageReady = false;


void SIM800_Setup() {
  Serial2.begin(115200);
}

void sendCommand(String command) {
  Serial.println("Sending:" + command);
  Serial2.print(command + "\r");
} //end sendCommand


void listenToComPort() {
  if (Serial2.available()) {
    char readSIM900Input = Serial2.read();
    sim900IncomingMessage += readSIM900Input;

//    Serial.print(sim900IncomingMessage);
//    Serial.print("/");
//    Serial.print(sim900IncomingMessageHandler);
//    Serial.print(":");
//    Serial.print(readSIM900Input);
//    Serial.print("-");
//    Serial.println((byte)readSIM900Input);
    // get last indx
    int lastIndexOfRN = sim900IncomingMessage.lastIndexOf("\r\n");
    int lastIndexOfATD = sim900IncomingMessage.lastIndexOf("ATD");

    if (lastIndexOfATD != -1 && sim900IncomingMessage.lastIndexOf("\r") != -1) {
        sim900IncomingMessageHandler = -1;
        sim900IncomingMessageReady = true;
    } 
    else if (sim900IncomingMessageHandler == -1 && lastIndexOfRN != -1) {
      sim900IncomingMessageHandler = lastIndexOfRN;
      
    } else if (sim900IncomingMessageHandler != -1 && sim900IncomingMessageHandler != lastIndexOfRN) {
      sim900IncomingMessageHandler = -1;
      sim900IncomingMessageReady = true;
    }//end if
  } //end if
} //end listenToComPort
