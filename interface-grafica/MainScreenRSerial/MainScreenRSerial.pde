//MAPA
import de.fhpotsdam.unfolding.utils.*;
import de.fhpotsdam.unfolding.marker.*;
import de.fhpotsdam.unfolding.*;
import de.fhpotsdam.unfolding.core.*;
import de.fhpotsdam.unfolding.geo.*;
import de.fhpotsdam.unfolding.providers.*;
UnfoldingMap map;
Location posCarroGEO;
SimplePointMarker posCarroMKR;
MarkerManager bulbassauro;

//SERIAL
import processing.serial.*;
Serial entradaSerial;
String portaSerial, parsed[];
int failCOM;

PFont fontePP, fonteP, fonteM, fonteG,fontePM;
PImage header, pause, crosshairs, alerta1, alerta1RES, potMaxPNG, pedalErro1, poucaCarga,desconectado,aguardadados;
Table table;

//GERAL:
String inicioSessao, inString="000000",noString="nononono", arquivo="Dados 07.11.csv", quebraHorasGPS[];
float alturaVaria=0, alturaVariaC1=0, distRectC1=0, alturaVariaC2=0, distRectC2=0, alturaVariaC3=0, distRectC3=0, alturaVariaC4=0, distRectC4=0, distRect=5, distRect2;
float moveX, moveY, resX=0, resY=0;
int totalLinhas=0, tableCount=0,constHR=0;

//DADOS:
String data="xxx", hora="xxx";
float gyroX=0, gyroY=0, gyroZ=0, accX=0, accY=0, somaAccX=0, somaAccY=0, longit=0, latit=0, velocidade=0, altit=0;
int satel=0;

//CRONÔMETRO:
int ultimaCont=0, segundoPassado=0, segundosString=0, contaTempo=0, startCron=0;

void setup() {
  //TELA
  size(1366, 768, P2D);
  frame.setResizable(true);
  //FONTES
  fontePP=loadFont("DINPro-Medium-10.vlw");
  fontePM=loadFont("DINPro-Medium-12.vlw");
  fonteP=loadFont("DINPro-Medium-15.vlw");
  fonteM=loadFont("DINPro-Medium-30.vlw");
  fonteG=loadFont("DINPro-Medium-45.vlw");
  //IMAGENS
  header = loadImage("Header3.png");
  pause = loadImage("PauseLeitura.png");
  crosshairs = loadImage("Crosshairs2.png");
  alerta1 = loadImage("AlertaBateria1.png");
  alerta1RES = loadImage("AlertaBateria1.png");
  potMaxPNG = loadImage("potMax.png");
  pedalErro1 = loadImage("ProblemaAcel.png");
  poucaCarga = loadImage("PoucaCarga.png");
  desconectado = loadImage("Desconectado.png");
  aguardadados = loadImage("AguardaDados.png");
  //TABELA
  //table = loadTable(arquivo, "header,csv");
  //totalLinhas=table.getRowCount();
  
  smooth();
  frameRate(30);
  imageMode(CENTER);
  inicioSessao = day()+"/"+month()+"/"+year()+" às "+hour()+":"+minute()+":"+second();
  preparaSerial();
  pegaMapa();
}

void draw() {
  background(224, 224, 224);
  image(header, width/2, height/18+30);
  if (failCOM == 1)                          ///Mensagem de erro para desconectado ou sem receber dados
    image(desconectado, width/2, height/2);
  if ((inString==null && failCOM==0) || (inString=="000000" && failCOM==0))
    image(aguardadados, width/2, height/2);
 
  mostraPos(1);
  if(inString != "000000" && inString != null){
  parseData();
  //mapa();
  rawData();
  velocimetro();
  caixaEsquerda1();
  caixaEsquerda2();
  caixaEsquerda3();
  caixaEsquerda4(startCron);
  forcaG();
  //1: ativado 0: desativado
  }
}

