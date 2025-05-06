#include <Servo.h>
#include "SR04.h"
#include <LiquidCrystal_I2C.h>

#define pinEcho 2
#define pinTrig 3
#define pinServo 5
#define pinBottoneJoystick 6
#define pinBuzzer 12
#define pinLed 13
#define Joystick A0
#define Fotoresistore A1
//#define SDA A4
//#define SCL A5

LiquidCrystal_I2C lcd(0x27, 16, 2);                                 //inizializzazione schermo lcd
SR04 sr04 = SR04(pinEcho, pinTrig);                                 //inizializzazione SH04
Servo servo;                                                        //inizializzazione Servo
long distanza;                                                      //variabile distanza dell'SH04
unsigned long Millisecondi_LCD = 0;                                 //variabile che misura i millisecondi per il conteggio dell'LCD
const long intervallo_LCD = 500;                                    //intervallo nell'aggiornamento dell'LCD
unsigned long tempo_attuale_LCD = 0;                                //variabile che contiene i millisecondi relativi all'LCD     
unsigned long rimbalzo = 0;                                         //variabile che misura i millisecondi per il conteggio del servo
unsigned long intervallo_servo = 0;                                 //crea la variabile che misura l'intervallo di tempo del servo
unsigned long millisecondi_servo = 0;                               //variabile che registra i millisecondi per l'aggiornamento del servomotore
int ritardo_servo = 15;                                             //variabile che ha salvato il delay del servomotore
int stato = HIGH;                                                   //Lo stato del pulsante
int pos;                                                            //la posizione del servomotore in modalità manuale
int Angolo_servo = 0;                                               //angolo in cui il servo si muove 
int stepServo = 1;                                                  //Passo del movimento automatico
int lettura;                                                        //variabile che legge la pressione del pulsante
int ritardo=50;                                                     //variabile che gestisce il rimbalzo del bottone
int luce;                                                           //variabile che salva la luminosità dell'ambiente

void segnalaDistanza(long distanza){
  lcd.setCursor(0, 1);
  if(distanza>10){                                                   //se la distanza >=10, allora non accade nulla
    lcd.print("Via Libera          ");
    digitalWrite(pinBuzzer, LOW);
  }
  else{                                                              //altrimenti accendi il buzzer e stampa un messaggio
    lcd.print("Ostacolo Vicino!    ");
    digitalWrite(pinBuzzer, HIGH);
  }
}

void stampaDistanza(){
  tempo_attuale_LCD = millis();                                     //intervallo di tempo del motore
  if(tempo_attuale_LCD - Millisecondi_LCD >= intervallo_LCD){       //se il tempo trascorso meno i millisecondi impostati superano l'intervallo, allora aggiorna lo schermo LCD
    Millisecondi_LCD = tempo_attuale_LCD;                           //aggiorna il tempo dello schermo LCD
    distanza = sr04.Distance();                                     //rileva la distanza del sensore
    lcd.print("Distanza: ");                                        //operazioni di output sullo schermo LCD
    lcd.setCursor(0, 0);
    lcd.setCursor(10, 0);
    lcd.print("      ");
    lcd.setCursor(10, 0);
    lcd.print(distanza);
    lcd.print("cm");
    segnalaDistanza(distanza);
    lcd.setCursor(0, 2);
    if(stato==HIGH){
      lcd.print("Modalita' Manuale   ");
    }
    else{
      lcd.print("Modalita' Automatico");
    }
    lcd.setCursor(0, 3);
    lcd.print("                    ");
  }
}

void motoreAutomatico() {
  millisecondi_servo = millis();                                    //registra il tempo relativo al servomotore
  if(millisecondi_servo - intervallo_servo >= ritardo_servo){       //se il tempo trascorso meno i millisecondi impostati superano l'intervallo, allora aggiorna la posizione del motore
    intervallo_servo = millisecondi_servo;                          //aggiorna il tempo del servo
    Angolo_servo += stepServo;                                      //aggiorna l'angolo del servo
    if(Angolo_servo >= 180 || Angolo_servo <= 0){                   //se l'angolo supera i limiti del motore, allora invertili per fare ripartire il motore
      stepServo = -stepServo;
    }
  servo.write(Angolo_servo);                                        //ruota il servo all'angolo impostato
  stampaDistanza();                                                //stampa a video la distanza ogni aggiornamento di posizione
  }
}

void controllaMotore() {
  pos = map(analogRead(Joystick), 0, 1023, 0, 180);                 //la variabile posizione può assumere solo valori da 0 a 180, quindi bisogna convertire la lettura analogica del jostick in quel valore usando questa funzione
  servo.write(pos);                                                 //ruota il servo all'angolo impostato
  stampaDistanza();                                                //stampa a video la distanza ogni aggiornamento di posizion
}

void rilevaLuce(){
  luce = map(analogRead(Fotoresistore), 0, 1023, 0, 100);           //rileva e converte la luce nella stanza
  if(luce < 50){                                                    //se la luce è maggiore di 50, allora attiva il led
    digitalWrite(pinLed, HIGH);
  }
  else{                                                             //altrimenti spegnilo
    digitalWrite(pinLed, LOW);
  }
}

void attivaLedAccensione(){                                         //funzione che attiva il led durante l'accensione
  for(int i=0; i<3; i++){                                           //il ciclo for lo viene ripetuto 3 volte
    digitalWrite(pinLed, HIGH);                                     //accendi il led per 1 secondo e poi spegnilo 
    delay(1000);
    digitalWrite(pinLed, LOW);
    delay(1000);
  }
}

void setup() {
  lcd.init();                                                      //inizializza lo schermo LCD
  lcd.begin(20, 4);
  lcd.backlight();                                                 
  pinMode(pinBuzzer, OUTPUT);                                      //imposta i pin digitali in base alla loro funzione
  pinMode(pinBottoneJoystick, INPUT_PULLUP);
  pinMode(10, OUTPUT);
  servo.attach(pinServo);
  attivaLedAccensione();                                           //attiva il led per segnalare l'accensione del radar
}

void loop() {
  lettura = digitalRead(pinBottoneJoystick);                        //legge lo stato del pulstante del joystick
  if(lettura == LOW && (millis() - rimbalzo > ritardo)){            //se rileva un cambio di stato e registra un ritardo maggiore 
    stato = !stato;                                                 //allora inverti lo stato
    rimbalzo = millis();                                            //e reimposta il rimbalzo
  }
  if (stato == LOW) {                                               //se lo stato è LOW, avvia il motore automatica
    motoreAutomatico();
  } else {                                                          //altrimenti attiva la modalità manuale
    controllaMotore();
  }
  rilevaLuce();                                                     //rileva la luce nella stanza
}
