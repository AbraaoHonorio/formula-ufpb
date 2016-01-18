#include <TinyGPS++.h>
#include <Wire.h>
#include <ADXL345.h>
#include <SPI.h>
#include <SdFat.h>
SdFat SD;
TinyGPSPlus gps;

const int chipSelect = 53;

char latit[10];
char longit[10];

#define Register_ID 0
#define Register_2D 0x2D
#define Register_X0 0x32
#define Register_X1 0x33
#define Register_Y0 0x34
#define Register_Y1 0x35
#define Register_Z0 0x36
#define Register_Z1 0x37

// Endereco I2C do sensor : 83 em decimal ou 0x53
int ADXAddress = 0x53;  // the default 7-bit slave address
int reading = 0;
int val=0;
int X0,X1,X_out;
int Y0,Y1,Y_out;
int Z1,Z0,Z_out;
double Xg,Yg,Zg;


void setup() {
  Serial.begin(38400); //SERIAL
  Serial1.begin(38400); //GPS
  Serial3.begin(9600); //XBEE

  Serial.println("---Formula SAE UFPB---");
  delay(300);
  Serial.println("Formato dos dados");
  Serial.println("DiaMesAno,HoraMinutosSegundos,Longitude,Latitude,Velocidade,Altitude,Satelites,GyroX,GyroY,GyroZ");
  delay(300);

  Wire.begin();
  Serial.println("Inicializando Giroscopio...");
  delay(1000); // wait for the sensor to be ready
  
  Wire.beginTransmission(ADXAddress);
  Wire.write(Register_2D);
  Wire.write(8);                //measuring enable
  Wire.endTransmission();     // stop transmitting

  Serial.print("Inicializando SD Card...");
  pinMode(53, OUTPUT);
  //Verifica se o cartão SD está conectado
  if (!SD.begin(chipSelect))
    Serial.println(" Erro!");
  else
    Serial.println(" OK!");
  //escreve os dados e fecha o arquivo
}


void loop() {
  while (Serial1.available()) {
    String dados = "";
    if (gps.encode(Serial1.read())) {
      dtostrf((gps.location.lat()), 5, 5, latit);
      dtostrf((gps.location.lng()), 5, 5, longit);
 
      pegaAcc();
      // Impressão de valores GPS
      dados += gps.date.year(); dados += "-";
      dados += gps.date.month(); dados += "-";
      dados += gps.date.day(); dados += ",";
      
      dados += gps.time.hour(); dados += "-";
      dados += gps.time.minute(); dados += "-";
      dados += gps.time.second(); dados += ",";
      
      dados += longit; dados += ",";
      dados += latit; dados += ",";
      dados += gps.speed.kmph();  dados += ",";
      dados += gps.altitude.meters(); dados += ",";
      dados += gps.satellites.value(); dados += ",";

      // Impressão de valores GYRO Acelerometro
      dados += (Xg); dados += ",";
      dados += (Yg);
      
      Serial.println(dados);
      Serial3.println(dados);
      File dataFile = SD.open("Corrida 1.2016.txt", FILE_WRITE);
      //escreve os dados e fecha o arquivo
      if (dataFile) {
        dataFile.println(dados);
        dataFile.close();
        delay(10);
      }
    delay(100);
    }
  }
}

void pegaAcc(){
    //--------------X
  Wire.beginTransmission(ADXAddress); // transmit to device
  Wire.write(Register_X0);
  Wire.write(Register_X1);
  Wire.endTransmission();
  Wire.requestFrom(ADXAddress,2); 
  if(Wire.available()<=2)   
  {
    X0 = Wire.read();
    X1 = Wire.read(); 
    X1=X1<<8;
    X_out=X0+X1;   
  }

  //------------------Y
  Wire.beginTransmission(ADXAddress); // transmit to device
  Wire.write(Register_Y0);
  Wire.write(Register_Y1);
  Wire.endTransmission();
  Wire.requestFrom(ADXAddress,2); 
  if(Wire.available()<=2)   
  {
    Y0 = Wire.read();
    Y1 = Wire.read(); 
    Y1=Y1<<8;
    Y_out=Y0+Y1;
  }
  Xg=X_out/256.0;
  Yg=Y_out/256.0;
  Zg=Z_out/256.0;
}
