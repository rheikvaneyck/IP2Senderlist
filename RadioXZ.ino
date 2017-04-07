#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#define DS3231_ADDRESS 0x68
#define LCD_ADDRESS 0x27
#define BACKLIGHT_PIN 13
#define MAX_ROT_VALUE 16

volatile boolean TurnDetected;
volatile boolean up;

const int PinCLK=7;                   // Used for generating interrupts using CLK signal
const int PinDT=6;                    // Used for reading DT signal
const int PinSW=5;                    // Used for the push button switch

boolean knock = false;

// Set the LCD I2C address
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Creat a set of new characters
const uint8_t charBitmap[][8] = {
   { 0x04,0x04,0x0E,0x0E,0x1F,0x1F,0x1F,0x00 },
   { 0x00,0x00,0x00,0x00,0x00,0x1F,0x00,0x00 }
 };

void isr ()  { 
  // Interrupt service routine is executed when a 
  // HIGH to LOW transition is detected on CLK
  if (digitalRead(PinCLK))
    up = !digitalRead(PinDT);
  else
    up = digitalRead(PinDT);
  TurnDetected = true;
}

struct radio_station {
  char freq[6];
  char rds[17];
};

void init_eeprom(boolean reset=false) {
  EEPROM.update(0,'I');
  if(reset) EEPROM.update(1,0);
}

void show_eeprom() {
  radio_station station;
  int size_of_station = sizeof(station);
  int count = EEPROM.read(1);
  Serial.print(count);Serial.print(":");
  for(int i=0;i<EEPROM.read(1);i++) {
    EEPROM.get(2+i*size_of_station, station);
    Serial.print(station.freq);Serial.print(';');Serial.println(station.rds);
  }
} 

void get_station(byte nr, struct radio_station &station) {
  byte size_of_station = sizeof(station);
  byte count = EEPROM.read(1);
  if (nr>count) nr = count;
  if (nr<1) nr = 1;
  EEPROM.get(2+(nr-1)*size_of_station, station);
}  

void set_station(float frequency) {
  uint16_t frequencyB=4*(frequency*1000000+225000)/32768; //calculating PLL word
  uint8_t frequencyH=frequencyB>>8;
  uint8_t frequencyL=frequencyB&0xFF;
  delay(100);
  Wire.beginTransmission(0x60);   //writing TEA5767
  Wire.write(frequencyH);
  Wire.write(frequencyL);
  Wire.write(0xB0);
  Wire.write(0x10);
  Wire.write(0x00);
  Wire.endTransmission();
}

void setup() {
  int charBitmapSize = (sizeof(charBitmap ) / sizeof (charBitmap[0]));

  pinMode(PinCLK,INPUT);
  pinMode(PinDT,INPUT);  
  pinMode(PinSW,INPUT); 
  attachInterrupt (4,isr,FALLING);   // interrupt 0 is always connected to pin 2 on Arduino UNO

  init_eeprom();
  // initialize the lcd
  lcd.init();
  lcd.backlight();
  for ( int i = 0; i < charBitmapSize; i++ )
   {
      lcd.createChar ( i, (uint8_t *)charBitmap[i] );
   }
  lcd.home();                
  lcd.print(" Radio FMX");
  lcd.setCursor(0,1);
  lcd.print(char(0));
  for (int i=0;i<15;i++) {
    lcd.print(char(1));
  }
  Serial.begin(38400);
//  while(!Serial) {
//   ;
//  }
  Serial.println("RadioXZ"); 
}

void loop() {
  static int virtualPosition=0;    // without STATIC it does not count correctly!!!
  radio_station station;
  
  if (TurnDetected)  {		    // do this only if rotation was detected
    lcd.setCursor(virtualPosition, 1);
    lcd.print(char(1));
    if (up) {
      virtualPosition = (virtualPosition + 1) % MAX_ROT_VALUE;
    } else {     
      virtualPosition--;
      if (virtualPosition < 0) virtualPosition = MAX_ROT_VALUE - 1;
    }
    // do NOT repeat IF loop until new rotation detected
    TurnDetected = false;  
    lcd.setCursor(virtualPosition, 1);
    lcd.print(char(0));    
    Serial.print ("Count = ");  
    Serial.println (virtualPosition);
    get_station(virtualPosition+1, station);
    lcd.home();
    lcd.print(virtualPosition+1);
    lcd.print(" ");
    lcd.print(station.rds);
    lcd.print(" ");
    lcd.print(station.freq);
    lcd.print("                ");
    Serial.println(station.rds);
    String frequency = String(station.freq);
    set_station(frequency.toFloat());
  }  
  
  if(Serial.available()>0) {
    String line = String(Serial.readString());
    switch(line.charAt(0)) {
      case 'k': Serial.println("OK");
        knock = true;
        break;
      case 'r': Serial.println("RESET");
        init_eeprom(true);
        knock = false;
        break;
      case 's': show_eeprom();
        break;
      default:
        if(knock==true) {
          int idx = line.indexOf(';');
          if(idx != -1) {
            String f = String(line.substring(0,idx));
            f.toCharArray(station.freq,6);
            station.freq[f.length()] = 0;
            String r = String(line.substring(idx+1));
            r.toCharArray(station.rds, 16);
            station.rds[r.length()] = 0;
            // Serial.println(station.freq);
            // Serial.println(station.rds);
            byte counter = EEPROM.read(1);
            EEPROM.put(2+(counter*sizeof(station)), station);
            delay(500);
            counter = (counter % 16) + 1;
            EEPROM.write(1,counter);
            Serial.println("OK");  
          }
        }
        break;
    }
  } 
}
