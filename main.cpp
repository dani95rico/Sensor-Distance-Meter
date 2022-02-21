#include "mbed.h"


//PINES DEL MICROPROCESADOR ASOCIADOS A LOS LEDS, DISPLAYS Y BOTONES----------------------------------------------
DigitalOut d1(p25); //CREAMOS DISPLAY 1 (DISPLAY DE LA IZQUIERDA)
DigitalOut d2(p10); // CREAMOS DISPLAY 2  (DISPLAY DE LA DERECHA)

DigitalOut led1(p30);  // CREAMOS LED1 (LED DE LA IZQUIERDA) Y LO ASOCIAMOS AL PIN 29
DigitalOut led2(p28);   // CREAMOS LED2 (LED DEL CENTRO) Y LO ASOCIAMOS AL PIN 28
DigitalOut led3(p27);   // CREAMOS LED3 (LED DE LA DERECHA) Y LO ASOCIAMOS AL PIN 27

InterruptIn boton1(p17);  // CREAMOS BODOTON1 (BOTON DE LA IZQUIERDA) Y LO ASOCIAMOS AL PIN 17
InterruptIn boton2(p15);  // CREAMOS BOTON2 (BOTON DEL CENTRO) Y LO ASOCIAMOS AL PIN 16
InterruptIn boton3(p11);  // CREAMOS BOTON3 (BOTON DE LA DERECHA) Y LO ASOCIAMOS AL PIN 15

InterruptIn echo(p6);
DigitalOut trigger(p26);

Serial pc(USBTX, USBRX);
//------------------------------------------------------------------------------------------------------------------

//ARRAY QUE CONTIENE LOS SEGMENTOS DEL DISPLAY---------------------------------------------------------------------
BusOut display(p18, p19, p20, p21, p22,p23,p24,p12);    //ARRAY DE LOS SEGMENTOS DEL DISPLAY

unsigned long numeros[]={0x88, 0xBE, 0x11, 0x14, 0x26, 0x44, 0x40, 0x1E, 0x00, 0x06};
//-----------------------------------------------------------------------------------------------------------------

// DEFINICION DE LAS VARIABLES ON Y OFF
#define ON 0
#define OFF 1

// DEFINICION DE LAS VARIABLES ENCENDIDO, APAGADO, MANUAL, AUTOMATICO, CENTIMETROS Y METROS.
#define ENCENDIDO 1
#define APAGADO 2
#define MANUAL 3
#define AUTOMATICO 4
#define METROS 5
#define CENTIMETROS 6

// DEFINIMOS VARIABLES ESTADO, MODO Y ESCALA. EL TIMER SE USA PARA QUE EL DISPOSITIVO SE APAGE SI LLEVA SIN REALIZAR UNA MEDIDA 30 SEGUNDOS
int estado=APAGADO;
int modo;
int escala=METROS;
int decena=0;
int unidad=0;
int medirAux=0;
int bandera;
Timer timer30;
Ticker displays;
// FUNCIONES Y VARIABLES PARA MEDIR CON EL SENSOR---------------------------------------------------------------------------
Timer timerSensor;
int maxDistanciaSensor;
int minDistanciaSensor;
float  distanciaSensor;

void startTimer(){
    timerSensor.start();// COMIENZA EL TIMER 
    } 
    
void stopTimer(){
    timerSensor.stop();// PARO EL TIMER
    }   

void init(){
    echo.rise(&startTimer);// CON EL ECHO RECOGEMOS EL TIEMPO QUE HA TARDADO EN IR Y VOLVER LA SEÑAL.
    echo.fall(&stopTimer);// POR ESO LLAMAMOS AL TIMER CON LAS FUNCIONES DE ARRIBA
    distanciaSensor=-1;// INICIALIZAMOS VARIABLES DEL SENSOR EN ESTA FUNCION
    minDistanciaSensor=2;
    maxDistanciaSensor=400;  
    }
    

    
void medirSensor(){// CON ESTA FUNCION MIDO CON EL SENSOR
    trigger=1;// ENVIO SEÑAL CON EL TRIGGER LA HAGO 1 Y 0 EN UN INTERVALO DE 10 MS
    wait_us(10);
    trigger=0;
    wait_us(10);
    timerSensor.stop();// PARO EL TIMER Y RECOJO EL TIEMPO QUE HA TARDADO EN IR Y VOLVER
    distanciaSensor=timerSensor.read()*1e6 /58;// CALCULO CON LA RELACION DEL TIEMPO Y LA DISTANCIA A CUANTA DISTANCIA ESTA EL OBJETO EN EL QUE REBOTA LA ONDA
    if(distanciaSensor < minDistanciaSensor){
        distanciaSensor=minDistanciaSensor;
        }
    if(distanciaSensor > maxDistanciaSensor){
        distanciaSensor = maxDistanciaSensor;
        }
        timerSensor.reset();// RESETEO TIMER
    }
    