void preparaSerial() {
  printArray(Serial.list());
  portaSerial = "COM5";         /// ALTERA A PORTA COM DO ARDUINO AQUI
  if (portaSerial.equals("COM5")) {
    entradaSerial = new Serial(this, "COM5", 9600);
    failCOM = 0;
  } else
    failCOM = 1;
  println("OK");
  delay(500);
}

void serialEvent(Serial entradaSerial) {      ///Lê a porta serial até pular linha
  entradaSerial.bufferUntil('\n');
  inString = entradaSerial.readString();
  println(inString);
}

void parseData() {
  String parsed[] = split(inString,",");
  if(parsed[0] != null)
    data = parsed[0];
  if(parsed[1] != null)
    hora=parsed[1];
  if(parsed[2] != null)
    longit = Float.parseFloat(parsed[2]);
  if(parsed[3] != null)
    latit =  Float.parseFloat(parsed[3]);
  if(parsed[4] != null)
    velocidade = Float.parseFloat(parsed[4]);
  if(parsed[5] != null)
    altit = Float.parseFloat(parsed[5]);
  if(parsed[6] != null)
    satel = Integer.parseInt(parsed[6]);
//  satel = row.getInt("Satelites");
  //gyroX = row.getFloat("GyroX");
  //gyroY = row.getFloat("GyroY");
  //gyroZ = row.getFloat("GyroZ");
  //if(parsed[6] != null)
    //accX = Float.parseFloat(parsed[6]);
  //if(parsed[7] != null)  
    //accY = Float.parseFloat(parsed[7]);
  //accX = row.getFloat("AccX");
  //accY = row.getFloat("AccY");
  
  posCarroGEO = new Location(latit, longit);
  SimplePointMarker posCarroMKR = new SimplePointMarker(posCarroGEO);
  color c = color(255, 0, 0);
  posCarroMKR.setColor(c);
  posCarroMKR.setStrokeWeight(2);

  bulbassauro = new MarkerManager();
  bulbassauro.addMarker(posCarroMKR);
  map.addMarkerManager(bulbassauro);
  MapUtils.createDefaultEventDispatcher(this, map); 
  tableCount++;
}

void mapa(){
  pushMatrix();
  translate(width/1.15, height/1.3);
  map.draw();
  bulbassauro.clearMarkers();
  popMatrix();
}

void pegaMapa() {
  //Map(processing.core.PApplet p, float x, float y, float width, float height, AbstractMapProvider provider) 
  //map = new UnfoldingMap(this, new GeoMapApp.TopologicalGeoMapProvider()); NAO PEGA
  //map = new UnfoldingMap(this, new MapBox.ControlRoomProvider()); NAO PEGA
  //map = new UnfoldingMap(this, new MapBox.WorldLightProvider()); NAO PEGA
  //map = new UnfoldingMap(this, new Microsoft.AerialProvider()); VISAO AEREA
  //map = new UnfoldingMap(this, new Microsoft.HybridProvider()); VISAO AEREA COM RUAS MARCADAS
  //map = new UnfoldingMap(this, new Microsoft.RoadProvider()); APENAS RUAS
  map = new UnfoldingMap(this,0,0,width/4,height/3,new OpenStreetMap.OpenStreetMapProvider()); //COMPLETO
  //map = new UnfoldingMap(this, new OpenStreetMap.OSMGrayProvider()); NAO PEGA
  //map = new UnfoldingMap(this, new StamenMapProvider.Toner()); GAY
  //map = new UnfoldingMap(this, new StamenMapProvider.TonerBackground()); GAYER;
  //map = new UnfoldingMap(this, new StamenMapProvider.TonerLite()); CCHLA
  //map = new UnfoldingMap(this, new StamenMapProvider.WaterColor()); NAO PEGA
  Location UFPBLoc = new Location(-7.136198, -34.845351);
  map.zoomAndPanTo(UFPBLoc, 17);
}

