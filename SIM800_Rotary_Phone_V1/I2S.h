#include <Arduino.h>
#include <ESP8266SAM.h>
#include <AudioFileSourceSPIFFS.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>

AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2S *out = NULL;
AudioFileSourceID3 *id3;


// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode) {
    string += 2;
  }

  while (*string) {
    char a = *(string++);
    if (isUnicode) {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();
}


void I2S_setup() {
  SPIFFS.begin();

  out = new AudioOutputI2S();
  out->begin();
  out->SetGain(1.0);

  mp3 = new AudioGeneratorMP3();
}


void activate_ringer() {
  audioLogger = &Serial;
  file = new AudioFileSourceSPIFFS("/ring.mp3");
  id3 = new AudioFileSourceID3(file);
  id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
  //out = new AudioOutputI2S();
  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);
}

void stop_ringer() {
  if (mp3->isRunning())
    mp3->stop();
}

void process_ringer() {
  if (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  } else {
    Serial.printf("MP3 done\n");
    delay(100);
  } //end if
} //end process_ringer()


#define SENTENCE_SYSTEMCHECK 0
#define SENTENCE_PHONEREADY 1
#define SENTENCE_DAILINGNUMBER 2
#define SENTENCE_RUNWIFIPORTAL 3


void say_sentence(byte ID) {
  ESP8266SAM *sam = new ESP8266SAM;
  switch (ID) {
    case SENTENCE_SYSTEMCHECK:
      sam->Say(out, "please wait running system test.");
      break;
    case SENTENCE_PHONEREADY:
      sam->Say(out, "phone is ready for use.");
      break;
    case SENTENCE_DAILINGNUMBER:
      sam->Say(out, "dailing number.");
      break;
    case SENTENCE_RUNWIFIPORTAL:
      sam->Say(out, "activating wifi portal.");
      break;
    default:
      sam->Say(out, "error.");
      break;
  }

  delete sam;
}


void say_digit(byte digit) {
  ESP8266SAM *sam = new ESP8266SAM;
  switch (digit) {
    case 0:
      sam->Say(out, "zero.");
      break;
    case 1:
      sam->Say(out, "one.");
      break;
    case 2:
      sam->Say(out, "two.");
      break;
    case 3:
      sam->Say(out, "three.");
      break;
    case 4:
      sam->Say(out, "four.");
      break;
    case 5:
      sam->Say(out, "five.");
      break;
    case 6:
      sam->Say(out, "six.");
      break;
    case 7:
      sam->Say(out, "seven.");
      break;
    case 8:
      sam->Say(out, "eight.");
      break;
    case 9:
      sam->Say(out, "nine.");
      break;
    default:
      sam->Say(out, "error.");
      break;
  }

  delete sam;
}
