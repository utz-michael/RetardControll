#include <EEPROM.h>
#include <SPI.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// constants won't change. They're used here to
// set pin numbers:
const int TransbrakePIN = 2;     // the number of the pushbutton pin Transbrake Button

const int RevoPIN =  3;      // the number of the LED pin NOS



// variables will change:
int keyPress;                  // LCD Button
int buttonState = LOW;         // variable for reading the pushbutton status
int nosactive;
int x;
int i;
int i_old;
unsigned long lastDelay = 0;
unsigned long mDelay;
unsigned long vDelay;

char* Leistung[] = {"0 - 5%", "6 - 11%", "12 - 18%","19 - 24%", "25 - 30%", "31 - 36%", "37 - 42", "43 - 49%", "50 - 55%","56 - 61%","62 - 67%","68 - 73%","74 - 80%","81 - 86%","87 - 92%","93 - 100%"};

int Retard[16];

int RetardEingang;

#define filterSamples   9              // filterSamples should  be an odd number, no smaller than 3
int sensSmoothArray1 [filterSamples];   // array for holding raw sensor values for sensor1

// spi port für wiederstand
const int csPin = 10;


void setup() {
  // initialize the LED pin as an output:
  Serial.begin(9600);

  pinMode(RevoPIN, OUTPUT);    // Nos Aktiv


  // initialize the pushbutton pin as an input:
  pinMode(TransbrakePIN, INPUT);     //transbrake

  //NOS ausgang ausschalten
  digitalWrite(RevoPIN, LOW);

  // SPI Wiederstand setup
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); //We know this from the Data Sheet

  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);
  digitalPotWrite(0, 0); //  Retard sicher ausschalten Wiederstandswert setzen  48.82 Ohm pro einheit


  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("     Retard     ");
  lcd.setCursor(0, 1);
  lcd.print("   Controller   ");
  //delay(5000);



  readmem (); // eeprom lesen

  lcd.setCursor(0, 0);


  nosactive = 0; // nos sicher deaktivieren
  digitalPotWrite(0, 0); //  Retard sicher ausschalten Wiederstandswert setzen  48.82 Ohm pro einheit
}
void loop()
{


  keyPress = analogRead(0);

  buttonState = digitalRead(TransbrakePIN); // abfrage transbrake
  

  keyPress = analogRead(0);
  if (keyPress < 600 && keyPress > 400 ) {
    nosactive = 1;
  }

  buttonState = digitalRead(TransbrakePIN); // abfrage ob transbrake gedrückt
   if ( buttonState == HIGH ) {
    lcd.setCursor(0, 0);
    lcd.print("   Transbrake   ");
    lcd.setCursor(0, 1);
    lcd.print("     active     ");
  }

  // Abfrage der steigenden flanke des Transbrake Buttons
  if (buttonState == HIGH && x == 0 ) {
    delay (1000);
    x = 1; // steigende Flanke dedektiert
    nosactive = 0; // nos timer sicher ausgeschaltet
  }

  // Abfrage der fallenden Flanke des Transbrake Buttons
  if (buttonState == LOW && x == 1 ) {
    nosactive = 1; // nos timer einschalten
    x = 0; //Flanken dedektierung zurücksetzen
  }


  //nos starten ------------------------------------------------------------------------------------------------------------------------------------------------
  if (nosactive == 1) {
    mDelay = micros(); // zeit speichern am anfang der warteschleife
    lastDelay = mDelay;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nitrous Oxide   ");
    lcd.setCursor(0, 1);
    lcd.print("     on        ");
    
    do {
      digitalWrite(RevoPIN, HIGH);






      mDelay = micros();           // MicrosekundenzÃ¤hler auslesen
      vDelay = mDelay - lastDelay;  // Differenz zum letzten Durchlauf berechnen

      if (vDelay > 10000000 ) {
        digitalWrite(RevoPIN, LOW);  // nos dauer
        digitalPotWrite(0, 0); //  Retard ausschalten Wiederstandswert setzen  48.82 Ohm pro einheit
        nosactive = 0;
      }





      //---------------------------------------------------------------------

      retardFormel ();

      //-------------------------------------------------------------------
    }
    while (nosactive == 1);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nitrous Oxide   ");
    lcd.setCursor(0, 1);
    lcd.print("     off       ");
    
  }
  else {
    nosactive = 0;
    digitalPotWrite(0, 0); //  Retard ausschalten Wiederstandswert setzen  48.82 Ohm pro einheit
  }
  nosactive = 0;
  digitalPotWrite(0, 0); //  Retard ausschalten Wiederstandswert setzen  48.82 Ohm pro einheit


  // display button abfrage setup routine einschalten ------------------------------------------------

  keyPress = analogRead(0);


  if (keyPress < 873 && keyPress > 603) {



    do {
      keyPress = analogRead(0);
    } while (keyPress < 900 );
    lcd.clear();


    for (int i = 0; i <= 15; i++) {

      // Leistung   ------------------------------------------------------------------------------------
      do
      {
        lcd.setCursor(0, 0);
        lcd.print("Revo %");
        lcd.setCursor(7, 0);
        lcd.print(Leistung[i]);
        lcd.setCursor(0, 1);
        lcd.print("Retard");
        lcd.setCursor(7, 1);
        lcd.print(Retard[i]/10.0);

        keyPress = analogRead(0);
        // up
        if (keyPress < 221 && keyPress > 66 ) {
          Retard[i] = Retard[i] + 5;
          if (Retard[i] >= 200) {
          Retard[i] = 200;
        }
        // Teste entprellen
        do {
          keyPress = analogRead(0);
          } while (keyPress < 900 );
          //------------------------------
        }

      // down
      if (keyPress < 395 && keyPress > 230 ) {
          Retard[i] = Retard[i] - 5;
          if (Retard[i] <= 0) {
          Retard[i] = 0;
        }
        if (Retard[i] >= 200) {
          Retard[i] = 0;
        }
        // Teste entprellen
        do {
          keyPress = analogRead(0);
          } while (keyPress < 900 );
          //------------------------------
        }



    } while (keyPress > 50 ); // rechte taste abfragen
      // Teste entprellen
      do {
        keyPress = analogRead(0);
      } while (keyPress < 900 );

      //------------------------------

    }
    lcd.clear();
    lcd.setCursor(0, 0);
  lcd.print("     Retard     ");
  lcd.setCursor(0, 1);
  lcd.print("   Controller   ");
    writemem ();   // daten speichern


  }


}
void writemem () {
  for (int i = 0; i <= 15; i++) {
  
  byte firstByte = byte(Retard[i] >> 8);
  byte secondByte = byte(Retard[i] & 0x00FF);
  
  EEPROM.write(i, firstByte);
  EEPROM.write(i+20, secondByte);
}
  return;
}

