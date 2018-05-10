
#include <EEPROM.h>


const int TransbrakePIN = 53;     // the number of the pushbutton pin Transbrake Button
const int RevoPIN =  51;      // the number of the LED pin NOS
const int gangPIN =  4;  
/*
const int TransbrakePIN = 2;     // the number of the pushbutton pin Transbrake Button
const int RevoPIN =  3;      // the number of the LED pin NOS
*/

int buttonState = HIGH; // variable for reading the pushbutton status

int nosactive;
int x;

int geschalten = 0;

int val = 0; //zuerst steht der Motor
char incoming[4] = {}; //wegen Endzeichen
int setup1 = 0;
int gang3sekunden;
int gang3zehntelsekunden;

//int i;
//int i_old;
unsigned long lastDelay = 0;
unsigned long mDelay;
unsigned long vDelay;
unsigned long laufzeit;
unsigned long gang;

unsigned long sicherheit = 0;

int RetardEingang;

#define filterSamples   9              // filterSamples should  be an odd number, no smaller than 3
int sensSmoothArray1 [filterSamples];   // array for holding raw sensor values for sensor1



const int RET1 = 49;
const int RET2 = 47;
const int RET3 = 45;
const int RET4 = 43;




void setup() {
  Serial.begin(9600);        // connect to the serial port 
//---------------------------------------------- Set PWM frequency for D4 & D13 ------------------------------
 
TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to     1 for PWM frequency of 62500.00 Hz
//TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  7812.50 Hz
//  TCCR0B = TCCR0B & B11111000 | B00000011;    <// set timer 0 divisor to    64 for PWM frequency of   976.56 Hz (Default)
//TCCR0B = TCCR0B & B11111000 | B00000100;    // set timer 0 divisor to   256 for PWM frequency of   244.14 Hz
//TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    61.04 Hz
  
//gang einlesen
setup_gear_timer();

gang3sekunden = EEPROM.read(0);
gang3zehntelsekunden = EEPROM.read(1);
gang = ((gang3sekunden * 10)+ gang3zehntelsekunden)*100000;
Serial.print(gang);  
  // initialize the LED pin as an output:
 laufzeit=9000000 ; 
 
  pinMode(RevoPIN, OUTPUT);    // Nos Aktiv
 


  // initialize the pushbutton pin as an input:
  pinMode(TransbrakePIN, INPUT_PULLUP);     //transbrake
 
  //NOS ausgang ausschalten
  digitalWrite(RevoPIN, LOW);
 
  // REtard Relais
  pinMode(RET1, OUTPUT);
  pinMode(RET2, OUTPUT);
  pinMode(RET3, OUTPUT);
  pinMode(RET4, OUTPUT);
  digitalWrite(RET1, LOW);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, LOW);
  analogWrite(gangPIN, (0*51)); //gang 1

  nosactive = 0; // nos sicher deaktivieren
  
}
void loop()
{
 
  buttonState = digitalRead(TransbrakePIN); // abfrage ob transbrake gedrückt
  retardFormel (); 
  // Abfrage der steigenden flanke des Transbrake Buttons
  if (buttonState == LOW  ) {
    sicherheit++;
    x = 1; // steigende Flanke dedektiert
    //nosactive = 0; // nos timer sicher ausgeschaltet
     digitalWrite(RevoPIN, HIGH);
     analogWrite(gangPIN, (0*51)); //gang 1
     
  }

  // Abfrage der fallenden Flanke des Transbrake Buttons
  if (buttonState == HIGH && x == 1 && sicherheit >= 4000) {
    
    
    nosactive = 1; // nos timer einschalten
    x = 0; //Flanken dedektierung zurücksetzen
    sicherheit = 0;
  }
  

  //nos starten ------------------------------------------------------------------------------------------------------------------------------------------------
  if (nosactive == 1) {
    mDelay = micros(); // zeit speichern am anfang der warteschleife
    lastDelay = mDelay;

    
    
    do {
      
      digitalWrite(RevoPIN, HIGH);
      analogWrite(gangPIN, (1*51)); //gang 2
      mDelay = micros();           // MicrosekundenzÃ¤hler auslesen
      vDelay = mDelay - lastDelay;  // Differenz zum letzten Durchlauf berechnen

// dritter Gang schalten //
       if (vDelay >= gang && geschalten == 0){
        analogWrite(gangPIN, (2*51)); //gang 3
        geschalten = 1;
       }




      if (vDelay >= laufzeit ) {
      
        digitalWrite(RevoPIN, LOW);  
        digitalWrite(RET1, LOW);
        digitalWrite(RET2, LOW);
        digitalWrite(RET3, LOW);
        digitalWrite(RET4, LOW);
        geschalten = 0;
        nosactive = 0;
      }
      //---------------------------------------------------------------------

      retardFormel ();

      //-------------------------------------------------------------------

 




      
    }
    while (nosactive == 1);
    
   
    
  }
  else {
    
    nosactive = 0;
    geschalten = 0;
  
  }
  
  nosactive = 0;
  geschalten = 0;
  }





