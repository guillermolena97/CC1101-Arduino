#include <avr/sleep.h>
#include <EnableInterrupt.h>
#include <cc1100_arduino.h>

#define DEBUG(a) Serial.print(a);

uint8_t txbuffer[FIFOBUFFER], txbuffer2[FIFOBUFFER];
uint8_t My_addr, Rx_addr, pktlen, pktlen_tc;
uint32_t num_secuencia;
uint32_t num_paquetes;
int packets_sent;
uint8_t dummy_data;
char ready_PER;

//--------------------------[class constructors]-----------------------------
//init CC1100 constructor
CC1100 cc1100;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);Serial.println("Iniciando set up ... ");

  //cc1100.reset();

  // init CC1101 RF-module and get My_address from EEPROM
  cc1100.begin(My_addr);                   //inits RF module with main default settings
  
  cc1100.sidle();                          //set to ILDE first
  cc1100.set_mode(0x04);                   //set modulation mode 1 = GFSK_1_2_kb; 2 = GFSK_38_4_kb; 3 = GFSK_100_kb; 4 = MSK_250_kb; 5 = MSK_500_kb; 6 = OOK_4_8_kb
  cc1100.set_ISM(0x02);                    //set frequency 1=315MHz; 2=433MHz; 3=868MHz; 4=915MHz
  cc1100.set_channel(0x01);                //set channel
  cc1100.set_output_power_level(0);        //set PA level in dbm //Patable index: -30  -20 -15  -10   0    5    7    10 dBm
  cc1100.set_myaddr(0x01);                 //set my own address
  
  cc1100.receive();                        //set to RECEIVE mode

  Rx_addr = 0x02;
  num_paquetes = 500;
  pktlen_tc = 10;
  pktlen = 9;

  dummy_data = 0b01010101;

  txbuffer[3] = 'P'; txbuffer[4] = 'E'; txbuffer[5] = 'R'; txbuffer[6] = (num_paquetes>>24 & 0xFF); txbuffer[7] = (num_paquetes>>16 & 0xFF); txbuffer[8] = (num_paquetes>>8 & 0xFF); txbuffer[9] = (num_paquetes & 0xFF);

  Serial.println(" ... set up finalizado");
  Serial.setTimeout(500);
}

void loop() {
  //pedir por teclado un y/n para empezar el calculo de la PER
  if (Serial.available())
  {
    ready_PER = Serial.read();
    DEBUG((char)ready_PER);
  }

  if (ready_PER == 'y')
  {
    //Serial.println(My_addr);
    cc1100.sent_packet(My_addr, Rx_addr, txbuffer, pktlen_tc, 40, TRUE); //true para esperar un ack
  
    Serial.println("... Preparando PER ...");
    
    delay(2000);
  
    num_secuencia = 1;
    packets_sent = 0;
  
    txbuffer2[3] = (num_secuencia>>24 & 0xFF); txbuffer2[4] = (num_secuencia>>16 & 0xFF); txbuffer2[5] = (num_secuencia>>8 & 0xFF); txbuffer2[6] = (num_secuencia & 0xFF);
    txbuffer2[7] = dummy_data; txbuffer2[8] = dummy_data;
  
    while(packets_sent<num_paquetes)
    {
      cc1100.sent_packet(My_addr, Rx_addr, txbuffer2, pktlen, 40, FALSE); //false para no esperar un ack
      
      packets_sent++;
      num_secuencia++;
  
      txbuffer2[3] = (num_secuencia>>24 & 0xFF); txbuffer2[4] = (num_secuencia>>16 & 0xFF); txbuffer2[5] = (num_secuencia>>8 & 0xFF); txbuffer2[6] = (num_secuencia & 0xFF);
  
      delay(100); //100 minimo??
   }
  
    Serial.println("Se ha finalizado de enviar los paquetes para calcular la PER");

    ready_PER = 'n';
  
    //delay(60000); //1 minuto (aun hay q ver como hacer terminar el progrrama)
  }
  
}
