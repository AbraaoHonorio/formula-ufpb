int led = 4; 
int hall = 7;
int statushall = LOW;
unsigned long inicio, fim, volta;
float r;


void setup() {

pinMode(4, OUTPUT);
pinMode(7, INPUT);
Serial.begin(9600);

}


void loop() {
statushall = digitalRead(hall);     //Leitura do sensor (0 ou 1);

if(statushall == 0)
  {
    { digitalWrite(4, HIGH);    //Acende o LED se a leitura do sensor for 0

  if(inicio == 0)     //Coloquei um valor qualquer, pois depois chamarei o inicio de 0
    {
    inicio = millis();     //Marca o tempo em que o programa ta rodando e guarda na variavel inicio
   delay(10);     //Delay para não fazer mais leituras enquanto o imã ta passando pelo sensor
    }
    // O início deixa de ser 0, pois assume o valor millis
  else
    { fim = millis();     //Marca o tempo que o programa rodou até aqui, que seria a volta seguinte
      volta = fim - inicio;    //Faz a subtração do tempo da segunda passada do imã pela primeira e da o tempo da volta
      r = 60000/volta;    //RPM
      if(r<5000){Serial.print(r);    //Mostra o valor obtido
      Serial.println(" rpm");    //Adiciona a unidade rpm
      }inicio = 0;     //Declara o início sendo 0 para iniciar uma nova contagem do tempo de volta
    }
    }
  }

else  
  { digitalWrite(4, LOW);
  }

}