float distanciaMedida(){// CON ESTA FUNCION SE MIDE DIRECTAMENTE Y DEVUELVE UNA DISTANCIA EN CM
    medirSensor();
    return distanciaSensor;
    }
    
void pintarSensor(){// FUNCION QUE SIRVE PARA PINTAR UNIDADES Y DECENAS EN LOS LEDS Y PARA CAMBIAR DE ESCALA.
    if(escala==CENTIMETROS){
        if(distanciaSensor<=99){
                unidad = (int)distanciaSensor % 10;
                decena = ((int)distanciaSensor / 10) % 10;
        }else{
                unidad=9;
                decena=9;
            }
    }else{
        unidad=((int)distanciaSensor/10)%10;
        decena=(int)distanciaSensor/100;
        }
   }
//FIN FUNCIONES Y VARIABLES PARA MEDIR CON EL SENSOR--------------------------------------------------------------------------------------------  
   
void flip(){
        //wait(0.01);
        if(estado==ENCENDIDO){
            //wait(0);
        if(medirAux==1){
        d1=0;
        d2=1;
        display=numeros[unidad];
        wait(0.01);
        d2=0;
        d1=1;
        display=numeros[decena];
        wait(0.005);
        
        }
        if(medirAux==2){
            d1=0;
            d2=1;
            display=numeros[unidad];
            wait(0.01);
            d2=0;
            d1=1;
            display=numeros[decena];
            pintarSensor();
            timer30.reset();
            timer30.start();
            }
        if(medirAux==0){
            d1=0;
            d2=1;
            display=numeros[0];
            wait(0.01);
            d2=0;
            d1=1;
            display=numeros[0];
            wait(0.005);
            }
            init();
            distanciaSensor=distanciaMedida();
            }
        
}

// FUNCION APAGAR------------------------------------------------------------------------------------------------------------
void apagar(){
                
        d1=0;
        d2=0;
        led1=OFF;
        led2=OFF;
        led3=OFF;
        wait(0.25);
        estado=APAGADO;
        modo=MANUAL;
        escala=METROS;
        unidad=0;
        decena=0;
        wait(0.25);
        
        medirAux=0;
        pc.printf("AT+OK\r\n");
        
       
                
}
//----------------------------------------------------------------------------------------------------------------------------


//FUNCION ENCENDER ------------------------------------------------------------------------------------------------------------ 
void encender(){
        led3=ON;//ENCENDEMOS EL LED 3 (DERECHA) INDICA QUE LA ESCALA ESTÁ EN METROS
        wait(0.25);
        estado=ENCENDIDO; //CAMBIAMOS DE ESTADO A ENCENDIDO
        modo=MANUAL;
        escala=METROS;
        bandera=1;
         pc.printf("AT+OK\r\n");
         medirAux=0;
 
}
//--------------------------------------------------------------------------------------------------------------------------------        
 
 
//FUNCION MEDIR-------------------------------------------------------------------------------------------------------------------     
void medir(){
    Timer t;    // USAMOS EL TIMER PORQUE EN ESTE PUNTO HEMOS PULSADO EL BOTON
    t.start();
      // INICIAMOS EL TIMER
    while(boton1){  // SI EL BOTON SIGUE PULSADO...
        if(t.read()>=1.5){  // Y EL TIMER LLEGA SUPERA LOS 1.5 SEGUNDOS MIENTRAS EL BOTON ESTA PULSADO...
            apagar();   // APAGAMOS¡
            }
        }
        t.reset();  // RESETEAMOS EL TIMER
        
    if(estado==ENCENDIDO){  // SI NO HEMOS APAGADO... EL ESTADO ES ENCENDIDO ENTONCES...
            if (modo == MANUAL){   // SI EL MODO ES MANUAL ENCENDEMOS UNA VEZ EL LED 
                    pintarSensor();
                    //led1=ON;//ENCENDEMOS EL LED 1 (IZQUIERDA)
                    //wait(0.2);
                    //led1=OFF;//APAGAMOS EL LED 1 (IZQUIERDA)
                    timer30.reset();    // TIMER QUE INDICA QUE AQUI SE HA REALIZADO SU ULTIMA MEDIDA
                    timer30.start();
                    medirAux=1;
                    wait(0.005);
                    
                   
            }
            if(modo==AUTOMATICO){
                     bandera++;
                     if((bandera%2)==0){
                     medirAux=2;
                     }else{
                         medirAux=0;
                         modo=MANUAL;
                         wait(0.05);
                         led2=OFF;
                         wait(0.05);
                         }
            }  
        }  
}

//---------------------------------------------------------------------------------------------------------------------------------------------       
 

