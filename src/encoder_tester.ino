#include "FS.h"
#include "SD.h"
#include "SPI.h"


#define ChannelA 33  //Green
#define ChannelA_negatted 25
#define ChannelB 26  //Purple
#define ChannelB_negatted 27
#define ChannelZ 14 //Yellow
#define ChannelZ_negatted 12


volatile long encoderPos = 0;
volatile long encoderPosInversed = 0;
volatile long FullRotations = 0;
volatile int sd_card = 1;
File myFile;


TaskHandle_t Task1;
TaskHandle_t Task2;

String endChar = String(char(0xff)) + String(char(0xff)) + String(char(0xff));



bool appendFile(fs::FS &fs, const char * path, const char * message){
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return false;
  }
  if(file.print(message)){
    delay(20);
      //Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
    return false;
  }
  file.close();
  return true;
}

void setup() {
  Serial.begin(115200);
  //Create the two different takst to each Core

  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    sd_card=0;
  }

  //Core 0
  xTaskCreatePinnedToCore(
    TaskCore0, /* Function to implement the task */
    "Task1", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    1,  /* Priority of the task */
    &Task1,  /* Task handle. */
    0); /* Core where the task should run */

  //Core 1
  xTaskCreatePinnedToCore(
    TaskCore1,
    "Attach_Interrupt_Task",
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    1,  /* Priority of the task */
    &Task2,  /* Task handle. */
    1); /* Core where the task should run */

  if (sd_card==1){
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
  }

  //Declare the Pin functionality
  pinMode(ChannelA, INPUT_PULLUP);
  pinMode(ChannelA_negatted, INPUT_PULLUP);
  pinMode(ChannelB, INPUT_PULLUP);
  pinMode(ChannelB_negatted, INPUT_PULLUP);
  pinMode(ChannelZ, INPUT_PULLUP);
  pinMode(ChannelZ_negatted, INPUT_PULLUP);
  delay(2000);
}

void TaskCore1(void *pvParameters) {
  //Attach Interrupt in core 1
  Serial.print("TaskCore1 running on core ");
  Serial.println(xPortGetCoreID());
  Serial.println("With Frequency: " + String(getCpuFrequencyMhz()));
  attachInterrupt(digitalPinToInterrupt(ChannelA), doEncoderA, RISING);
  attachInterrupt(digitalPinToInterrupt(ChannelB), doEncoderB, RISING);
  attachInterrupt(digitalPinToInterrupt(ChannelZ), doEncoderZ, RISING);
  for (;;) {
  }
  vTaskDelete(NULL);
}

void TaskCore0( void * pvParameters) {
  Serial2.begin(9600);
  while (!Serial2) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("TaskCore0 running on core ");
  Serial.println(xPortGetCoreID());
  Serial.println("With Frequency: " + String(getCpuFrequencyMhz()));
  if (sd_card==1){
    while(!appendFile(SD, "/encoder.txt", "\n% Start of Writing %\n")){
    }
  }
  delay(2000);
  for (;;) {
   int ValueA = digitalRead(ChannelA);
    int ValueB = digitalRead(ChannelB);
    int Value_A = digitalRead(ChannelA_negatted);
    int Value_B = digitalRead(ChannelB_negatted);
    int ValueZ = digitalRead(ChannelZ);
    int Value_Z = digitalRead(ChannelZ_negatted);
     // Serial.println("Pos: " + String(encoderPos) + "  Full Rotations: " + String(FullRotations));
   // Serial.println("Channel A: " + String(ValueA) + "  Channel B: " + String(ValueB) + "  Channel _A: "+ String(Value_A) + "  Channel _B: "+ String(Value_B) + "  Channel Z: "+ String(ValueZ) + "  Channel _Z: "+ String(Value_Z));

    Serial2.print("n0.val=" + String(encoderPos) + endChar);
    Serial2.print("n1.val=" + String(FullRotations) + endChar);
    Serial2.print("add 4,0," + String(ValueA*30) + endChar);
    Serial2.print("add 4,1," + String((ValueB + 10)*30) + endChar);
    Serial2.print("add 5,0," + String(Value_A*30) + endChar);
    Serial2.print("add 5,1," + String((Value_B + 10)*30) + endChar);
    Serial2.print("add 9,0," + String(ValueZ*30) + endChar);
    Serial2.print("add 9,1," + String((Value_Z + 10)*30) + endChar);
    if (SD.begin(5) && sd_card==1) {
      char buffer [33];
      appendFile(SD, "/encoder.txt", " ;Channel A: ");
      appendFile(SD, "/encoder.txt", itoa(ValueA,buffer,10));
      appendFile(SD, "/encoder.txt", " ;Channel B: ");
      appendFile(SD, "/encoder.txt", itoa(ValueB,buffer,10));
      appendFile(SD, "/encoder.txt", " ;Channel _A: ");
      appendFile(SD, "/encoder.txt", itoa(Value_A,buffer,10));
      appendFile(SD, "/encoder.txt", " ;Channel _B: ");
      appendFile(SD, "/encoder.txt", itoa(Value_B,buffer,10));
      appendFile(SD, "/encoder.txt", " ;Channel Z: ");
      appendFile(SD, "/encoder.txt", itoa(ValueZ,buffer,10));
      appendFile(SD, "/encoder.txt", " ;Channel _Z: ");
      appendFile(SD, "/encoder.txt", itoa(Value_Z,buffer,10));
      appendFile(SD, "/encoder.txt", "\n");
    }


  }
  vTaskDelete(NULL);
}

void loop() {
  
}

void doEncoderA()
{
  if (digitalRead(ChannelA) != digitalRead(ChannelB)) {
    encoderPos++;
  } else {
    encoderPos--;
  }
}

void doEncoderB()
{
  if (digitalRead(ChannelA) == digitalRead(ChannelB)) {
    encoderPos++;
  } else {
    encoderPos--;
  }
}

void doEncoderZ()
{
  if (digitalRead(ChannelZ) == 1) {
    FullRotations++;
  }
}
