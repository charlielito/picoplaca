#include <Wire.h> //supports I2C communication with external clock
#include <U8g2lib.h> //OLED display grafics library
#include <EEPROM.h> //library to handle with permament data after turning off Device
#include <RTClib.h> // Libreria del reloj real time clock
#include "ds3231.h" // Libreria para obtener la temperatura del modulo reloj

RTC_DS1307 RTC;

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

String meses[]={"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};
String dias[]={"Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado", "Domingo"}; // Arreglo de dias de la semana.
int diaMeses[12] = {31,28,31,30,31,30,31,31,30,31,30,31}; // dias de los meses

unsigned long starttime;
unsigned long endtime;

String strHoras;
String strMinutos;
String strSegundos;

String strDiaSemana;
String strDia;
String strMes;
String strAnno;

boolean pixCorrido[3] = {false,false,false};
String Text[3] = {"","",""};
String textoAnterior[3] = {"","",""};

int filaPixX[3] = {0,0,0};
int filaPixY[3] = {0,0,0};

int segundos;
int minutos;
int horas;


int buttonExit = 3;
int buttonUp = 4;
int buttonDown = 5;

int diaSemana;

int contadorSet = 0;

int anchoFuente;

const byte button = 2;
const byte alarmPin = 13;
volatile int state = LOW; //alarm and configuration states
volatile int page = 0;
volatile bool buttonPressed = false; //handle with the config button
volatile long seconds0 = 0;
volatile long seconds1 = 0;


#define MENU_ITEMS 4
String menu_strings[] = {"Configurar: ", "Fecha/hora", "Pico y placa", "SOAT/Tecnico" };
int menu_current = 1;

void draw_menu(void) {

  u8g2.setFontRefHeightText();
  u8g2.setFontPosTop();
  u8g2.setFont(u8g2_font_helvB12_tr);
  //u8g.setFontMode(1);
  int h = u8g2.getFontAscent()-u8g2.getFontDescent();
  int w = u8g2.getWidth();
  for( int i = 0; i < MENU_ITEMS; i++ ) {        // draw all menu items
    int d = (w-u8g2.getStrWidth(menu_strings[i].c_str()))/2;
    u8g2.setDrawColor(1);
    if ( i == menu_current ) {               // current selected menu item
      u8g2.drawBox(0, i*h, w, h-1);     // draw cursor bar
      u8g2.setDrawColor(0);
    }
    u8g2.drawStr(d, i*h, menu_strings[i].c_str());

  }
}

void setup(void) {
  Serial.begin(9600);
  u8g2.begin();
  DateTime now = RTC.now(); // Permite utilizar sentencias now.lo que necesites

  attachInterrupt(digitalPinToInterrupt(button), changeState, CHANGE);
  
  RTC.adjust(DateTime(__DATE__, __TIME__)); // sets the RTC to the date & time this sketch was compiled
  verificaDiaSemana(); // Corre la función verifica dia.
  horas = now.hour();
  strDia = String(now.day());
  strMes = meses[now.month()];
  strAnno = String(now.year());
  leapYearCorrection(now.year());

  Serial.println(strDia + strMes + strAnno);
}

void changeState(){
  if (digitalRead(button) == HIGH){ //it was rising
    state = HIGH;
    buttonPressed = true;
    seconds0 = millis();
    seconds1 = seconds0;
    menu_current++;
    if (menu_current > 3)
      menu_current = 1;
  }
  else{ //falling
    seconds1 = millis();
    state = LOW;
  }
  digitalWrite(alarmPin, state);
}

void verificaDiaSemana(){
  DateTime now = RTC.now(); // Permite utilizar sentencias now.lo que necesites
  strDiaSemana = dias[((now.unixtime() / 86400L)-17266)%7];//now.dayOfWeek()
}

void leapYearCorrection(int Year){
  if ( (Year%4==0 && Year%100!=0) || (Year%400==0) )// it is a leap year
    diaMeses[1] = 29;
}

void configHora(){
  page = 1;
}

void loop(void) {
  DateTime now = RTC.now(); // Permite utilizar sentencias now.lo que necesites

  
  if (digitalRead(button) == HIGH){
    contadorSet = contadorSet + 1;
    if (contadorSet > 3){
      configHora();
    }
  }else{
    contadorSet = 0;
  }

  segundos = now.second();

  if (segundos < 15){

    if (segundos == 0){
      minutos = now.minute();
      if (minutos == 0){
        horas = now.hour();
        if (horas == 0){
          verificaDiaSemana();
          strDia = String(now.day());
          if (strDia == "1"){
            strMes = meses[now.month()];
            if (strMes == "1"){
              strAnno = String(now.year());
              leapYearCorrection(now.year());
            }
          }
        }
      }
    }

    if (segundos < 10){
      strSegundos = "0"+String(segundos);
    }else{
      strSegundos = String(segundos);
    }

    if (minutos < 10){
      strMinutos = "0"+String(minutos);
    }else{
      strMinutos = String(minutos);
    }

    if (horas < 10){
      strHoras = "0"+String(horas);
    }else{
      strHoras = String(horas);
    }

    Text[0] = strDiaSemana+", "+strDia+" de "+strMes+" del "+strAnno;
    setText(Text[0].length(),4, 1,1,0);
    
    Text[1] = strHoras+":"+strMinutos+":"+strSegundos;
    setText(Text[1].length(),2, 2,2,14);
  
    float temp = 30;//DS3231_get_treg();
    Text[2] = "Temp: "+ String(temp)+" "+String((char)176)+"C";
    setText(Text[2].length(),2, 3,3,2);
    
  }
  else{
    if (segundos >= 15 && segundos < 30){
      Text[0] = "Revisi"+String((char)243)+"n tecnicomecanica en";
      Text[1] = "386";
      Text[2] = "d"+String((char)237)+"as";}
    
    if (segundos >= 30 && segundos < 45){
      Text[0] = "Renovar SOAT en";  
      Text[1] = "324";  
      Text[2] = "d"+String((char)237)+"as";}
    
    if (segundos >= 45){
      Text[0] = "Cambiar aceite en";
      Text[1] = "20000";
      Text[2] = "kilometros";}
  
    setText(Text[0].length(),4, 1,1,0);
    setText(Text[1].length(),2, 2,2,0);
    setText(Text[2].length(),2, 3,3,0);
  }

  starttime = millis();
  while ((millis() - starttime) <= 1000){
    // picture loop
    u8g2.firstPage();  
    do {
      draw();
    } while( u8g2.nextPage() );
    // rebuild the picture after some delay
    //delay(50);
    
    if (pixCorrido[0] == true){
      filaPixX[0] -= 1;
    }
    if (pixCorrido[1] == true){
      filaPixX[1] -= 1;
    }
    if (pixCorrido[2] == true){
      filaPixX[2] -= 1;
    }
  }
}

void main_draw(){
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.drawStr(filaPixX[0],filaPixY[0],Text[0].c_str());

  u8g2.setFont(u8g2_font_helvB18_tr);
  u8g2.drawStr(filaPixX[1],filaPixY[1],Text[1].c_str());

   u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.drawStr(filaPixX[2],filaPixY[2],Text[2].c_str());
}



void draw() {

  switch(page){
    case 0:
      main_draw();
      break;
    case 1:
      draw_menu();
  }


}



void setText(int tamanoTexto, int posX, int posY, int num, int offset){
  //int tamanoTexto = texto.length();
  num--; //indices empiezan en 0
  switch (posY){
    case 1:
    filaPixY[num]= 12;
    anchoFuente = 9;
    break;
    case 2:
    filaPixY[num] = 40;
    anchoFuente = 17;
    break;
    case 3:
    filaPixY[num] = 60;
    anchoFuente = 9;
    break;
  }

  switch (posX){
    case 1:
    filaPixX[num] = 0+offset;
    pixCorrido[num] = false;
    break;
    case 2:
    filaPixX[num] = centrado(tamanoTexto,anchoFuente)+offset;
    pixCorrido[num] = false;
    break;
    case 3:
    filaPixX[num] = alinearDerecha(tamanoTexto,anchoFuente)+offset;
    pixCorrido[num] = false;
    break;
  }
  if (posX == 4 && textoAnterior[num] != Text[num]){
    textoAnterior[num] = Text[num];
    pixCorrido[num] = true;
    filaPixX[num] = 2;
  }
}

//-----------------------------------FUNCION CENTRADO-------------------------------
int centrado(int val,int pixFont){
  int pix = (int)((128-(val*pixFont))/2);
  return pix;
}

//-----------------------------------FUNCION DERECHA-------------------------------
int alinearDerecha(int val,int pixFont){
  int pix = 128-(pixFont*val);
  return pix;
}
