#include <EEPROM.h>
#include <SPI.h> 
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// constants won't change. They're used here to 
// set pin numbers:
const int TransbrakePIN = 2;     // the number of the pushbutton pin Transbrake Button
//const int ledPin1 =  13;      // the number of the LED pin Tranbrake
const int RevoPIN =  3;      // the number of the LED pin NOS



// variables will change:
int keyPress;                  // LCD Button
int buttonState = LOW;         // variable for reading the pushbutton status
int test;
        //


int ledState = LOW;             // ledState used to set the LED
int n = 0;                    // indikator ob delay durchgelaufen
long previousMillis = 0;        // will store last time LED was updated
long interval = 7000;           // LCD blink geschwindigkeit

volatile int nosactive = 0; // nos status fÃ¼r interupt

unsigned long lastDelay = 0;
unsigned long lastNOS = 0;
unsigned long vNOS = 0;

unsigned long mDelay;
unsigned long vDelay;
unsigned long NOS = 1000;    // default wert  
unsigned long Delay = 500;  // default wert
int x = 0;

float RetardCurve = 1.5 ; //Retard kurve 1.0 bis 3.0 per 50 ps NOS
float Retard = 1.5 ; // aktuell gesetztes retard
int retard;
int RetardEingang;
int WiederstandRAW;
int WiederstandRAW_Old = 0;
int MaxHP =600; // Maximale Leistung bei eingang 5V muss auf den flow angepasst werden 
// Rolling average
#define filterSamples   9              // filterSamples should  be an odd number, no smaller than 3
int sensSmoothArray1 [filterSamples];   // array for holding raw sensor values for sensor1 

// spi port für wiederstand
const int csPin = 10;


