
//biblioteca

#include "DHT.h"
#include "Wire.h"
#include "EEPROM.h"
#include "IRremote.h"
#include "DS1307.h"
#include "SoftwareSerial.h"
#include "LiquidCrystal_I2C.h"

//definições
                      // sensor dht11
SoftwareSerial bluetooth(7, 8);   // bluetooth [ pino 7 [RX] | pin 8 [TX]]
LiquidCrystal_I2C lcd(0x27, 16, 2); // lcd
DS1307 clock; //definir um objeto da classe DS1307 

//portas digitais

#define relay1 9
#define relay2 10
#define led_display 5
#define buzzer 6
#define SensorIR 3
#define DHTPIN 12

#define DHTTYPE DHT11 // o tipo do sensor (DHT11, DHT22, ETC...)


// booleana
bool turnOnLDR = true;          // permitir o uso do sensor ldr
bool allowNextDisplay = false; // permitir o proximo status
bool stade1 = false, stade2 = false; // estado do rele [ligado ou desligado]
bool notice = false;        // mostrar qual função foi ativada no status
bool allowSongs = true;    // ativar buzzer
bool allowTimerRelay = true; // permitir que o time fucione quando o rele for ativado

// byte
byte decrease_glare = 255; // nivel do brilho ao iniciar o programa
byte nextStatus = 0;       // proximo status
byte noticeStatus;         // verificador da nextStatus
byte timer;                // timer do rele

//millis config
unsigned long before = 0;
unsigned long interval = 550;

//função
int DisplayBrightness(byte adjustBrightness, bool use_fit); // função do brilho do display
int sensorLDR(bool allowSensor); // função do sensor LDR
int DHT11sensor(byte whatValues);
int Timer(byte zero); // contador

byte relay(byte whatRelay); // função para o rele
byte soungs(byte whatSongs); // tipos de som do buzzer

String dayOfWeek();

void setCLOCK(int year, byte month, byte day, byte hours, byte minutes, byte seconds, byte dayOfWeek);

bool timerRelay(bool remembers_the_time);

//IRremote
IRrecv irrecv(SensorIR); // configuração do pino do sensor IR
decode_results sensorir; // decodificador do sensor

//Sensor dht 
DHT dht(DHTPIN,DHTTYPE);

void setup() {

  dht.begin();           // iniciar o sensor de temperatura
  Wire.begin();          // iniciar comunicação i2c
  clock.begin();         // iniciar rtc    
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


  // 1° >> year | 2° >> month | 3° >> day | 4° >> hours | 5° >> minutes | 6° >> seconds |7° >> dayOfWeek
  //setCLOCK(2021,10,10,1,28,0,7);
  soungs(4);
}

void setCLOCK(int year, byte month, byte day, byte hours, byte minutes, byte seconds, byte dayOfWeek){

  clock.fillByYMD(year, month, day); //Jan 19,2013
  clock.fillByHMS(hours, minutes, seconds); //15:28 30"
  clock.fillDayOfWeek(dayOfWeek);//Saturday

  clock.setTime();//write time to the RTC chip
}

void loop() {

  sensorLDR(turnOnLDR);
  commands_ir();
  //RTCviews();
  //DHT11sensor();
  Display();
  timerRelay(false);
  
}

byte soungs(byte whatSongs){
  if(allowSongs == true){
      switch(whatSongs){
          case 0:          
            analogWrite(buzzer, 195);
              delay(50);
            analogWrite(buzzer, 50);
              delay(100);
            analogWrite(buzzer, 195);
              delay(50);
            analogWrite(buzzer, 0);
              delay(100);
          break;

          case 1:
            analogWrite(buzzer, 200);
            delay(20);            
          break;

          case 2:
            analogWrite(buzzer, 199);
            delay(20);                        
          break;

          case 3: // alarm
            analogWrite(buzzer,255);
            delay(190);
            analogWrite(buzzer,55);
            delay(55);            
          break;

          case 4:
            analogWrite(buzzer, 55);
              delay(50);
            analogWrite(buzzer, 90);
              delay(100);
            analogWrite(buzzer, 55);
              delay(50);
            analogWrite(buzzer, 0);
              delay(100);
          break;          

      }
      analogWrite(buzzer,0);
  }
}

void commands_ir() {

  static byte keyTime = 0;

  //keyTime++;

  if (irrecv.decode(&sensorir)) {
    Serial.println(sensorir.value, DEC);
    delay(100);

    switch (sensorir.value) {

      case 9249: // brilho [+]
        commands(0);
        soungs(2);
        break;

      case 25633: // brilho [-]
        commands(1);
        soungs(2);
        break;

      case 697376: // brilho [on/off]
        commands(2);
        soungs(1);        
        break;

      case 24609: // relay 1
        soungs(2); 
        commands(3);
        break;

      case 4129: // relay 2
        soungs(2);
        commands(4);
        break;

      case 13857: // desativar timer
        soungs(2);
        commands(5);
        break;

      case 515552: // next
        soungs(0);        
        commands(6);      
        break;

      case 500768: // audio off or on
        commands(7); 
        break;

    }
    irrecv.resume();
  }
}