int digitalSmooth(int rawIn, int *sensSmoothArray) {    // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k, temp, top, bottom;
  long total;
  static int i;
  // static int raw[filterSamples];
  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j = 0; j < filterSamples; j++) { // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting
  while (done != 1) {      // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++) {
      if (sorted[j] > sorted[j + 1]) {    // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j + 1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }
  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filterSamples * 15)  / 100), 1);
  top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j < top; j++) {
    total += sorted[j];  // total remaining indices
    k++;
  }
  
  return total / k;    // divide by number of samples
}



void retardFormel() {

  // Retart steuerung

  RetardEingang = digitalSmooth(analogRead(0), sensSmoothArray1) ; // einlesen und Filtern der Analogen Spannung vom REVO Controller
  //----- Berechnung des Wiederstandes und des Retards------------------------------------------------


if ( RetardEingang <= 164 ) { 
  digitalWrite(RET1, LOW);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, LOW);}
if ( 165 <= RetardEingang && RetardEingang <= 231 ) {
  digitalWrite(RET1, HIGH);// 2°
  digitalWrite(RET2, HIGH);// 4°
  digitalWrite(RET3, LOW);// 8°
  digitalWrite(RET4, LOW);} // 16°  19 - 24 % revo 0,97 V 80 - 120 PS
if ( 232 <= RetardEingang && RetardEingang <= 298 ) {
  digitalWrite(RET1, LOW);// 2°
  digitalWrite(RET2, LOW);// 4°
  digitalWrite(RET3, HIGH);// 8°
  digitalWrite(RET4, LOW);} // 16°  25 - 30 % revo 1,3 V 130 - 170 PS
if ( 299 <= RetardEingang && RetardEingang <= 366 ) {
  digitalWrite(RET1, HIGH);// 2°
  digitalWrite(RET2, LOW);// 4°
  digitalWrite(RET3, HIGH);// 8°
  digitalWrite(RET4, LOW);} // 16°  31 - 36 % revo 1,63 V 180 - 220 PS
if ( 367 <= RetardEingang && RetardEingang <= 433 ) {
  digitalWrite(RET1, LOW);// 2°
  digitalWrite(RET2, HIGH);// 4°
  digitalWrite(RET3, HIGH);// 8°
  digitalWrite(RET4, LOW);} // 16°  37 - 42 % revo 1,96 V 230 - 275 PS
if ( 434 <= RetardEingang && RetardEingang <= 500 ) {
  digitalWrite(RET1, LOW); //2°
  digitalWrite(RET2, HIGH); // 4°
  digitalWrite(RET3, HIGH); // 8°
  digitalWrite(RET4, LOW);} // 16°   43 - 49 % revo 2,28 V 280 - 340 PS
if ( 501 <= RetardEingang && RetardEingang <= 565 ) {
  digitalWrite(RET1, LOW); // 2°
  digitalWrite(RET2, LOW); // 4°
  digitalWrite(RET3, LOW); // 8°
  digitalWrite(RET4, HIGH);} // 16°  50 - 55 % Revo 2,61 V 350 - 390 PS
