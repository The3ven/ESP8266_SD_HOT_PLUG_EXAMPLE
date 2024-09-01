/*
SD Card HOT Plug Example

This example is for demostrate SD Card HOT PLUG Support Using
NODEMCU (ESP8266) Using Scheduler lib and generic listfiles examples

  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

  created   Nov 1 24
  The3ven
  
  This example code is in the public domain.

*/



#include <Arduino.h>
#include <Scheduler.h>
#include <Task.h>
#include <SPI.h>
#include <SD.h>

File root;
bool begin_check = false;

String sdCheckFile = "sdTest.txt";
int blink_delay = 1000;
bool sd_status = false;
void printDirectory(File dir, int numTabs);


String fileNameChecker(String fileName) {
  return (fileName[0] != '/') ? "/" + fileName : fileName;
}



class SdTask : public Task {
protected:
  void loop() {
    bool All_Is_Well = begin_check && sd_status;
    Serial.println("All well ? " + String(All_Is_Well));
    if (All_Is_Well) {
      delay(1000);
      root = SD.open("/");
      printDirectory(root, 4);
      Serial.println("done!");
    } else {
      pinMode(2, HIGH);
    }
    delay(1000);
  }
} sd_task;



class SdBeginTask : public Task {
protected:
  void loop() {
    sdCheckFile = fileNameChecker(sdCheckFile);
    if (SD.exists(sdCheckFile)) {  // in case of as card begin previously
      begin_check = true;
    } else {
      if (sd_status) {
        File testFile;
        testFile = SD.open(sdCheckFile, "w+");  // New SD card and there is no test file to check status then create one
        if (!testFile)                          // can`t open file seems sd card is not connected or can`t begin
        {
          begin_check = false;  // sd not begin
        } else {
          // delay(2);
          // unsigned int BeginCnt = testFile.readString().toInt();
          // delay(2);
          // BeginCnt++;
          // delay(2);
          // Serial.println("SD Begin Count " + String(BeginCnt));
          // testFile.print(String(BeginCnt));
          // delay(2);
          begin_check = true;  // file created sd card is ok
        }
      } else {
        begin_check = false;  // Sd card is not begin previously
      }
    }

    if (!begin_check) {  // can`t create and file is not exist
      if (sd_status) {   // if SD card is begin previously then close sd card first!
        SD.end();
      }

      if (!SD.begin(SS)) {  // check we can begin sd card ?
        sd_status = false;  // we can`t begin
      } else {
        sd_status = true;  // it got begin in one go
      }

      for (int i = 0; i < 20 && !sd_status && !SD.begin(SS); i++)
        ;  // now try to begin it 20 time

      if (!sd_status && !SD.begin(SS)) {  // still not begin ?
        sd_status = false;                // no it`s dead sd card sad
      } else {
        sd_status = true;  // it begin successfully
      }
    }
    yield();     // fed shit dog so it keep quite and let us do our work lol!
    delay(500);  // check sd card each 500 mili second
  }
} sdbegin_task;

void setup() {
  Serial.begin(115200); // Serial Communication begin
  pinMode(2, OUTPUT); // ESP8266 inbuild led
  pinMode(2, HIGH); // set HIGH to light it up
  Scheduler.start(&sdbegin_task); // SD Card handling task
  Scheduler.start(&sd_task); // Print file list tast
  Scheduler.begin(); // start scheduler 
}

void printDirectory(File dir, int numTabs) {
  pinMode(2, HIGH);
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) { Serial.print('\t'); }
    Serial.print(entry.name());
    pinMode(2, LOW);
    delay(200);
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      time_t cr = entry.getCreationTime();
      time_t lw = entry.getLastWrite();
      struct tm* tmstruct = localtime(&cr);
      Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      tmstruct = localtime(&lw);
      Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    pinMode(2, HIGH);
    entry.close();
  }
}


void loop() {
}