void setup() {
  // initialize the LED pin as an output:
Serial.begin(9600);
//  pinMode(ledPin1, OUTPUT);     // Transbarke Indikator nicht genutzt
  pinMode(RevoPIN, OUTPUT);    // Nos Aktiv
  
  
  // initialize the pushbutton pin as an input:
  pinMode(TransbrakePIN, INPUT);     //transbrake
  
 //NOS ausgang ausschalten
  digitalWrite(RevoPIN, HIGH); 
  
// SPI Wiederstand setup
 SPI.begin();
 SPI.setBitOrder(MSBFIRST); //We know this from the Data Sheet

 pinMode(csPin,OUTPUT);
 digitalWrite(csPin, HIGH);
 digitalPotWrite(0,0); //  Retard sicher ausschalten Wiederstandswert setzen  48.82 Ohm pro einheit







 // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
   lcd.setCursor(0, 0);
    lcd.print("       NOS      ");
    lcd.setCursor(0, 1);
    lcd.print("   Controller   ");
  delay(5000);
 
   buttonState = digitalRead(TransbrakePIN); // abfrage beim einschalten 

// default werte herstellen beim einschalten

   if (buttonState == HIGH) {    
     
   
  
    writemem ();  // default werte in eeprom speichern
       
 lcd.setCursor(0, 0);
  lcd.print("Reset Default   ");  
  delay(5000);
 
  do
    {
     buttonState = digitalRead(TransbrakePIN);
     lcd.setCursor(0, 0);
     lcd.print("Release Button  "); 
    } 
    while (buttonState == HIGH);
}
  lcd.clear();

   readmem (); // eeprom lesen
  
 lcd.setCursor(0, 0);
delay(1000); 

 
nosactive = 0; // nos sicher deaktivieren
digitalPotWrite(0,0); //  Retard sicher ausschalten Wiederstandswert setzen  48.82 Ohm pro einheit 
}
void loop(){
   
   unsigned long currentMillis = millis();
  
   if(currentMillis - previousMillis > interval) {
     // save the last time you blinked the LCD 
     previousMillis = currentMillis;   

     // if the LED is off turn it on and vice-versa:
     if (ledState == LOW){
       ledState = HIGH;
       lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("delay:");
    lcd.setCursor(7, 0);
    lcd.print(Delay);
    lcd.setCursor(11, 0);
    lcd.print("ms");
    lcd.setCursor(0, 1);
    keyPress = analogRead(0); 
    lcd.print("duration:");
    lcd.setCursor(10, 1);
    lcd.print(NOS);
    lcd.setCursor(14, 1);
    lcd.print("ms");
     }
     else
     {
       ledState = LOW;
       lcd.clear();
    lcd.setCursor(0, 0);
    keyPress = analogRead(0); 
    lcd.print("Retard/50HP ");
    lcd.print(RetardCurve);
    lcd.setCursor(0, 1);
    lcd.print("Max HP ");
    lcd.print(MaxHP);
    lcd.print(" PS");
     }
 
   }
   
    buttonState = digitalRead(TransbrakePIN); // abfrage transbrake
    if ( buttonState == HIGH ) {
      //digitalWrite(ledPin1, HIGH);
  
     lcd.setCursor(0, 0);
        lcd.print("   Transbrake   ");
        lcd.setCursor(0, 1);
        lcd.print("     active     ");
}
   
   
    else
  {
  //digitalWrite(ledPin1, LOW);
}
    keyPress = analogRead(0); 
if (keyPress < 600 && keyPress > 400 ) {nosactive = 1;}
  
 buttonState = digitalRead(TransbrakePIN); // abfrage ob transbrake gedrückt


// Abfrage der steigenden flanke des Transbrake Buttons
   if (buttonState == HIGH && x==0 ) {  
     delay (1000);
     x = 1; // steigende Flanke dedektiert
     nosactive = 0; // nos timer sicher ausgeschaltet
     }
     
// Abfrage der fallenden Flanke des Transbrake Buttons     
if (buttonState == LOW && x==1 ) { 
  nosactive = 1; // nos timer einschalten
  x=0; //Flanken dedektierung zurücksetzen
  }
  

 //nos starten ------------------------------------------------------------------------------------------------------------------------------------------------
 if (nosactive == 1) {
   mDelay = micros(); // zeit speichern am anfang der warteschleife
   lastDelay = mDelay;
   
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("duration:");
   lcd.setCursor(10, 0);
    lcd.print(NOS); // ausgabe zeit 
    lcd.setCursor(14, 0);
    lcd.print("ms");
   lcd.setCursor(0, 1);
   lcd.print("RetardBase:");
   lcd.setCursor(11, 1);
    lcd.print(RetardCurve);
   do {
 
    
    

    
    
             
 mDelay = micros();           // MicrosekundenzÃ¤hler auslesen
 vDelay = mDelay - lastDelay;  // Differenz zum letzten Durchlauf berechnen
  
   if (vDelay > Delay * 1000 && n == 0) { 
    digitalWrite(RevoPIN, LOW);
 
 
lastNOS = mDelay ;
 n = 1;
  }
   vNOS = mDelay - lastNOS;
   
  if (vNOS > NOS * 1000 && n == 1){ 
    digitalWrite(RevoPIN, HIGH);  // nos dauer
    digitalPotWrite(0,0); //  Retard ausschalten Wiederstandswert setzen  48.82 Ohm pro einheit
    nosactive = 0;
     n = 0 ;
     
      }
 buttonState = digitalRead(TransbrakePIN); // abfrage während des laufes
   
if (buttonState == HIGH && x==0 ) { // abbruch kriterium und neustart
  digitalWrite(RevoPIN, HIGH);
  digitalPotWrite(0,0); //  Retard ausschalten Wiederstandswert setzen  48.82 Ohm pro einheit
  nosactive = 0;
  n=0;
  x=1;
 }
 //---------------------------------------------------------------------
 
 retardFormel ();
 
 //-------------------------------------------------------------------
 }
     while (nosactive == 1);  
    
    
   Serial.println(test);  
   test = 0;
  delay (500); //Blocken nach run    
  
  lcd.clear();
        
      
      


 }
   else {nosactive = 0;
   digitalPotWrite(0,0); //  Retard ausschalten Wiederstandswert setzen  48.82 Ohm pro einheit 
 }
nosactive = 0;
digitalPotWrite(0,0); //  Retard ausschalten Wiederstandswert setzen  48.82 Ohm pro einheit 


 // display button abfrage setup routine einschalten ------------------------------------------------
 
   keyPress = analogRead(0); 


 if(keyPress < 873 && keyPress > 603){
     
  
   
  do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 );
    lcd.clear();
   // setup start delay  ------------------------------------------------------------------------------------
      do
{
      lcd.setCursor(0, 0);
      lcd.print("Setup Delay    ");
      lcd.setCursor(0, 1);
      lcd.print(Delay);
      lcd.print("ms        ");
      
      keyPress = analogRead(0);
      // up
   if(keyPress < 221 && keyPress > 66 ){
        Delay = Delay + 100;
        if (Delay >= 10000){Delay = 10000;}
     // Teste entprellen
     do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 ); 
  //------------------------------ 
    }
      
         // down
  if(keyPress < 395 && keyPress > 230 ){
        Delay = Delay - 100;
        if (Delay <= 0){Delay = 0;}
    if (Delay >= 10000){Delay = 0;  
    }
  // Teste entprellen
       do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 ); 
  //------------------------------
    }
     
      
      
      } while (keyPress > 50 ); // rechte taste abfragen
   // Teste entprellen    
       do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 );
  
   //------------------------------
      
      // Nos dauer einstellen ------------------------------------------------------------------------------
      lcd.clear();
       do
{
      lcd.setCursor(0, 0);
      lcd.print("Setup NOS length");
      lcd.setCursor(0, 1);
      lcd.print(NOS);
      lcd.print("ms        ");
    
      
     keyPress = analogRead(0);
      // up
   if(keyPress < 221 && keyPress > 66 ){
        NOS = NOS + 100;
        if (NOS >= 20000){NOS = 20000;}
     
     
       // Teste entprellen
     do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 ); 
  //------------------------------ 
      }
     
         // down
    if(keyPress < 395 && keyPress > 230 ){
        NOS = NOS - 100;
        if (NOS <= 0){NOS = 0;}
      if (NOS >= 20000){NOS = 0;} 
   // Teste entprellen
       do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 ); 
  //------------------------------
    }
     
      
      
      } while (keyPress > 50 ); // rechte taste abfragen
   // Teste entprellen    
       do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 );
  
   //------------------------------


      // Retard einstellen ------------------------------------------------------------------------------
      lcd.clear();
       do
{
      lcd.setCursor(0, 0);
      lcd.print("Retard per 50 HP");
      lcd.setCursor(0, 1);
      lcd.print(RetardCurve);
      
    
      
     keyPress = analogRead(0);
      // up
   if(keyPress < 221 && keyPress > 66 ){
        RetardCurve = RetardCurve + 0.10;
        if (RetardCurve >= 3.00){RetardCurve = 3.00;}
     
     
       // Teste entprellen
     do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 ); 
  //------------------------------ 
      }
     
         // down
    if(keyPress < 395 && keyPress > 230 ){
        RetardCurve = RetardCurve - 0.1;
        if (RetardCurve <= 1.0){RetardCurve = 1.0;}
      if (RetardCurve >= 3.0){RetardCurve = 1.0;} 
   // Teste entprellen
       do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 ); 
  //------------------------------
    }
     
      
      
      } while (keyPress > 50 ); // rechte taste abfragen
   // Teste entprellen    
       do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 );
  
   //------------------------------       
       
       // Leistung  einstellen ------------------------------------------------------------------------------
      lcd.clear();
       do
{
      lcd.setCursor(0, 0);
      lcd.print("Setup Max HP ");
      lcd.setCursor(0, 1);
      lcd.print(MaxHP);
      lcd.print("PS        ");
    
      
     keyPress = analogRead(0);
      // up
   if(keyPress < 221 && keyPress > 66 ){
        MaxHP = MaxHP + 50;
        if (MaxHP >= 1000){NOS = 1000;}
     
     
       // Teste entprellen
     do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 ); 
  //------------------------------ 
      }
     
         // down
    if(keyPress < 395 && keyPress > 230 ){
        MaxHP = MaxHP - 50;
        if (MaxHP <= 0){MaxHP = 0;}
      if (MaxHP >= 1000){MaxHP = 0;} 
   // Teste entprellen
       do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 ); 
  //------------------------------
    }
     
      
      
      } while (keyPress > 50 ); // rechte taste abfragen
   // Teste entprellen    
       do {
     keyPress = analogRead(0); 
  }while (keyPress < 900 );
  
   //------------------------------
      
  lcd.clear();
   writemem ();   // daten speichern
 
     
    } 

  
}
void writemem () {
   
  retard = RetardCurve * 10.0;
   
// integer in byte umwandeln
  byte firstByte = byte(Delay >> 8);
  byte secondByte = byte(Delay & 0x00FF);
  byte thirdByte = byte(NOS >> 8);
  byte forthByte = byte(NOS & 0x00FF);
  byte fifthByte = byte(retard >> 8);
  byte sixthByte = byte(retard & 0x00FF);
  byte seventhByte = byte(MaxHP >> 8);
  byte eightByte = byte(MaxHP & 0x00FF);
 
    //eeprom schreiben
      EEPROM.write(0, firstByte);
      EEPROM.write(1, secondByte);
      EEPROM.write(2, thirdByte);
      EEPROM.write(3, forthByte);
      EEPROM.write(4, fifthByte);
      EEPROM.write(5, sixthByte);
      EEPROM.write(6, seventhByte);
      EEPROM.write(7, eightByte);
   
      return;
}