void caixaEsquerda1() {
  alturaVariaC1=height/20;
  distRectC1=width/200;
  textFont(fonteP);
  pushMatrix();
  translate(width/200, height/6); //Alterando aqui move tudo abaixo
  rect(0, 0, width/6+7, height/3);
  fill(255, 255, 255);
  textAlign(CENTER);
  text("Modo de Leitura TXT", width/12, height/40);
  if (width<1800)
    textFont(fontePM);
  textAlign(CORNER);
  text("Início da sessão: "+inicioSessao, distRectC1, alturaVariaC1);
  alturaVariaC1+=15;
  text("Lendo arquivo: " + arquivo, distRectC1, alturaVariaC1);
  alturaVariaC1+=15;
  text("Mostrando linha "+tableCount+ " de "+totalLinhas, distRectC1, alturaVariaC1);
  alturaVariaC1+=15;
  text("Data da gravação: "+data, distRectC1, alturaVariaC1);
  alturaVariaC1+=30;
  text("F: Avançar 100 linhas", distRectC1, alturaVariaC1);
  alturaVariaC1+=15;
  text("D: Voltar 100 linhas", distRectC1, alturaVariaC1);
  alturaVariaC1+=15;
  text("C: Iniciar Cronômetro", distRectC1, alturaVariaC1);
  alturaVariaC1+=15;
  text("V: Parar Cronômetro", distRectC1, alturaVariaC1);
  alturaVariaC1+=15;
  text("P: Pausar", distRectC1, alturaVariaC1);
  popMatrix();
}

void caixaEsquerda2() {
  alturaVariaC2=height/20;
  distRectC2=width/200;
  pushMatrix();
  translate(width/200, height/1.98);
  fill(55, 55, 55);
  rect(0, 0, width/12, height/4);
  fill(255, 255, 255);
  textFont(fonteP);
  textAlign(CENTER);
  text("Baterias", width/24, height/50);
  textAlign(CORNER);
  if (width<1800)
    textFont(fontePM);
  text("Pack 1: 00ºC", distRectC2, alturaVariaC2);
  alturaVariaC2+=15;
  text("Pack 2: 00ºC", distRectC2, alturaVariaC2);
  alturaVariaC2+=15;
  text("Pack 3: 00ºC", distRectC2, alturaVariaC2);
  alturaVariaC2+=25;
  text("Status: nononono", distRectC2, alturaVariaC2);
  alturaVariaC2+=25;
  text("Carga: 00%", distRectC2, alturaVariaC2);
  popMatrix();
}

void caixaEsquerda3() {
  alturaVariaC3=height/20;
  distRectC3=width/200;
  pushMatrix();
  translate((width/200)+(width/12)+7, height/1.98);
  fill(55, 55, 55);
  rect(0, 0, width/12, height/4);
  textAlign(CENTER);
  fill(255, 255, 255);
  textFont(fonteP);
  text("Etc", width/24, height/50);
  if (width<1800)
    textFont(fontePM);
  popMatrix();
}

void caixaEsquerda4(int startCron) {
  if (startCron == 1) {
    textFont(fonteP);
    quebraHorasGPS = split(hora, "-");
    segundosString = Integer.parseInt(quebraHorasGPS[2]);
    if (segundosString!=segundoPassado)
      contaTempo++;
    segundoPassado = segundosString;
  }
  cronos();
}

void cronos() {
  alturaVariaC4=height/45;
  distRectC4=width/200;
  pushMatrix();
  translate(width/200, height/1.31);
  fill(55, 55, 55);
  rect(0, 0, width/6+7, height/7);
  textAlign(CENTER);
  fill(255, 255, 255);
  textFont(fonteP);
  text("CRONÔMETRO v0.1", width/12, alturaVariaC4);
  alturaVariaC4+=40;
  textFont(fonteM);
  if (startCron==1)
    text("00:"+contaTempo, width/12, alturaVariaC4);
  else
    text("00:00", width/12, alturaVariaC4);
  alturaVariaC4+=30;
  textFont(fonteP);
  text("Último tempo: "+ultimaCont+"s", width/12, alturaVariaC4);
  textAlign(CORNER);
  popMatrix();
  if (velocidade>30) {
    ultimaCont=contaTempo;
    startCron=0;
  }
}