void readmem () {
  for (int i = 0; i <= 15; i++) {
    byte  firstByte = EEPROM.read(i);
    byte  secondByte = EEPROM.read(i+20);
  Retard[i] = int(firstByte << 8) + int(secondByte);
  
}
  return;
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
  for ( j = bottom; j < top; j++) {
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

void retardFormel() {

  // Retart steuerung

  RetardEingang = digitalSmooth(analogRead(5), sensSmoothArray1) ; // einlesen und Filtern der Analogen Spannung vom REVO Controller
  //----- Berechnung des Wiederstandes und des Retards------------------------------------------------
//Serial.println(RetardEingang);
if ( RetardEingang <= 32 ) { i = 0;}
if ( 33 <= RetardEingang && RetardEingang <= 97 ) {  i = 1;}
if ( 98 <= RetardEingang && RetardEingang <= 164 ) {  i = 2;}
if ( 165 <= RetardEingang && RetardEingang <= 232 ) {  i = 3;}
if ( 233 <= RetardEingang && RetardEingang <= 299 ) {  i = 4;}
if ( 300 <= RetardEingang && RetardEingang <= 366 ) {  i = 5;}
if ( 367 <= RetardEingang && RetardEingang <= 433 ) {  i = 6;}
if ( 434 <= RetardEingang && RetardEingang <= 500 ) {  i = 7;}
if ( 501 <= RetardEingang && RetardEingang <= 565 ) {  i = 8;}
if ( 566 <= RetardEingang && RetardEingang <= 631 ) {  i = 9;}
if ( 632 <= RetardEingang && RetardEingang <= 698 ) {  i = 10;}
if ( 699 <= RetardEingang && RetardEingang <= 764 ) {  i = 11;}
if ( 765 <= RetardEingang && RetardEingang <= 832 ) {  i = 12;}
if ( 833 <= RetardEingang && RetardEingang <= 899 ) {  i = 13;}
if ( 900 <= RetardEingang && RetardEingang <= 942 ) {  i = 14;}
if ( 943 < RetardEingang ) {  i = 15;}
//Serial.println(i);
if (i != i_old){

if ( Retard[i] == 0 ){ digitalPotWrite(0,0);}
if ( Retard[i] == 5 ){ digitalPotWrite(0,7);}
if ( Retard[i] == 10 ){ digitalPotWrite(0,17);}
if ( Retard[i] == 15 ){digitalPotWrite(0,27 );}
if ( Retard[i] == 20 ){digitalPotWrite(0,36);}
if ( Retard[i] == 25 ){ digitalPotWrite(0,46);}
if ( Retard[i] == 30 ){ digitalPotWrite(0,56);}
if ( Retard[i] == 35 ){ digitalPotWrite(0,65);}
if ( Retard[i] == 40 ){ digitalPotWrite(0,75);}
if ( Retard[i] == 45 ){ digitalPotWrite(0,85);}
if ( Retard[i] == 50 ){ digitalPotWrite(0,95);}
if ( Retard[i] == 55 ){ digitalPotWrite(0,106);}
if ( Retard[i] == 60 ){ digitalPotWrite(0,116);}
if ( Retard[i] == 65 ){ digitalPotWrite(0,127);}
if ( Retard[i] == 70 ){ digitalPotWrite(0,137);}
if ( Retard[i] == 75 ){ digitalPotWrite(0,148);}
if ( Retard[i] == 80 ){ digitalPotWrite(0,160);}
if ( Retard[i] == 85 ){ digitalPotWrite(0,170);}
if ( Retard[i] == 90 ){ digitalPotWrite(0,181);}
if ( Retard[i] == 95 ){ digitalPotWrite(0,193);}
if ( Retard[i] == 100 ){ digitalPotWrite(0,205);}
if ( Retard[i] == 105 ){ digitalPotWrite(0,217);}
if ( Retard[i] == 110 ){ digitalPotWrite(0,229);}
if ( Retard[i] == 115 ){ digitalPotWrite(0,241);}
if ( Retard[i] == 120 ){ digitalPotWrite(0,253);}
if ( Retard[i] == 125 ){ digitalPotWrite(0,266);}
if ( Retard[i] == 130 ){ digitalPotWrite(0,279);}
if ( Retard[i] == 135 ){ digitalPotWrite(0,292);}
if ( Retard[i] == 140 ){ digitalPotWrite(0,305);}
if ( Retard[i] == 145 ){ digitalPotWrite(0,318);}
if ( Retard[i] == 150 ){ digitalPotWrite(0,331);}
if ( Retard[i] == 155 ){ digitalPotWrite(0,345);}
if ( Retard[i] == 160 ){ digitalPotWrite(0,359);}
if ( Retard[i] == 165 ){ digitalPotWrite(0,373);}
if ( Retard[i] == 170 ){ digitalPotWrite(0,387);}
if ( Retard[i] == 175 ){ digitalPotWrite(0,403);}
if ( Retard[i] == 180 ){ digitalPotWrite(0,418);}
if ( Retard[i] == 185 ){ digitalPotWrite(0,433);}
if ( Retard[i] == 190 ){ digitalPotWrite(0,449);}
if ( Retard[i] == 195 ){ digitalPotWrite(0,464);}
if ( Retard[i] == 200 ){ digitalPotWrite(0,480);}
}
i_old = i;
  return;
  //-------------------------------------------------------------------------------------------------
}