void readmem () {

 // eeprom lesen   
   byte  firstByte = EEPROM.read(0);
   byte  secondByte = EEPROM.read(1);
   byte  thirdByte = EEPROM.read(2);
   byte  forthByte = EEPROM.read(3);
   byte  fifthByte = EEPROM.read(4);
   byte  sixthByte = EEPROM.read(5);
   byte  seventhByte = EEPROM.read(6);
   byte  eightByte = EEPROM.read(7);
 
  //byte in integer wandeln   
   Delay = int(firstByte << 8) + int(secondByte);
   NOS = int(thirdByte << 8) + int(forthByte);
   retard = int(fifthByte << 8) + int (sixthByte);
   MaxHP = int(seventhByte << 8) + int (eightByte);
   
    RetardCurve = retard / 10.0;
    
return;
}

int digitalSmooth(int rawIn, int *sensSmoothArray){     // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k, temp, top, bottom;
  long total;
  static int i;
 // static int raw[filterSamples];
  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting              
  while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++){
      if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j+1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

/*
  for (j = 0; j < (filterSamples); j++){    // print the array to debug
    Serial.print(sorted[j]); 
    Serial.print("   "); 
  }
  Serial.println();
*/

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filterSamples * 15)  / 100), 1); 
  top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j< top; j++){
    total += sorted[j];  // total remaining indices
    k++; 
    // Serial.print(sorted[j]); 
    // Serial.print("   "); 
  }