//FUNCION CAMBIOMODO---------------------------------------------------------------------------------------------------------------------------             
void cambioModo(){
    if(estado==ENCENDIDO){
            if(medirAux!=2){
            if(modo==MANUAL){
                    led2=ON;//ENCENDEMOS EL LED 2 (CENTRO)
                    wait(0.25);  
                    modo=AUTOMATICO;//CAMBIAMOS DE MODO A AUTOMÁTICO 
                    medirAux=0;                 
            }else{
                    led2=OFF;//APAGAMOS EL LED 2 (CENTRO)
                    wait(0.25);
                    modo=MANUAL;//CAMBIAMOS DE MODO A MANUAL 
                    medirAux=0;
                }
                }
                
        }
}   
//---------------------------------------------------------------------------------------------------------------------------------------------
    
  
// FUNCION CAMBIOESCALA-----------------------------------------------------------------------------------------------------------------------                     
void cambioEscala(){
   if(estado==ENCENDIDO){
            if(medirAux!=2){
            if( escala ==METROS){
                    led3=OFF;
                    wait(0.25);
                    escala = CENTIMETROS;
            }else{    
                    led3=ON;
                    wait(0.25);
                    escala = METROS;
                }
                }
                
        }
}
//------------------------------------------------------------------------------------------------------------------------------------------------

char buffer[50];
char *comparar;
int valor; 

void interrupcion(){
    pc.scanf("%s", buffer);
    
    comparar="AT+START=0";
    if((buffer!=NULL)&&(buffer!='\0')){
               valor=strcmp(comparar,buffer);
               if(valor==0){
                    apagar();
                   }
            }
            
    comparar="AT+START=1";
    if((buffer!=NULL)&&(buffer!='\0')){
               valor=strcmp(comparar,buffer);
               if(valor==0){
                   encender();
                   }          
            }
            
    comparar="AT+ESC=0";
    if((buffer!=NULL)&&(buffer!='\0')){
               valor=strcmp(comparar,buffer);
               if(valor==0){
                   if(escala==METROS){
                       cambioEscala();
                       }
                       pc.printf("AT+OK \r\n");
                   }          
            }   
    comparar="AT+ESC=1"; 
     if((buffer!=NULL)&&(buffer!='\0')){
               valor=strcmp(comparar,buffer);
               if(valor==0){
                   if(escala==CENTIMETROS){
                       cambioEscala();
                       }
                       pc.printf("AT+OK \r\n");
                   }          
            }   
     
     comparar="AT+ESC?";       
     if((buffer!=NULL)&&(buffer!='\0')){
               valor=strcmp(comparar,buffer);
               if(valor==0){
                   pc.printf("AT+OK \r\n");
                   if(escala==CENTIMETROS){
                       pc.printf("AT+ESC=0 \r\n");
                       }
                    if(escala==METROS){
                        pc.printf("AT+ESC=1 \r\n");
                        }
                   }          
            }  
      
      comparar="AT+GET?";
      if((buffer!=NULL)&&(buffer!='\0')){
               valor=strcmp(comparar,buffer);
               if(valor==0){
                   medir();
                   pc.printf("AT+OK \r\n");
                   if(estado==ENCENDIDO){
                   if(escala==CENTIMETROS){
                   pc.printf("AT+GET = %f cm\r\n", distanciaSensor);
                    }
                   if(escala==METROS){
                   pc.printf("AT+GET = %f m\r\n", distanciaSensor/100);   
                    }
                    timer30.reset();
                    timer30.start();
                    }else{
                        pc.printf("Dispositivo apagado, pulse AT+START=1 para encender\r\n");
                        
                        }
                    
                }          
        }   
            
                      
            
}







// MAIN ------------------------------------------------------------------------------------------------------------------------------------------
int main() {
     // INICIALIZAMOS LOS 3 LEDS AL ESTADO OFF, EL ESTADO A APAGADO, EL MODO A MANUAL Y LA ESCALA A METROS
    led1=OFF;
    led2=OFF;
    led3=OFF;
    displays.attach(&flip,0.02);
    pc.baud(115200);
    pc.attach(&interrupcion);
    while(1){

        if(medirAux==2){
            led1=ON;
            wait(0.25);
            led1=OFF;
            }

        switch(estado){
            
            case APAGADO:
                d1=0;
                d2=0;
                 boton1.rise(&encender);
            break;
        
            case ENCENDIDO:
                    timer30.reset();    // INICIO TIMER A 0
                    timer30.start();    // COMIENZA A CONTAR...
                    wait(0.15);
                    
                    while(timer30.read()>=0.1){     // MIENTRAS EL TIMER ESTA CONTANDO... ESPERA LOS BOTONES
                   
                    boton1.rise(&medir);
                    boton2.fall(&cambioModo);  
                    boton3.fall(&cambioEscala);
                        
                        if(timer30.read()>=30){      // SI NO SE PULSA EL BOTON MEDIR EL TIMER NO SE RESETEA... EL TIMER SE RESETEA AL ENCENDER Y CADA VEZ QUE MIDE 
                            apagar();               // SI NO SE RESETEA A LOS 30 SEGUNDOS ENTRA EN ESTADO DE APAGADO
                            timer30.reset();        // RESET DEL TIMER
                            }
                    }
                    
            break;
        }
    }
}
