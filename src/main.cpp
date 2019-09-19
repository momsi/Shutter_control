/*--------------To do
Maybe set the interrupt to FALLING instead od LOW (testing needed, FALLING could be missed in debouncing delays.)
*/


/*-------------Setup
Using an Arduino Nano clone

Keypad: generic 4x4 (non serial)
    Attach to Pins (from left to right): 12,11,10,9,8,7,6,5

Display: SSD1306 128 x 64 OLED
    Attach PINS:  VCC:5V
                  GND:GND
                  SCL:A5
                  SDA:A4
    I2C Adress:   0x3C

Mode Switch:
    Attach to PIN D4
    Pullup!:  LOW is Auto-mode
              HIGH is Manual-mode 

Shutter-Trigger: 
    Attach to PIN D3 and GND
    Pullup!:  Is triggered when pulled to GND 

(Abort-Trigger:
    Attach to PIN D2 and GND
    Pullup!:  Is triggered when pulled to GND
    INTERRUPT: Needs to be attached to an interrupt PIN, confirm if using different controller)
            
*/

#include <Arduino.h>
#include <keypad.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

//---------------- Variables
int INTERNAL_LED_PIN = 13;
int MODE_SWITCH_PIN = 4;
int TRIGGER_BUTTON_PIN = 3;
int EMERGENCY_ABORT_PIN =2;

volatile bool EMERGENCY_ABORT_CALLED=false;
volatile bool AUTO_MODE_ENABLED;
volatile int time_to_open=0;
volatile unsigned long time_sutter_was_opened=0;
volatile unsigned long time_open=0;

//---------------- Keypad
const byte numRows=4;
const byte numCols=4;
char keymap[numRows][numCols] ={
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}};
byte rowPins[numRows] = {12,11,10,9};
byte colPins[numCols] = {8,7,6,5};
Keypad keypad = Keypad(makeKeymap(keymap),rowPins,colPins,numRows,numCols);

//---------------- Display
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//----------------------------------

void abort(){
  time_open=0;
  time_to_open=0;
  time_sutter_was_opened=0;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 10);
  display.println("ABORTED");
  display.display();
  delay(2000);
  asm volatile ("  jmp 0");
  EMERGENCY_ABORT_CALLED=false;
}

void display_menu_auto(int time){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.print("time: ");
  display.print(time);
  display.display();
}

void display_menu_manual(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 10);
  display.print("Manual");
  display.display();
}

void display_refresh(int time){
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(50, 5);
  display.println(time);
  display.display();
}

void shutter_open(){
  digitalWrite(INTERNAL_LED_PIN,HIGH);
}

void shutter_close(){
  digitalWrite(INTERNAL_LED_PIN,LOW);
}

void shutter_auto(){
  int time_remaining=time_to_open;
  unsigned long millis_prev=0;
  shutter_open();
  if (EMERGENCY_ABORT_CALLED){abort();return;}
  while (time_remaining>=0){
    if (EMERGENCY_ABORT_CALLED){abort();return;}
    if (millis() - millis_prev >= 1000){
      millis_prev = millis();
      display_refresh(time_remaining);
      time_remaining--;}}
  shutter_close();
  display_menu_auto(time_to_open);
}


int mode_auto(){
  static char userinput[4];
  static int count = 0;
  char key= keypad.getKey();
  static int input=0;

  if (AUTO_MODE_ENABLED==false){
    time_to_open=0;
    input=0;
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(40, 10);
    display.print("Auto");
    display.display();
    AUTO_MODE_ENABLED=true;}
  
  switch (key){
    case NO_KEY:
      break;
    
    case '#':   // Enter
      userinput[count] = '\0';
      input = atoi(userinput);
      time_to_open = input;
      display_menu_auto(time_to_open);
      break;
    
    case '*':   // Delete Input
      count = 0;
      memset(userinput,0,sizeof(userinput));
      input = 0;
      time_to_open = input;
      display_menu_auto(time_to_open);
      break;
    
    default:
      if (count < 3){     // Number entered
        userinput[count] = key;
        count+=1;
        break;}
      if (count==4){   // more than 3 chars input
        break;}}
return input;
}

void mode_manual(){
  if (AUTO_MODE_ENABLED==true){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(30, 10);
    display.print("Manual");
    display.display();
    AUTO_MODE_ENABLED=false;}
}

void trigger_depressed(){
  if (AUTO_MODE_ENABLED==true && time_to_open!=0){
    shutter_auto();}
  else if (AUTO_MODE_ENABLED==true && time_to_open==0){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print("set time!!");
    display.display();
    delay(1000);
    display_menu_auto(0);}
  else if (AUTO_MODE_ENABLED==false){
    int time_next_refresh=0;
    bool close_shutter=0;
    shutter_open();
    time_sutter_was_opened=millis();
    while (!close_shutter){
      if (EMERGENCY_ABORT_CALLED){
        abort();return;}
      time_open=millis()-time_sutter_was_opened;
      time_open=time_open/1000;
      if(time_open-time_next_refresh>=1){
        display_refresh(time_open);
        time_next_refresh++;}
      if (digitalRead(TRIGGER_BUTTON_PIN)==0){
        shutter_close();
        close_shutter=true;
        delay(200); // for debouncing --- could be replaced with millis(), but not urgently needed due to small duration 
        display_menu_manual();}}}
}

void emergency_abort(){
  shutter_close();
  EMERGENCY_ABORT_CALLED=true;
}

//----------------------------------

void setup() {
  pinMode(INTERNAL_LED_PIN,OUTPUT);
  pinMode(MODE_SWITCH_PIN,INPUT_PULLUP);
  pinMode(TRIGGER_BUTTON_PIN,INPUT_PULLUP);
  pinMode(EMERGENCY_ABORT_PIN,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(EMERGENCY_ABORT_PIN), emergency_abort, LOW);
  
  EMERGENCY_ABORT_CALLED=false;
  time_open = 0;
  time_to_open = 0;
  time_sutter_was_opened = 0;

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  if (digitalRead(MODE_SWITCH_PIN)==0){
    AUTO_MODE_ENABLED=true;
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(40, 10);
    display.print("Auto");
    display.display();}
  else{
    AUTO_MODE_ENABLED=false;
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(30, 10);
    display.print("Manual");
    display.display();}
}

void loop() {
  digitalRead(MODE_SWITCH_PIN)==0 ? mode_auto():0;
  digitalRead(MODE_SWITCH_PIN)==1 ? mode_manual():void();
  if (digitalRead(TRIGGER_BUTTON_PIN)==0){
    delay(200); // for debouncing --- could be replaced with millis(), but not urgently needed due to small duration
    trigger_depressed();}
  if (EMERGENCY_ABORT_CALLED){
    //shutter_close();
    abort();}
}