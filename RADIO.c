#include <Wire.h>
#include <EEPROM.h>

boolean knock = false;

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

void setup() {
  init_eeprom();
  Serial.begin(38400);
  Serial.println("RadioXZ"); 
}

void loop() {
  radio_station station;

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
