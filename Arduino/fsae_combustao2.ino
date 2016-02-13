
/* FORMULA SAE UFPB
 * DATA DE CRIAÇÃO: 04/09/2015
 * CRIADO POR: Frederico Augusto Santos Brasil
 * MANTIDO POR: Frederico A. S. Brasil | Filipe Cavalcanti | Josias Ferreira
 * DESCRIÇÃO: Versão final do software respsonsável pela telemetria e sensoriamento da equipe Formula UFPB - Combustão na competição 2015
 * FUNÇÕES: Data/Hora, Velocidade Instantânea, Latitude, Longitude, Inclinação, Aceleração linear e lateral
 * VERSÃO: 1.0
 * FRITZING: fsae_combustao.fzz
 */



// ------------------------------------------------------
// Declarações e bibliotecas relativas ao GPS e RTC
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <TinyGPS++.h>
TinyGPSPlus gps;
//#include <stdlib.h>

char latit[10];
char longit[10];
// ------------------------------------------------------
// Declarações e bibliotecas relativas ao Giroscópio e Acelerômetro
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <Wire.h>
#include <ADXL345.h>

const float alpha = 0.5;
 
double fXg = 0;
double fYg = 0;
double fZg = 0;
 
ADXL345 acc;

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24

// I2C address of the L3G4200D.
// Use I2C scanner to find this value!
int L3G4200D_Address = 0x69;

// Can fine-tune this if you need to
float DPS_MULT = 0.0000085;

// Delta angles (raw input from gyro)
int x = 0;
int y = 0;
int z = 0;
// Actual angles
float angX = 0;
float angY = 0;
float angZ = 0;
// Previous angles for calculation
float p_angX = 0;
float p_angY = 0;
float p_angZ = 0;

// Calibration values
int gyroLowX = 0;
int gyroLowY = 0;
int gyroLowZ = 0;
int gyroHighX = 0;
int gyroHighY = 0;
int gyroHighZ = 0;

// Used for calculating delta time
unsigned long pastMicros = 0;

void updateAngle()
{
  getGyroValues();
}

void calibrate()
{
  Serial.print("Calibrando o Gyro, nao mexa...");
  for (int i = 0; i < 200; i++) {
    getGyroValues();

    /*if (x > gyroHighX)
      gyroHighX = x;
    else if (x < gyroLowX)
      gyroLowX = x;

    if (y > gyroHighY)
      gyroHighY = y;
    else if (y < gyroLowY)
      gyroLowY = y;
*/
    if (z > gyroHighZ)
      gyroHighZ = z;
    else if (z < gyroLowZ)
      gyroLowZ = z;

    delay(10);
  }
  Serial.println(" Calibrado!");
}

// Print angles to Serial (for use in Processing, for example)
//void sendJson() {
//  char json[40];
// sprintf(json, "{\"x\":%d,\"y\":%d,\"z\":%d}", (int)(angX*1000), (int)(angY*1000), (int)(angZ*1000));
//Serial.println(json);
//}

void getGyroValues() {
  byte xMSB = readRegister(L3G4200D_Address, 0x29);
  byte xLSB = readRegister(L3G4200D_Address, 0x28);
  x = ((xMSB << 8) | xLSB);

  byte yMSB = readRegister(L3G4200D_Address, 0x2B);
  byte yLSB = readRegister(L3G4200D_Address, 0x2A);
  y = ((yMSB << 8) | yLSB);

  byte zMSB = readRegister(L3G4200D_Address, 0x2D);
  byte zLSB = readRegister(L3G4200D_Address, 0x2C);
  z = ((zMSB << 8) | zLSB);
}

