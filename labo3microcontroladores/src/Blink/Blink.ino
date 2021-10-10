#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Software SPI (slower updates, more flexible pin options):
        // pin 7 - Serial clock out (SCLK)
        // pin 6 - Serial data out (DIN)
        // pin 5 - Data/Command select (D/C)
        // pin 4 - LCD chip select (CS)
        // pin 3 - LCD reset (RST)

//Configuracion de pines para el display
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

//Definicion de variables globales
float V1= {0.00};
float V2= {0.00};
float V3= {0.00};
float V4= {0.00};
float v1k = {0.00};
float v2k = {0.00};
float v3k = {0.00};
float v4k = {0.00};

float boton = {0.00};
float interruptor = {0.00};
float k1= {4.8};
float threshold = {1.00};
bool etiqueta = true;

float current1 = {0.00};
float current2 = {0.00};
float current3 = {0.00};
float current4 = {0.00};


//Funcion de Setup para los puertos y pantalla 
void setup(){
  Serial.begin(9600); //Inicio del puerto Serial
  //Configira los pines como salida
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);  
  pinMode(13,OUTPUT);
  //Inicia la pantalla y se selecciona el contraste
  display.begin();
  display.setContrast(75);
  
  //Muestra la flor de Adafruit
  display.display();
  delay(2000);
  display.clearDisplay();   //Limpia el display

}

//Funcion encargada de encender los leds de emergencia 
//Recibe las tensiones de los cuatro puertos y si alguna pasa de 11.90 o de -11.90 enciende el led. 
void warning_DC(float v1, float v2, float v3, float v4){
  if((v1 >= 11.90) ||  (v1 <= -11.90)){
      digitalWrite(10, HIGH);
    }
  else{
      digitalWrite(10, LOW); //Mientras no pase del valor de umbral mantiene el led apagado
    }   
  if (v2 >= 11.90 || V2 <= -11.90){
      digitalWrite(11, HIGH);
    }
  else{
      digitalWrite(11, LOW); //Mientras no pase del valor de umbral mantiene el led apagado
    } 
  if (v3 >= 11.90 || V3 <= -11.90){
      digitalWrite(12, HIGH);
    }
  else{
      digitalWrite(12, LOW); //Mientras no pase del valor de umbral mantiene el led apagado
    } 
  if (v4 >= 11.90 || v4 <= -11.90){
      digitalWrite(13, HIGH);
    }
  else{
      digitalWrite(13, LOW); //Mientras no pase del valor de umbral mantiene el led apagado
    }   
}

//Como la funcion AC devuelve valores RMS es importante hacer una funcion de alarma con los valores RMS
void warning_AC(float v1, float v2, float v3, float v4){
  if((v1 >= 8.41)){
      digitalWrite(10, HIGH);
    }
  else{
      digitalWrite(10, LOW); //Mientras no pase del valor de umbral mantiene el led apagado
    }   
  if (v2 >= 8.41){
      digitalWrite(11, HIGH);
    }
  else{
      digitalWrite(11, LOW); //Mientras no pase del valor de umbral mantiene el led apagado
    } 
  if (v3 >= 8.41){
      digitalWrite(12, HIGH);
    }
  else{
      digitalWrite(12, LOW); //Mientras no pase del valor de umbral mantiene el led apagado
    } 
  if (v4 >= 8.41){
      digitalWrite(13, HIGH);
    }
  else{
      digitalWrite(13, LOW); //Mientras no pase del valor de umbral mantiene el led apagado
    }   
}

//------------------------------------------- Funciones para el calculo del valor maximo en modo AC ---------------------------------------\\
//Obtiene el valor de la amplitud de la onda para el primer puerto
float get_max_v1() {
  float max_v = 0.00;
  for(int i = 0; i < 100; i++) {
    float r = analogRead(A0);  // Lee el valor en el primer puerto (A0)
    if(max_v < r) max_v = r;
    delayMicroseconds(200);
  }
  return max_v;
}

//Obtiene el valor de la amplitud de la onda para el segundo puerto
float get_max_v2() {
  float max_v = 0.00;
  for(int i = 0; i < 100; i++) {
    float r = analogRead(A1);  // Lee el valor en el segundo puerto (A1)
    if(max_v < r) max_v = r;
    delayMicroseconds(200);
  }
  return max_v;
}

//Obtiene el valor de la amplitud de la onda para el tercer puerto
float get_max_v3() {
  float max_v = 0.00;
  for(int i = 0; i < 100; i++) {
    float r = analogRead(A2);  // Lee el valor en el tercer puerto (A2)
    if(max_v < r) max_v = r;
    delayMicroseconds(200);
  }
  return max_v;
}

//Obtiene el valor de la amplitud de la onda para el cuarto puerto
float get_max_v4() {
  float max_v = 0.00;
  for(int i = 0; i < 100; i++) {
    float r = analogRead(A3);  // rLee el valor en el cuarto puerto (A3)
    if(max_v < r) max_v = r;
    delayMicroseconds(200);
  }
  return max_v;
}