void rawData() {
  if(width<1800)
    constHR=12;
  else
    constHR=15;
  alturaVaria=(height/40)+20;
  distRect2=width/15+10;
  textFont(fonteP);
  pushMatrix();
  translate(width/1.15, height/6); //Alterando aqui move tudo abaixo
  fill(255, 255, 255);
  rect(0, 0, width/8, height/2.5);
  textAlign(CENTER);
  fill(55, 55, 55);
  text("DADOS BRUTOS", width/16, height/40);
  textAlign(CORNER);
  if (width<1900)
    textFont(fontePM);  
  text("Vel. Eixo: ", distRect, alturaVaria);
  text(noString, distRect2, alturaVaria); 
  alturaVaria+=constHR;
  text("Temp. Motor: ", distRect, alturaVaria);
  text(noString, distRect2, alturaVaria); 
  alturaVaria+=constHR;
  text("Temp. Baterias: ", distRect, alturaVaria);
  text(noString, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("Ângulo Volante: ", distRect, alturaVaria);
  text(noString, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("Corrente: ", distRect, alturaVaria);
  text(noString, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("Tensão: ", distRect, alturaVaria);
  text(noString, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("Potência Inst: ", distRect, alturaVaria);
  text(noString, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("Latitude: ", distRect, alturaVaria);
  text(latit, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("Longitude: ", distRect, alturaVaria);
  text(longit, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("Altitude: ", distRect, alturaVaria);
  text(altit, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("Velocidade GPS: ", distRect, alturaVaria);
  text(velocidade, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("Hora: ", distRect, alturaVaria);
  text(hora, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("Data: ", distRect, alturaVaria);
  text(data, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("Satélites: ", distRect, alturaVaria);
  text(satel, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("G-Force X: ", distRect, alturaVaria);
  text(accX, distRect2, alturaVaria);
  alturaVaria+=constHR;
  text("G-Force Y: ", distRect, alturaVaria);
  text(accY, distRect2, alturaVaria);
  popMatrix();
}

void forcaG() {
  image(crosshairs, width/1.65, height/2); 
  moveX=map(accX, -2, 2, -200, 200);
  moveY=map(accY, -2, 2, -200, 200);
  pushMatrix();
  translate(width/1.65, height/2);
  fill(255, 0, 0);
  ellipseMode(CENTER);
  ellipse(moveX, -moveY, 15, 15);
  popMatrix();
}

void velocimetro() {
  textFont(fonteG);
  fill(55, 55, 55);
  textAlign(CENTER);
  text(velocidade, width/2.5, height/2);
  textFont(fonteM);
  text("Km/h", width/2.5, height/1.85);
}

void mostraPos(int x) {
  if (x==1) {
    if (mouseX!=0 && mouseY!=0) {
      resX=(float)width/mouseX;
      resY=(float)height/mouseY;
    }
    text("X: "+resX, mouseX+20, mouseY);
    text("Y: "+resY, mouseX+20, mouseY+15);
  }
}

void keyPressed() {
  final int k = keyCode;
  if (k == 'P')
    if (looping) {
      image(pause, width/2, height/2);
      noLoop();
    } else          
      loop();
  if (k == 'F')
    tableCount+=100;
  if (k == 'D' && tableCount>100)
    tableCount-=100;
  if (k == 'C') {
    contaTempo=0;
    startCron = 1;
  }
  if (k == 'V') {
    contaTempo=0;
    startCron = 0;
  }
}

