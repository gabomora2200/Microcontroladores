#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Estados para el boton (No tienen nada que ver con la FSM)
#define ON  1
#define OFF 0

// Variables globales
int intr_count = 0;
int sec = 0;
int msec = 0;
int boton = OFF;
int estado;

//Funciones de estado
void CarrosAvance(void);
void BlinkVerdeRed(void);
void StopCarros(void);
void PeatonesAvance(void);
void BlinkRedVerde(void);

//Estructura FSM
struct FSM{
    void (*stateptr)(void);
    unsigned char time; 
    unsigned char next[2];
};

typedef struct FSM semaforo;

//Estados finitos para la maquina
#define CA  0 // Carros avanzando
#define BVR 1 // Parpadeo verde rojo
#define SC  2 // Stop carros
#define PA  3 // Peatones avanzando
#define BRV 4 // parpadeo rojo verde

int estado; // Se define el estado base o default. 

// Definicion FSM con sus respectivos tiempos y estados siguientes
semaforo fsm[5] = {
    {&CarrosAvance,10,{CA,BVR}},
    {&BlinkVerdeRed,6,{SC,SC}},
    {&StopCarros,2,{PA,PA}},
    {&PeatonesAvance,10,{BRV,BRV}},
    {&BlinkRedVerde,6,{CA,CA}}
};

// Definicion de funciones de estado
void CarrosAvance(void){
    PORTB = (1<<PB0)|(0<<PB1)|(0<<PB2)|(1<<PB3)|(0<<PB4); //Enciende las luces para el avance de los carros
}
void BlinkVerdeRed(void){
  PORTB ^= (1<<PB0)|(1<<PB3); // hace las luces parpadear
}
void StopCarros(void){
    PORTB = (0<<PB0)|(1<<PB1)|(0<<PB2)|(1<<PB3)|(0<<PB4); //Enciende la luz amarilla 
}
void PeatonesAvance(void){
    PORTB = (0<<PB0)|(0<<PB1)|(1<<PB2)|(0<<PB3)|(1<<PB4); //Enciende las luces para el avance de los peatones
}
void BlinkRedVerde(void){
  PORTB ^= (1<<PB2)|(1<<PB4); // hace las luces parpadear
}
// Funciones miscelaneas
void timer_setup(){ //Funcion de configuracion del timer
  TCCR0A=0x00;   //Se usa el modo normal de operacion del timer
  TCCR0B=0x00;
  TCCR0B |= (1<<CS00)|(1<<CS02);   //prescaling usando el 1024
  sei();
  TCNT0=0;
  TIMSK|=(1<<TOIE0); //habilitando la interrupcion en TOIE0
}

void setup_boton(){// funcion de configuracion de los puertos del mcu
  DDRB = (1<<DDB4)|(1<<DDB3)|(1<<DDB2)|(1<<DDB1)|(1<<DDB0); // configuracion de los puertos de salida (LED)
  GIMSK |= (1<<INT1);     // habilitando en INT1 (external interrupt) 
  MCUCR |= (1<<ISC11);    // se configura con flanco negativo del reloj
  PORTB &= (0<<PB0)|(0<<PB1)|(0<<PB2)|(0<<PB3)|(0<<PB4); // Como buena practica es necesario iniciar con todos los pines en cero. 
}

//Motor de la maquina de estados
void engine_fsm(){
  switch (estado){
    case CA:
      (fsm[estado].stateptr)(); // pone las luces  de carros avanzando 
      if (boton && sec >= fsm[estado].time){
        estado = fsm[estado].next[boton]; // si hay boton y ademan pasaron 10 segundos, pase de estado
        intr_count = 0;
        sec = 0;
        msec =0;
      }
      else{
        estado = CA; // si no hay bonton quedese en el estado de carros avanzando
      }
      break;

    case BVR:
      if (msec == fsm[estado].time){ // despues del tiempo definido pase de estado
        estado = fsm[estado].next[boton];
        intr_count = 0;
        sec = 0;
        msec =0;
      }
      else{
        estado = BVR; // si no se cumple la condicion de tiempo siga parpadeando
      }
      break;
    
    case SC:
      (fsm[estado].stateptr)(); // pone la luz en amarrillo
      if (sec == fsm[estado].time){ // si pasaron 2 segundos pase de estado
        estado = fsm[estado].next[boton];
        intr_count = 0;
        sec = 0;
        msec =0;
      }
      break;

    case PA:
      (fsm[estado].stateptr)(); // pone la luz para los peatones 
      if (sec == fsm[estado].time){ // si ppasa el tiempo para los peatones pase de estado
        estado = fsm[estado].next[boton];
        intr_count = 0;
        sec = 0;
        msec =0;
      }
      break;

    case BRV:
      if (msec == fsm[estado].time){// parpadea antes de volver al estado de carros avanzando
        estado = fsm[estado].next[boton]; 
        intr_count = 0;
        sec = 0;
        msec =0;
        boton = OFF; // devuelve el sistema a su estado inicial
      }
      else{
        estado = BRV; 
      }
      break;

    default:
      break;
  }
}

// Interrupt service routine
ISR (INT1_vect){        // Interrupt service routine     
  boton = ON;
}

ISR (TIMER0_OVF_vect){      //Interrupt vector for Timer0
  if (intr_count == 20){  //cuenta un tercio de segundo 
    if( estado == BVR ){ 
      (fsm[BVR].stateptr)(); // parpadear
    }
    else if( estado == BRV){
      (fsm[BRV].stateptr)(); // parpadear
    }
    ++msec; //contador de tercios de segundo
  }
  if (intr_count == 60){    // cuenta un segundo 
    intr_count = 0;
    ++sec; // cuenta un segundo 
  }
  else intr_count++;
}

int main(void){
  setup_boton();
  timer_setup();
  estado = CA;  // Estado inicial de la maquina, carros avanzando
  while (1) {
    engine_fsm();
  }
}