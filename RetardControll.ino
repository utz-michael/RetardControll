
const int TransbrakePIN = 53;     // the number of the pushbutton pin Transbrake Button
const int RevoPIN =  51;      // the number of the LED pin NOS

/*
const int TransbrakePIN = 2;     // the number of the pushbutton pin Transbrake Button
const int RevoPIN =  3;      // the number of the LED pin NOS
*/

int buttonState = HIGH; // variable for reading the pushbutton status

int nosactive;
int x;
//int i;
//int i_old;
unsigned long lastDelay = 0;
unsigned long mDelay;
unsigned long vDelay;
unsigned long laufzeit;

unsigned long sicherheit = 0;

int RetardEingang;

#define filterSamples   9              // filterSamples should  be an odd number, no smaller than 3
int sensSmoothArray1 [filterSamples];   // array for holding raw sensor values for sensor1

// spi port für wiederstand
/*
const int RET1 = 10;
const int RET2 = 11;
const int RET3 = 12;
const int RET4 = 13;
*/

const int RET1 = 49;
const int RET2 = 47;
const int RET3 = 45;
const int RET4 = 43;

void setup() {
  
 
  
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


  nosactive = 0; // nos sicher deaktivieren
  
}
void loop()
{
 
  buttonState = digitalRead(TransbrakePIN); // abfrage ob transbrake gedrückt
   
  // Abfrage der steigenden flanke des Transbrake Buttons
  if (buttonState == LOW  ) {
    sicherheit++;
    x = 1; // steigende Flanke dedektiert
    //nosactive = 0; // nos timer sicher ausgeschaltet
     digitalWrite(RevoPIN, HIGH);
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
      mDelay = micros();           // MicrosekundenzÃ¤hler auslesen
      vDelay = mDelay - lastDelay;  // Differenz zum letzten Durchlauf berechnen

      if (vDelay >= laufzeit ) {
      
        digitalWrite(RevoPIN, LOW);  
        digitalWrite(RET1, LOW);
        digitalWrite(RET2, LOW);
        digitalWrite(RET3, LOW);
        digitalWrite(RET4, LOW);
        nosactive = 0;
      }
      //---------------------------------------------------------------------

      retardFormel ();

      //-------------------------------------------------------------------

   if (buttonState == LOW  ) {
    sicherheit++;
    x = 1; // steigende Flanke dedektiert
    //nosactive = 0; // nos timer sicher ausgeschaltet
     digitalWrite(RevoPIN, HIGH);
     }

     // Abfrage der fallenden Flanke des Transbrake Buttons
     if (buttonState == HIGH && x == 1 && sicherheit >= 4000) {
    
      nosactive = 1; // nos timer einschalten
      x = 0; //Flanken dedektierung zurücksetzen
     sicherheit = 0;
     mDelay = micros(); // zeit speichern am anfang der warteschleife
     lastDelay = mDelay;
      }




      
    }
    while (nosactive == 1);
    
   
    
  }
  else {
    nosactive = 0;
  digitalWrite(RET1, LOW);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, LOW);
  }
  nosactive = 0;
  digitalWrite(RET1, LOW);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, LOW);
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


if ( RetardEingang <= 366 ) { 
  digitalWrite(RET1, LOW);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, LOW);}
if ( 367 <= RetardEingang && RetardEingang <= 433 ) {
  digitalWrite(RET1, HIGH);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, LOW);} // 37 - 42 % revo 1,96 V 230 - 275 PS
if ( 434 <= RetardEingang && RetardEingang <= 500 ) {
  digitalWrite(RET1, LOW);
  digitalWrite(RET2, HIGH);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, LOW);} // 43 - 49 % revo 2,28 V 280 - 340 PS
if ( 501 <= RetardEingang && RetardEingang <= 565 ) {
  digitalWrite(RET1, LOW);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, HIGH);
  digitalWrite(RET4, LOW);} // 50 - 55 % Revo 2,61 V 350 - 390 PS
if ( 566 <= RetardEingang && RetardEingang <= 1023 ) {
  digitalWrite(RET1, LOW);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, HIGH);} // 56 - 61 % Revo 2,93 V 400 - 440 PS




/*
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

if ( Retard[i] == 0 ){ digitalWrite(RET1, LOW);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, LOW);}
if ( Retard[i] == 15 ){digitalWrite(RET1, HIGH);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, LOW);}
if ( Retard[i] == 30 ){ digitalWrite(RET1, LOW);
  digitalWrite(RET2, HIGH);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, LOW);}
if ( Retard[i] == 45 ){digitalWrite(RET1, HIGH);
  digitalWrite(RET2, HIGH);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, LOW);}
if ( Retard[i] == 60 ){digitalWrite(RET1, LOW);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, HIGH);
  digitalWrite(RET4, LOW);}
if ( Retard[i] == 75 ){ digitalWrite(RET1, HIGH);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, HIGH);
  digitalWrite(RET4, LOW);}
if ( Retard[i] == 90 ){ digitalWrite(RET1, LOW);
  digitalWrite(RET2, HIGH);
  digitalWrite(RET3, HIGH);
  digitalWrite(RET4, LOW);}
if ( Retard[i] == 105 ){ digitalWrite(RET1, HIGH);
  digitalWrite(RET2, HIGH);
  digitalWrite(RET3, HIGH);
  digitalWrite(RET4, LOW);}
if ( Retard[i] == 120 ){ digitalWrite(RET1, LOW);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, HIGH);}
if ( Retard[i] == 135){ digitalWrite(RET1, HIGH);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, HIGH);}
if ( Retard[i] == 150 ){ digitalWrite(RET1, LOW);
  digitalWrite(RET2, HIGH);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, HIGH);}
if ( Retard[i] == 165 ){ digitalWrite(RET1, HIGH);
  digitalWrite(RET2, HIGH);
  digitalWrite(RET3, LOW);
  digitalWrite(RET4, HIGH);}
if ( Retard[i] == 180 ){ digitalWrite(RET1, LOW);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, HIGH);
  digitalWrite(RET4, HIGH);}
if ( Retard[i] == 195 ){ digitalWrite(RET1, HIGH);
  digitalWrite(RET2, LOW);
  digitalWrite(RET3, HIGH);
  digitalWrite(RET4, HIGH);}

*/
  return;
  //-------------------------------------------------------------------------------------------------
}
