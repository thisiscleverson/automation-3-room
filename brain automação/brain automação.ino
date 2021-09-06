
//biblioteca

#include "dht.h"
#include "Wire.h"
#include "EEPROM.h"
#include "IRremote.h"
#include "SoftwareSerial.h"
#include "LiquidCrystal_I2C.h"

//definições

dht my_dht;                       // sensor dht11
SoftwareSerial bluetooth(7, 8);   // bluetooth [ pino 7 [RX] | pin 8 [TX]]
LiquidCrystal_I2C lcd(0x27, 16, 2); // lcd


//portas digitais

#define relay1 2
#define relay2 7
#define led_display 5
#define buzzer 5
#define SensorIR 3

// booleana
bool turnOnLDR = true;

// byte

byte decrease_glare = 255;

//função

int DisplayBrightness(byte adjustBrightness, bool use_fit);
int sensorLDR(bool allowSensor);

byte relay(byte whatRelay, bool stade);

//IRremote
IRrecv irrecv(SensorIR); // configuração do pino do sensor IR
decode_results sensorir; // decodificador do sensor

void setup() {

  Wire.begin();          // iniciar comunicação i2c
  lcd.begin(12, 2);      // start communication with lcd
  Serial.begin(9600);    // iniciar comunicação serial
  bluetooth.begin(9600); // iniciar comunicação bluetooth

  // definições de porte digitais <----------------------->

  pinMode(buzzer, OUTPUT);     // buzer
  pinMode(led_display, OUTPUT); // brilho do display

  pinMode(relay1, OUTPUT); // RELAY 1
  pinMode(relay2, OUTPUT); // RELAY 2

  // display-config

  DisplayBrightness(255, false);

  irrecv.enableIRIn(); //ativar o recptor iR

  Serial.print("hello world!");

}


void loop() {

  sensorLDR(turnOnLDR);
  commands_ir();
}

void commands_ir() {

  static bool turn_on_display = true;
  static byte keyTime = 0;

  keyTime++;
  
  if (irrecv.decode(&sensorir)) {
    Serial.println(sensorir.value, DEC);

   if(keyTime >= 5){
    switch (sensorir.value) {

      case 9249: // brilho [+]
        if (decrease_glare < 255) {
          decrease_glare = decrease_glare + 5;
          DisplayBrightness(decrease_glare, false);
        }
        break;

      case 25633: // brilho [-]

        if (decrease_glare > 0) {
          decrease_glare = decrease_glare - 5;
          DisplayBrightness(decrease_glare, false);
        }
        break;

      case 697376: // brilho [on/off]
        turn_on_display = !turn_on_display;

        if (turn_on_display == true) { // ligar o display
          DisplayBrightness(255, false);
        }
        else if (turn_on_display == false) { // desligar display
          DisplayBrightness(0, false);
        }
        break;

      case 4: // luz on

        break;

      case 5: // luz off

        break;

      case 6: // relay on

        break;

      case 7: // relay off

        break;

      case 8: // arcodicionado

        break;

      case 10: // mostrar dia da semana

        break;

      case 11: // next

        break;

      case 12: // before

        break;

    }
    keyTime = 0;    
    irrecv.resume();
  }
 }
}

int sensorLDR(bool allowSensor) {

  if (allowSensor) { // permitir o uso do ldr
    int statusLDR = analogRead(A0);
    //Serial.println(statusLDR);
    delay(1);

    if (statusLDR >= 1019) { // verificar se é menor que noventa
      DisplayBrightness(127, true);
    }
    else if (statusLDR <= 1019) {
      DisplayBrightness(255, true);
    }

  }
}

int DisplayBrightness(byte adjustBrightness, bool use_fit) {

  if (adjustBrightness < 255) { // ajustar brilho com valores da variavel
    analogWrite(led_display, adjustBrightness);

    if (use_fit == false) { // verificar se o ajuste de brilho automatico não esta em uso
      turnOnLDR = false; // desativar sensor ldr
    }
  }
  else if (adjustBrightness > 0) {
    analogWrite(led_display, adjustBrightness);
  }

  if (adjustBrightness >= 255) { // ligar display
    lcd.display();
    lcd.backlight();
    turnOnLDR = true; // ativar sensor ldr
    //turn_on_display = true;
  }
  else if (adjustBrightness <= 0) { // desligar display
    lcd.noDisplay();
    lcd.noBacklight();
    //turn_on_display = false;
  }
}


byte relay(byte whatRelay, bool stade) {

  if (whatRelay == 1) {
    digitalWrite(relay1, stade);

  } else if (whatRelay == 2) {
    digitalWrite(relay1, stade);
  }

}

