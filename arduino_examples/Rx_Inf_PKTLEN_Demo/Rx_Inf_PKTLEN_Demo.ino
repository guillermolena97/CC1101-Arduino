//=============================================================================//
//=========================[ Prueba Rx Inf PKTLEN ]============================//
//=============================================================================//

#include <avr/sleep.h>
#include <EnableInterrupt.h>
#include <cc1100_arduino.h>

#define DEBUG(a) Serial.println(a);

//Inicializamos variables
uint8_t My_addr;
volatile uint8_t cc1101_packet_available;
uint16_t len = 0x3E8;
uint8_t frame_buffer[0x3E8];
int contador = 0;
int i, j;
uint8_t referencia = 0b01010101;

//init CC1100 constructor
CC1100 cc1100;

void setup() {
  Serial.begin(9600);Serial.println();

  // init CC1101 RF-module and get My_address from EEPROM
  cc1100.begin(My_addr);                   //inits RF module with main default settings
  
  cc1100.sidle();                          //set to ILDE first
  cc1100.set_mode(0x04);                   //set modulation mode 1 = GFSK_1_2_kb; 2 = GFSK_38_4_kb; 3 = GFSK_100_kb; 4 = MSK_250_kb; 5 = MSK_500_kb; 6 = OOK_4_8_kb
  cc1100.set_ISM(0x02);                    //set frequency 1=315MHz; 2=433MHz; 3=868MHz; 4=915MHz
  cc1100.set_channel(0x01);                //set channel
  cc1100.set_output_power_level(0);        //set PA level in dbm
  cc1100.set_myaddr(0x02);                 //set my own address
  
  cc1100.receive();                        //set to RECEIVE mode

//SYNC MODE ESTA APAGADO??? HABRA QUE PONER SET_SYNC OFF??

  // init interrrupt function for available packet
  enableInterrupt(GDO2, rf_available_int, RISING);
  
}

void loop() {

  if(cc1101_packet_available == TRUE)
  {
    for(i = 2; i<len; i++){
      for(j=0; j<8; j++){
        //cogemos cada byte y lo vamos comparando bit a bit con la referencia
        //La comparacion la hacemos con un and y moviendo siempre el bit que queremos referenciar a la derecha del todo
        if( ((frame_buffer[i]>>j ) & 0b00000001 ) != ((referencia>>j ) & 0b00000001 )){
          contador+=1;
        }  
      } 
    }
  }
  else{
    printf("Esperando a que lleguen paquetes...  \n");
  }

  delay(5000); //ms
}


void rf_available_int(void) 
{
  disableInterrupt(GDO2);
  
  if(cc1100.packet_available() == TRUE){
    if(cc1100.receive_frame(frame_buffer, &len, 0, 16) == TRUE)
        cc1101_packet_available = TRUE; //set flag that a package is in RX buffer
        
        
    }
    else
    {
        cc1101_packet_available = FALSE; //set flag that an package is corrupted
    }
  
  enableInterrupt(GDO2, rf_available_int, RISING); 
}