int setupL3G4200D(int scale) {
  //From  Jim Lindblom of Sparkfun's code

  // Enable x, y, z and turn off power down:
  writeRegister(L3G4200D_Address, CTRL_REG1, 0b00001111);

  // If you'd like to adjust/use the HPF, you can edit the line below to configure CTRL_REG2:
  writeRegister(L3G4200D_Address, CTRL_REG2, 0b00000000);

  // Configure CTRL_REG3 to generate data ready interrupt on INT2
  // No interrupts used on INT1, if you'd like to configure INT1
  // or INT2 otherwise, consult the datasheet:
  writeRegister(L3G4200D_Address, CTRL_REG3, 0b00001000);

  // CTRL_REG4 controls the full-scale range, among other things:

  if (scale == 250) {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00000000);
  } else if (scale == 500) {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00010000);
  } else {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00110000);
  }

  // CTRL_REG5 controls high-pass filtering of outputs, use it
  // if you'd like:
  writeRegister(L3G4200D_Address, CTRL_REG5, 0b00000000);
}

void writeRegister(int deviceAddress, byte address, byte val)
{
  Wire.beginTransmission(deviceAddress); // start transmission to device
  Wire.write(address);       // send register address
  Wire.write(val);         // send value to write
  Wire.endTransmission();     // end transmission
}

int readRegister(int deviceAddress, byte address)
{
  int v;
  Wire.beginTransmission(deviceAddress);
  Wire.write(address); // register to read
  Wire.endTransmission();

  Wire.requestFrom(deviceAddress, 1); // read a byte

  while (!Wire.available()) {
    // waiting
  }
  v = Wire.read();
  return v;
}

// ------------------------------------------------------
// Declarações e bibliotecas relativas ao Xbee e Cartão SD
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <SPI.h>
#include <SdFat.h>
SdFat SD;

const int chipSelect = 53;

double pitch, roll, Xg, Yg, Zg;

void getacc(){
      
      acc.read(&Xg, &Yg, &Zg);
 
       //Low Pass Filter
       fXg = Xg * alpha + (fXg * (1.0 - alpha));
       fYg = Yg * alpha + (fYg * (1.0 - alpha));
       fZg = Zg * alpha + (fZg * (1.0 - alpha));
 
       //Roll & Pitch Equations
       roll  = (atan2(-fYg, fZg)*180.0)/M_PI;
       pitch = (atan2(fXg, sqrt(fYg*fYg + fZg*fZg))*180.0)/M_PI;
      delay(10);
}

void setup() {
  //Inicializa o console serial
  Serial.begin(38400);
  Serial1.begin(38400);
  Serial3.begin(57600);
  acc.begin();
  delay(200);  
  // ------------------------------------------------------
  // Configurações relativas ao GPS e RTC
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++

  //GPS conectado a Serial1 do arduino Mega

  // seconds, minutes, hours, day of the week, day of the month, month, year
  //myRTC.setDS1302Time(00, 44, 19, 4, 23, 9, 2015);
  //myRTC.updateTime();
  //String dataSessao = "";
  //dataSessao += myRTC.dayofmonth + myRTC.month + myRTC.year + myRTC.hours + myRTC.minutes + myRTC.seconds;

  Serial.println("Formula SAE UFPB");
  delay(300);
  Serial.println("Formato dos dados");
  Serial.println("DiaMesAno,HoraMinutosSegundos,Longitude,Latitude,Velocidade,Altitude,Satelites,GyroX,GyroY,GyroZ");
  delay(300);

  // ------------------------------------------------------
  // Configurações relativas ao Giroscópio e Acelerômetro
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Wire.begin();

  Serial.println("Inicializando Giroscopio...");
  setupL3G4200D(2000); // Configure L3G4200  - 250, 500 or 2000 deg/sec

  delay(1000); // wait for the sensor to be ready
  calibrate();
  attachInterrupt(0, updateAngle, RISING);

  pastMicros = micros();
  

  // ------------------------------------------------------
  // Configurações relativas ao Xbee e Cartão SD
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Serial.print("Inicializando SD Card...");
  pinMode(53, OUTPUT);
  //Verifica se o cartão SD está conectado
  if (!SD.begin(chipSelect))
    Serial.println(" Erro!");
  else
    Serial.println(" OK!");
  //escreve os dados e fecha o arquivo

}
       String nomear(String nome)
      {
         return nome;
      }
      char filename[] = "000000000000.txt";
       void getFilename(char *filename) 
       {
      int year = gps.date.year();; int month = gps.date.month(); int day = gps.date.day(); int hour = gps.time.hour(); int minute = gps.time.minute();
      filename[0] = '2';
      filename[1] = '0';
      filename[2] = (year-2000)/10 + '0';
      filename[3] = year%10 + '0';
      filename[4] = month/10 + '0';
      filename[5] = month%10 + '0';
      filename[6] = day/10 + '0';
      filename[7] = day%10 + '0';
      filename[8] = hour/10 + '0';
      filename[9] = hour%10 + '0';
      filename[10] = minute/10 + '0';
      filename[11] = minute%10 + '0';
      filename[12] = '.';
      filename[13] = 't';
      filename[14] = 'x';
      filename[15] = 't';
      
    return;
  }
