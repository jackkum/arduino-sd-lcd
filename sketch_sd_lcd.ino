#include <SD.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// lcd init
LiquidCrystal lcd(2,3,4,5,6,7);
// Root path
File root;

/**
 * File struct
 */
typedef struct {
  char name[13];
  unsigned int size;
} file_list;

// buttons
const int buttonUpPin     = 8;
const int buttonDownPin   = 9;
// ligth
const int ledPin          =  14;

// state of button
unsigned int buttonState  = 0;
// counter of files
unsigned int filesCount   = 0;
// index of file
unsigned int currentIndex = 0;

// files array
file_list *files;

/**
 * Resize files array
 */
file_list *addFile(file_list *term){
  file_list *t = (file_list*) realloc(term, sizeof(term) + sizeof(file_list));
  if(t == NULL){
    free(term);
  }

  return t;
}

/**
 * Clear lcd
 */
void lcdClear() {
  // left part of lcd
  lcd.setCursor(0,0);

  for(int i = 0; i < 8; i++){
    // write space
    lcd.write(0x20);
  }

  // right part of lcd
  lcd.setCursor(0,1);

  for(int i = 0; i < 8; i++){
    // write space
    lcd.write(0x20);
  }
}

/**
 * Write to lcd
 */
void lcdPrint(const char * str){
  // clear it
  lcdClear();

  // move to start
  lcd.setCursor(0,0);

  // write forst 8 bytes
  for(int i = 0; i < 8; i++){
    // no more bytes
    if(!(*str)){
      return;
    }

    lcd.write(*str++);
  }

  // move to right part of lcd
  lcd.setCursor(0,1);

  for(int i = 0; i < 8; i++){
    // nothing to write
    if(!(*str)){
      return;
    }

    lcd.write(*str++);
  }
}

/**
 * Init device
 */
void setup() {
  
  // init lcd
  lcd.begin(8, 2);
  
  // print init
  lcdPrint("Init SD card...");

  // Debug information
  Serial.begin(9600);
  Serial.print("Initializing SD card...");

  // IO init
  pinMode(10,            OUTPUT);
  pinMode(buttonUpPin,   INPUT);
  pinMode(buttonDownPin, INPUT);
  pinMode(ledPin,        OUTPUT);

  // Init sd card
  if ( ! SD.begin(10)) {
    Serial.println("initialization failed!");
    lcdPrint("Init SD faild");
    return;
  }

  Serial.println("Open root dir...");
  lcdPrint("Read dir...");
  root = SD.open("/");

  // read current directory
  readDirectory(root);

  // no files on sd
  if(filesCount == 0){
    Serial.println("No files");
    lcdPrint("No files");
  } else {
    // print current (first) file
    Serial.println(files[currentIndex].name);
    lcdPrint(files[currentIndex].name);
  }
}

/**
 * Moving position
 */
void movePos(const int to) {
  
  int index    = currentIndex + to;
  currentIndex = (int)(index % (index < 0 ? -filesCount : filesCount));

  Serial.print("Index: ");
  Serial.write(currentIndex + 0x30);
  Serial.write('\n');

  // print current filename
  lcdPrint(files[currentIndex].name);
  Serial.println(files[currentIndex].name);
}

/**
 * Work loop
 * Reading buttons
 */
void loop() {

  // read state of button up
  buttonState = digitalRead(buttonUpPin);

  // button pushed
  if (buttonState == HIGH) {
    // lighting
    digitalWrite(ledPin, HIGH);

    // move up cursor
    movePos(-1);

    // wait
    delay(500);
  } else {
    // switch off the light
    digitalWrite(ledPin, LOW);
  }

  // read state of button down
  buttonState = digitalRead(buttonDownPin);

  if (buttonState == HIGH) {
    // lighting
    digitalWrite(ledPin, HIGH);

    // move down cursor
    movePos(1);

    // wair
    delay(1000);
  } else {
    // switch off the light
    digitalWrite(ledPin, LOW);
  }
}

/**
 * Reading directory
 */
void readDirectory(File dir) {
  
  while(true) {

    // get file or dir
    File entry =  dir.openNextFile();

    // no more files
    if ( ! entry) {
      // reset position
      dir.rewindDirectory();
      break;
    }

    // just debug
    Serial.println(entry.name());

    // resize files array
    files = addFile(files);

    if(files == NULL){
      Serial.println("Error adding memory");
      return;
    }
    
    // copy name of file
    strcpy(files[filesCount].name, entry.name());
    // remeber size of file
    files[filesCount].size = entry.size();
    // add counter
    filesCount++;
  }
}