void commands(int commands) {
  static bool turn_on_display = true;

  switch (commands) {


    case 0: // brilho [+]
      if (decrease_glare < 255) {
        Serial.println(decrease_glare);
        decrease_glare = decrease_glare + 15;
        DisplayBrightness(decrease_glare, false);
      }
      break;

    case 1: // brilho [-]
      if (decrease_glare > 0) {
        Serial.println(decrease_glare);
        decrease_glare = decrease_glare - 15;
        DisplayBrightness(decrease_glare, false);
      }
      break;

    case 2:
      turn_on_display = !turn_on_display;

      if (turn_on_display == true) { // ligar o display
        DisplayBrightness(255, false);
      }
      else if (turn_on_display == false) { // desligar display
        DisplayBrightness(0, false);
      }
      break;

    case 3:     
        relay(1);
        stade1 = !stade1;
        lcd.clear(); // limpar o lcd antes do mostrar o status do rele
        notice = true;
        noticeStatus = 2;
        nextStatus = 2;
        allowNextDisplay = true;      
      break;

    case 4:
        relay(2); 
        stade2 = !stade2;
        lcd.clear(); // limpar o lcd antes do mostrar o status do rele
        notice = true;
        noticeStatus = 2;
        nextStatus = 2;
        allowNextDisplay = true;      
      break;

    case 5:
        allowTimerRelay = !allowTimerRelay;
        lcd.clear(); // limpar o lcd antes do mostrar o status do rele
        notice = true;
        noticeStatus = 5;
        nextStatus = 5;
        allowNextDisplay = true;      
      break;

    case 6:        
        allowNextDisplay = true;       
      break;

    case 7:
        allowSongs = !allowSongs;
        lcd.clear(); // limpar o lcd antes do mostrar o status do rele
        notice = true;
        noticeStatus = 3;
        nextStatus = 3;
        allowNextDisplay = true;      
      break;

    case 8:

      break;

    case 9:

      break;

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

int DHT11sensor(byte whatValues){
  int h = dht.readHumidity(); // >>> Umidade 
  
  int c = dht.readTemperature(); // >>> Celsius 
  
  int f = dht.readTemperature(true); // >>> Fahrenheit

  if (isnan(h) || isnan(c) || isnan(f)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
  }

  if(whatValues == 1){
    return h;
  }
  else if (whatValues == 2){
    return c;
  }

  else if(whatValues == 3){
    return f;
  }

 

  //int hif = dht.computeHeatIndex(f, h); // computar temperatura de celsius e umidade
  //int hic = dht.computeHeatIndex(t, h, false); // converte para celsius

  /*
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F"));
  Serial.println("");
  */
  /*
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));  */
  
}

bool timerRelay(bool remembers_the_time){

  if(remembers_the_time == true && stade1 == true){
     timer =  clock.hour + 2;
     soungs(3);
  }


  if(timer == clock.hour && stade1 == true){
    soungs(3);
    stade1 = false;
    relay(1);    

    lcd.clear(); // limpar o lcd antes do mostrar o status do rele
    notice = true;
    noticeStatus = 2;
    nextStatus = 2;
    allowNextDisplay = true;    
  }

   
}

byte relay(byte whatRelay) {

  if (whatRelay == 1) {
    digitalWrite(relay1, stade1);
    timerRelay(allowTimerRelay);

  } else if (whatRelay == 2) {
    digitalWrite(relay2, stade2);
  }

}

String dayOfWeek(){
    
    if(clock.dayOfWeek == MON){
      return "MONDAY   ";
    }
    
    else if(clock.dayOfWeek == TUE){
      return "TUESDAY  ";
    }

    else if(clock.dayOfWeek == WED){
      return "WEDNESDAY";
    }

    else if(clock.dayOfWeek == THU){
      return "THURSDAY ";
    }

    else if(clock.dayOfWeek == FRI){
      return "FRIDAY   ";
    }

    else if(clock.dayOfWeek == SAT){
      return "SATURDAY ";
    }

    else if(clock.dayOfWeek == SUN){
      return "SUNDAY   ";
    }    
}

int Timer(byte zero){
  int timer = 0;

  if(zero == 0){
    timer = 0;    
    
  }else{
    delay(1);    
    return timer++;
  }      

}

void Display() {

  int h;
  int f;
  int c;
  
  if(allowNextDisplay == true){
      if((millis() / 10) - before >= interval){
         before = (millis() / 10);
         lcd.clear();        
         nextStatus++;

         if(notice == true){           
            if(nextStatus + 1 > noticeStatus){
              nextStatus = 0;
              allowNextDisplay = false;
              notice = false;
            }
         }
            //Serial.println(allowNextDisplay);     
      }

      if(nextStatus > 5){
         nextStatus = 0;
         allowNextDisplay = false;
      }
  }

  switch(nextStatus){
    case 0:
      //------------------------[hours]-------------------------------//    
      clock.getTime();
      lcd.setCursor(0,0);

      if(clock.hour < 10){
        lcd.print("0");            
      }

      lcd.print(clock.hour,DEC);
      lcd.print(":");

      if(clock.minute < 10){
        lcd.print("0");       
      }
      
      lcd.print(clock.minute, DEC); 

      //------------------------[date]-------------------------------//

      lcd.setCursor(6,0);
      
      if(clock.month < 10){
        lcd.print("0");      
      }    
      
      lcd.print(clock.month,DEC);
      lcd.print("/");
      if(clock.dayOfMonth < 10){
        lcd.print("0");
      }    
      lcd.print(clock.dayOfMonth,DEC);
      lcd.print("/");
      lcd.print(clock.year + 2000, DEC);

      //------------------------[date]-------------------------------//
      lcd.setCursor(0,1);
      lcd.print(dayOfWeek());

      //------------------------[DHT11]-------------------------------//
      lcd.setCursor(9,1);
      lcd.print("|");

      f = DHT11sensor(3);

      lcd.setCursor(11, 1);
      lcd.print(f);
      lcd.write(B11011111);
      lcd.print("F");
    break;

    case 1:
      h = DHT11sensor(1); // umidade
      c = DHT11sensor(2); // temperatura em Celsus
            
      lcd.setCursor(0,0);
      lcd.print("Celsus: ");
      lcd.print(c);
      lcd.write(B11011111);
      lcd.print("C");

      lcd.setCursor(0,1);
      lcd.print("humidity: ");
      lcd.print(h);
      lcd.print("%");                  
    break;
    
    case 2:
       lcd.setCursor(0,0);
       lcd.print("Relay 1: ");
       if(stade1 == true){
          lcd.print("ON ");         
       }
       else{
         lcd.print("OFF");
       }

       lcd.setCursor(0,1);
       lcd.print("Relay 2: ");
       if(stade2 == true){
          lcd.print("ON ");         
       }
       else{
         lcd.print("OFF");
       }       
    break;
    
    case 3:
      lcd.setCursor(0,0);
      lcd.print("audio: ");
      if(allowSongs == true){
          lcd.print("ON ");                
      }else{ lcd.print("OFF");}

      /*lcd.setCursor(0,1);
      lcd.print("Alarm: ");
      if(turnOnAlarm == true){
          lcd.print("ON ");
      }else{lcd.print("OFF");} */            
    break;

    case 4:

    lcd.setCursor(3,0);
    
    if(clock.month < 10){
      lcd.print("0");      
    }    
    
    lcd.print(clock.month,DEC);
    lcd.print("/");
    
    if(clock.dayOfMonth < 10){
      lcd.print("0");
    }    
    lcd.print(clock.dayOfMonth,DEC);
    lcd.print("/");
    lcd.print(clock.year + 2000, DEC);
    
    if(clock.dayOfMonth == 30 && clock.month == 11){ // meu aniversario
        lcd.setCursor(0,1);
        lcd.print("-> your birthday <-");   
    }
    else if(clock.dayOfMonth == 31 && clock.month == 7){ // aniversario do meu pai
        lcd.setCursor(0,1);
        lcd.print("-> dad <-");       
    }
    else if(clock.dayOfMonth == 16 && clock.month == 11){ // aniversario da minha mãe
        lcd.setCursor(0,1);
        lcd.print("-> mother <-"); 
    }
    else if(clock.dayOfMonth == 25 && clock.month == 10){ // aniversario do meu prim0
        lcd.setCursor(0,1);
        lcd.print("-> cousin <-"); 
    }
    else if(clock.dayOfMonth == 5 && clock.month == 8){ // aniversario da minha avó
        lcd.setCursor(0,1);
        lcd.print("-> grandmother <-"); 
    }
    else if(clock.dayOfMonth == 31 && clock.month == 12){ // ano novo
        lcd.setCursor(0,1);
        lcd.print("-> New Year's Day <-"); 
    }
    else if(clock.dayOfMonth == 25 && clock.month == 12){ // natal
        lcd.setCursor(0,1);
        lcd.print("-> Christmas <-"); 
    }else{
      lcd.setCursor(1,1);
      lcd.print("today nothing");                  
    }    
    break;

    case 5:
      lcd.setCursor(0,0);
      lcd.print("Timer: ");
      if(allowTimerRelay == true){
        lcd.print("ON ");
      }else{lcd.print("OFF");}

      lcd.setCursor(0,1);
      lcd.print("RelayOFF: ");
      lcd.print(timer);
      lcd.print("H");
                  
    break;
    
  }
}