void loop() {
  // ------------------------------------------------------
  // Rotinas relativas ao GPS e RTC
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++
  while(Serial1.available()) {
     //myRTC.updateTime();
    String dados = "";
    

    if (gps.encode(Serial1.read())) {
      dtostrf((gps.location.lat()), 5, 5, latit);
      dtostrf((gps.location.lng()), 5, 5, longit);

      // ------------------------------------------------------
      // Rotinas relativas ao Giroscópio e Acelerômetro
      // +++++++++++++++++++++++++++++++++++++++++++++++++++++++
      getGyroValues();
      
       //Calculate delta time
      float dt;
      if (micros() > pastMicros) //micros() overflows every ~70 minutes
        dt = (float) (micros() - pastMicros) / 1000000.0;
      else
        dt = (float) ((4294967295 - pastMicros) + micros()) / 1000000.0;


      // Calculate angles
     /*  if (x >= gyroHighX || x <= gyroLowX) {
        angX += ((p_angX + (x * DPS_MULT)) / 2) * dt;
        p_angX = x * DPS_MULT;
      } else {
        p_angX = 0;
      }

      if (y >= gyroHighY || y <= gyroLowY) {
        angY += ((p_angY + (y * DPS_MULT)) / 2) * dt;
        p_angY = y * DPS_MULT;
      } else {
        p_angY = 0;
      }*/

      if (z >= gyroHighZ || z <= gyroLowZ) {
        angZ += ((p_angZ + (z * DPS_MULT)) / 2) * dt;
        p_angZ = z * DPS_MULT;
      } else {
        p_angZ = 0;
      }

      pastMicros = micros();
      getacc();
      // Impressão de valores GPS
      dados += gps.date.year(); dados += "-"; 
      dados += gps.date.month(); dados += "-";
      dados += gps.date.day(); dados += ",";
      dados += gps.time.hour(); dados += "-"; 
      dados += gps.time.minute(); dados += "-"; 
      dados += gps.time.second(); String nome_arquivo=dados+".txt"; dados += ",";
      dados += longit; dados += ",";
      dados += latit; dados += ",";
      dados += gps.speed.kmph();  dados += ",";
      dados += gps.altitude.meters(); dados += ",";
      dados += gps.satellites.value(); dados += ",";
      
      // Impressão de valores GYRO Acelerometro
      dados += (roll); dados += ",";
      dados += (pitch); dados += ",";
      dados += (angZ * 1000); dados += ",";
      dados += fXg; dados += ",";
      dados += fYg;

      // ------------------------------------------------------
      // Rotinas relativas ao Xbee e Cartão SD
      // +++++++++++++++++++++++++++++++++++++++++++++++++++++++
      
      Serial.println(dados);
      getFilename();
      File dataFile = SD.open(filename, FILE_WRITE);
      //escreve os dados e fecha o arquivo
      if (dataFile) {
        dataFile.println(dados);
        dataFile.close();
        delay(10);        
      Serial3.println(dados);
    }
   }
  }
}


