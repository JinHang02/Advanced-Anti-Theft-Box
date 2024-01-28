#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD I2C address 0x27

// For the keypad and solenoid lock
const int RELAY_PIN  = A3; // connects to the IN pin of relay
const int ROW_NUM    = 4; // four rows
const int COLUMN_NUM = 4; // four columns

char keys[ROW_NUM][COLUMN_NUM] = { //Assign inputs values to list of strings
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

const String password = "1234"; // password to open safety box
const String Mode_A = "0A";  // Mode A
const String Mode_B = "0B";  // Mode B
String input_password;        //Assign this to contain the commands key-in by user

// For the pressure sensor and alarm
int weight = 0;               //variable to get values from pressure sensor
int temp = 0;                //variable to activate/deactivate alarm
int limit = 0;               //variable to set limit to activate alarm
int btn = 0;                //variable to restart whole program
int buzz = 11;              //pinOut to turn on/off alarm
int z = 0;                 //Counter for password
int k = 1;                  //counter for showing lcd
int x = 1;                  
bool stopProgram = false;    //Counter to stop/ restart the whole program

//Switch Mode A and B
int mode = 0 ;         //variable to switch Modes

void setup() {
  // For the keypad and solenoid lock
  Serial.begin(9600);
  input_password.reserve(32); // maximum input characters is 33
  pinMode(RELAY_PIN, OUTPUT); 
  digitalWrite(RELAY_PIN, LOW); // to set the solenoid lock to be locked at setup

  
  // For the pressure sensor and alarm
  pinMode(A0, INPUT);
  pinMode(12, INPUT);
  pinMode(buzz, OUTPUT);
  Serial.begin(9600);
  lcd.init();  
  lcd.backlight();          //setup lcd screen
  digitalWrite(buzz, LOW);  //to set alarm to turn off at setup
}

void loop() 
{
  //Start of two different Modes (A and B)
  //Mode A : Keypad system Online & Alarm System Offline
  //Description : This mode is ideally being used on morning/afternoon, which donators can donate money without triggering alarm and officers can retrieve money from donation box
  //              by inputing the correct password on keypad.

  while (mode == 0)
  {
  //For the keypad and solenoid lock
  char key = keypad.getKey();        
  // For the pressure sensor and alarm
  weight = analogRead(A0);       //get input from pressure sensor
  btn = digitalRead(12);
  Serial.println(weight);        //Display input from pressure sensor
  limit = weight ;   

  if (k=1) {
    lcd.setCursor(0,0);             
    lcd.print("Enter Command"); }      //Ensure lcd always display this message at start of mode A


  if (key){
    k = 0;
    lcd.setCursor(0,0);             
    lcd.print("Enter Command");
    lcd.setCursor(z,1);
    lcd.print(key);                   //Display inputs of keypad on LCD for users

    if(key == '*') {
      input_password = "";  //if '*' is inputed, the whole input of password/command is reset.
      z = 0;
      lcd.clear();
      k = 1;
    } else if(key == '#') {        //if '#' is inputed, the whole input of password/command is registered. (Act as "enter" function on PC)
      if(input_password == password ) {       // if password is correct, door will be unlocked to retrieve money
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("Password correct");   //Display message to show that door is unlocked
        lcd.setCursor(0,1);
        lcd.print("Open for 10s");
        digitalWrite(RELAY_PIN, HIGH);  // unlock the door for 20 seconds
        delay(10000);
        digitalWrite(RELAY_PIN, LOW); // lock the door after 20 seconds
      }
      else if(input_password == Mode_A) {      //if command is correct, it will be switched to Mode A
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("Switch to Mode A");         //Display message to show that system is switched to Mode A
        delay(2000);
        lcd.clear();
        k = 1;
        mode = 0;
      }
      else if(input_password == Mode_B) {      //if command is correct, it will be switched to Mode B
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("Switch to Mode B");         //Display message to show that system is switched to Mode B
        delay(2000);
        lcd.clear();
        k = 1;
        mode = 1;
      }      
      else {                             //if password/commmand is wrong, input will be reseted
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("Password/Command");     // Display message to show that password/command is wrong
        lcd.setCursor(0,1);
        lcd.print("Is Wrong!");
        delay(2000);
        lcd.clear();
        k = 1;
      }
      z = 0;
      input_password = ""; // reset the input password
      lcd.clear();
      k = 1;
    } else {
      z += 1 ;
      input_password += key; // append new character to input password string
    }
  }
  }

  ////*****************************************************************************************************************************************************************************/////
  //Mode B : Keypad system Offline & Alarm System Online
  //Description : This mode is ideally being used on evening/night, which the alarm will be triggered if there is a change of value of pressure acts on pressure sensor (suspicious
  //              theft activity) and the donation box cannot be opened through input of password (to prevent thief from opening if they steal the password).

  while (mode == 1)
  {
  //For the keypad and solenoid lock
  char key = keypad.getKey();
  // For the pressure sensor and alarm
  weight = analogRead(A0);             //get input from pressure sensor
  btn = digitalRead(12);
  Serial.println(weight);              //Display input from pressure sensor



  if ((weight < limit - 50) || (weight > limit + 50))  //Detect sudden change of weight within 0.5s
  {
    temp=1;                     //variable will be 1 to activate alarm , if value of weight changes beyond the range set.  
  } 

  if (x ==1 ){
  delay(200);                   // to give a delay of time so that limit will not equal to weight immediately
  limit = weight; }              // to compare with weight so that it can detect sudden change of weight
 
  if (temp == 1)
  {
    digitalWrite(buzz, HIGH);   //alarm is triggered
  }

  if (temp == 0)
  {
    digitalWrite(buzz, LOW);    //alarm is not triggered
  }
  
  if (btn == 1)
  {
    temp=0;
    stopProgram = true;        // variable to stop the alarm if pushbutton/reset button on Arduino is pushed
  }
  delay(100);

  if (stopProgram) {
    while (true) {
      digitalWrite(buzz, LOW);
      // Empty loop to stop the program and stop the alarm
    }}
  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Keypad input to switch mode.
  if (k=1);{
    lcd.setCursor(0,0);             
    lcd.print("Enter Command");}       //Ensure lcd always display this message at start of mode B

  if (key){
    x = 0;
    k = 0;
    lcd.setCursor(0,0);             
    lcd.print("Enter Command");
    lcd.setCursor(z,1);
    lcd.print(key);                    //Display inputs of keypad on LCD for users

    if(key == '*') {                   //if '*' is inputed, the whole input of password/command is reset.
      input_password = ""; // reset the input password
      z = 0;
      lcd.clear();
      k = 1;
    } else if(key == '#') {             //if '#' is inputed, the whole input of password/command is registered. (Act as "enter" function on PC)
      if(input_password == Mode_A ) {    //if command is correct, it will be switched to Mode A
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("Switch to Mode A");    //Display message to show that system is switched to Mode A
        delay(2000);
        lcd.clear();
        k = 1;
        x = 1;
        mode = 0;
      } 
      else if(input_password == Mode_B) {        //if command is correct, it will be switched to Mode B
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("Switch to Mode B");           //Display message to show that system is switched to Mode B
        delay(2000);
        lcd.clear();
        k = 1;
        x = 1;
        mode = 1;
      }     
      else  {                                   //if password/commmand is wrong, input will be reseted
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("Command is Wrong");         // Display message to show that password/command is wrong
        delay(2000);
        lcd.clear();
        k = 1;
        x = 1;
      }
      z = 0;
      input_password = ""; // reset the input password
      lcd.clear();
      k = 1;
      x = 1;
    } else {
      z += 1 ;
      input_password += key; // append new character to input password string
    }
  }
  }
}