//  Serial.println();
//  Serial.print("average = ");
//  Serial.println(total/k);
  return total / k;    // divide by number of samples
}

void digitalPotWrite(int address, int value) {
 digitalWrite(csPin, LOW); //select slave
 byte command = 0xB0; //0xB0 = 10110000 
 command += address; 
 SPI.transfer(command); 
 byte byte1 = (value >> 8);
 byte byte0 = (value & 0xFF); //0xFF = B11111111
 SPI.transfer(byte1);
 SPI.transfer(byte0);
 digitalWrite(csPin, HIGH); //de-select slave
}

void retardFormel(){

// Retart steuerung
 
  RetardEingang = digitalSmooth(analogRead(5), sensSmoothArray1) ; // einlesen und Filtern der Analogen Spannung vom REVO Controller
   //----- Berechnung des Wiederstandes und des Retards------------------------------------------------
 
   
  Retard =  ((MaxHP/(1023.0/RetardEingang)) /50.0)*RetardCurve;
  
  WiederstandRAW = Retard *1000.0 /48.828125 ;
  
  Serial.print("AnalogIn ");
  Serial.print(RetardEingang);   
  Serial.print(" Spannung ");
  Serial.print(RetardEingang*0.0049);   
  Serial.print(" retard raw ");
  Serial.print(WiederstandRAW);
  Serial.print(" Wiederstand ");
  Serial.print(WiederstandRAW*48.828125);
  Serial.print(" Retard ");
  Serial.println(Retard);
  
  //if ( (WiederstandRAW_Old - WiederstandRAW) >= 2 || (WiederstandRAW -WiederstandRAW_Old) >= 2) {
 // digitalPotWrite(0,WiederstandRAW); // Wiederstandswert setzen  48.82 Ohm pro einheit 
 // }
 // WiederstandRAW_Old = WiederstandRAW;
  digitalPotWrite(0,WiederstandRAW);
 
 test++;
 return;
 //-------------------------------------------------------------------------------------------------
 }