void loop(){

  //El arduino va a medir con una precision de 2^10-1 = 1023, por esto es necesario dividir los valores medidos entre 1023 para obetener el valor real
  //Ademas el valor medido se multiplica por 5 con el fin de transformar la lectura a un rango de [0,5]
  //Por ultimo se usa una logica de regla de 3 para pasar del rango electrico de Arduino [0,5] al rango real medido [-12,12]

  boton = analogRead(A4); // Revisa si el boton de modo (AC/DC) esta encendido
  interruptor = analogRead(A5);

  if (interruptor > 3.00){
    while (etiqueta){
      Serial.println("Canal 1");
      Serial.println("Canal 2");
      Serial.println("Canal 3");
      Serial.println("Canal 4");
      Serial.println("AC/DC");
      etiqueta = false;  
    }
  }
  
  if (boton > 3.00){ // Si el boton esta encendido mida AC
    
    float VAC1 = get_max_v1();
    VAC1 = ((((VAC1*5)/1023)*k1)-12);
    if ((VAC1+0.65) == 12) VAC1 += 0.65;  
    VAC1 /= sqrt(2); // Obtiene el valor RMS

    float VAC2 = get_max_v2();
    VAC2 = ((((VAC2*5)/1023)*k1)-12);
    if ((VAC2+0.65) == 12) VAC2 += 0.65;
    VAC2 /= sqrt(2); // Obtiene el valor RMS
    
    float VAC3 = get_max_v3();
    VAC3 = ((((VAC3*5)/1023)*k1)-12);
    if ((VAC3+0.65) == 12) VAC3 += 0.65;
    VAC3 /= sqrt(2); // Obtiene el valor RMS

    float VAC4 = get_max_v4();
    VAC4 = ((((VAC4*5)/1023)*k1)-12);
    if ((VAC4+0.65) == 12) VAC4 += 0.65;
    VAC4 /= sqrt(2); // Obtiene el valor RMS

    if (interruptor > 3.00){
      if ((current1 >= VAC1+threshold || current1 <= VAC1-threshold)||(current2 >= VAC2+threshold || current2 <= VAC2-threshold)||(current3 >= VAC3+threshold || current3 <= VAC3-threshold)||(current4 >= VAC4+threshold || current4 <= VAC4-threshold)){
        Serial.println(VAC1);
        Serial.println(VAC2);
        Serial.println(VAC3);
        Serial.println(VAC4);
        Serial.println("AC");
        current1 = VAC1;
        current2 = VAC2;
        current3 = VAC3;
        current4 = VAC4;
      }
    }

    //Pantalla
    
    display.print("Voltimetro AC \n");
    display.print("--------------");
    display.print("V1 :");
    display.print(VAC1);
    display.print(" Vrms");
    display.print("\n");
    display.print("V2 :");
    display.print(VAC2);
    display.print(" Vrms");
    display.print("\n");
    display.print("V3 :");
    display.print(VAC3);
    display.print(" Vrms");
    display.print("\n");
    display.print("V4 :");
    display.print(VAC4);
    display.print(" Vrms");
    display.print("\n");
    display.display();
    display.clearDisplay();

    warning_AC(VAC1,VAC2,VAC3,VAC4);
    
  }
  else{ // si el boton esta apagado mida DC

    //Mide los valores DC en esos canales
    V1 = analogRead(A0);
    V2 = analogRead(A1);
    V3 = analogRead(A2);
    V4 = analogRead(A3);
  
    //Transformar el rango de 0 a 5 a 0 a 24
    v1k = (((V1*5)/1023)*k1)-12;
    v2k = (((V2*5)/1023)*k1)-12;
    v3k = (((V3*5)/1023)*k1)-12;
    v4k = (((V4*5)/1023)*k1)-12;

    if (interruptor > 3.00){
      if ((current1 >= v1k+threshold || current1 <= v1k-threshold)||(current2 >= v2k+threshold || current2 <= v2k-threshold)||(current3 >= v3k+threshold || current3 <= v3k-threshold)||(current4 >= v4k+threshold || current4 <= v4k-threshold)){
        Serial.println(v1k);
        Serial.println(v2k);
        Serial.println(v3k);
        Serial.println(v4k);
        Serial.println("DC");
        current1 = v1k;
        current2 = v2k;
        current3 = v3k;
        current4 = v4k;
      }
    }
    //Pantalla 
  
    display.print("Voltimetro DC \n");
    display.print("--------------");
    display.print("V1: ");
    display.print(v1k);
    display.print(" V");
    display.print("\n");
    display.print("V2: ");
    display.print(v2k);
    display.print(" V");
    display.print("\n");
    display.print("V3: ");
    display.print(v3k);
    display.print(" V");
    display.print("\n");
    display.print("V4: ");
    display.print(v4k);
    display.print(" V");
    display.print("\n");
    display.display();
    display.clearDisplay();

    warning_DC(v1k,v2k,v3k,v4k);
  }
}