if ( 566 <= RetardEingang && RetardEingang <= 630 ) {
  digitalWrite(RET1, HIGH); // 2°
  digitalWrite(RET2, LOW); // 4°
  digitalWrite(RET3, LOW); // 8°
  digitalWrite(RET4, HIGH);} // 16°   56 - 61 % Revo 2,93 V 400 - 440 PS
if ( 631 <= RetardEingang && RetardEingang <= 1023 ) {
  digitalWrite(RET1, LOW); // 2°
  digitalWrite(RET2, HIGH); // 4°
  digitalWrite(RET3, LOW); // 8°
  digitalWrite(RET4, HIGH);} // 16°   62 - 67 % Revo 3,26 V 450 - 490 PS




  return;
  //-------------------------------------------------------------------------------------------------
}

void setup_gear_timer(){
do 
{
int i = 0;
 Serial.println(millis());
  if (Serial.available() > 0) {
    //sonst bleiben die 0 erhalten 60 -> 600
    memset(incoming, 0, sizeof(incoming));
    while (Serial.available() > 0 && i < sizeof(incoming) - 1) {
      incoming[i] = Serial.read();
      i++;
      delay(3);
    }
 
    //array of char in int wandeln
   
 
    Serial.print("Setup");
    setup1=1;
    //Serial.println(val);
    //Motorgeschwindigkeit setzen
    
  }
}while ((millis()<=2000)&& setup1 == 0);


if (setup1==1){
Serial.println("Schaltzeitpunkt 3. Gang");
gang3sekunden = EEPROM.read(0);
gang3zehntelsekunden = EEPROM.read(1);
Serial.print("Aktueller Wert: ");
Serial.print(gang3sekunden);
Serial.print(",");
Serial.print(gang3zehntelsekunden);
Serial.println("s");
Serial.print("Neuer Wert für Sekunden:");
do 
{
int i = 0;
 
  if (Serial.available() > 0) {
    //sonst bleiben die 0 erhalten 60 -> 600
    memset(incoming, 0, sizeof(incoming));
    while (Serial.available() > 0 && i < sizeof(incoming) - 1) {
      incoming[i] = Serial.read();
      i++;
      delay(3);
    }
 
    //array of char in int wandeln
    val = atoi(incoming);
    //Geschwindigkeit limitieren
    if (val < 0) {
      val = 0;
    } else if (val > 9) {
      val = 9;
    }
 
    Serial.print(val);
    gang3sekunden = val;
    EEPROM.write(0, gang3sekunden);
    setup1=3;
    //Serial.println(val);
    //Motorgeschwindigkeit setzen
    
  }
}while (setup1 != 3 );
Serial.println();
Serial.print("Neuer Wert für Zehntelsekunden:");
do 
{
int i = 0;
 
  if (Serial.available() > 0) {
    //sonst bleiben die 0 erhalten 60 -> 600
    memset(incoming, 0, sizeof(incoming));
    while (Serial.available() > 0 && i < sizeof(incoming) - 1) {
      incoming[i] = Serial.read();
      i++;
      delay(3);
    }
 
    //array of char in int wandeln
    val = atoi(incoming);
    //Geschwindigkeit limitieren
    if (val < 0) {
      val = 0;
    } else if (val > 9) {
      val = 9;
    }
 
    Serial.print(val);
    gang3zehntelsekunden = val;
    EEPROM.write(1, gang3zehntelsekunden);
    setup1=4;
    //Serial.println(val);
    //Motorgeschwindigkeit setzen
    
  }
}while (setup1 != 4 );

Serial.println();
Serial.println("Schaltzeitpunkt 3. Gang");
gang3sekunden = EEPROM.read(0);
gang3zehntelsekunden = EEPROM.read(1);
Serial.print("Aktueller Wert: ");
Serial.print(gang3sekunden);
Serial.print(",");
Serial.print(gang3zehntelsekunden);
Serial.println("s");





}






}


