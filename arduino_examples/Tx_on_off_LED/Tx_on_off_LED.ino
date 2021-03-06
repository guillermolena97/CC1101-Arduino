//--------------------[ Encender/apagar LED ]------------------------
#define DEBUG(a) Serial.println(a);

#include <avr/sleep.h>
#include <EnableInterrupt.h>
#include <cc1100_arduino.h>

uint8_t Tx_fifo[FIFOBUFFER], Rx_fifo[FIFOBUFFER];
uint8_t My_addr, Tx_addr, Rx_addr, Pktlen, Lqi, Rssi;
uint8_t rx_addr,sender,lqi, pktlen;
 int8_t rssi_dbm;
volatile uint8_t cc1101_packet_available;

//--------------------------[class constructors]-----------------------------
//init CC1100 constructor
CC1100 cc1100;

void setup() {
  // init serial Port for debugging
  Serial.begin(9600);Serial.println();
  
  // init CC1101 RF-module and get My_address from EEPROM
  cc1100.begin(My_addr);                   //inits RF module with main default settings
  
  cc1100.sidle();                          //set to ILDE first
  cc1100.set_mode(0x04);                   //set modulation mode 1 = GFSK_1_2_kb; 2 = GFSK_38_4_kb; 3 = GFSK_100_kb; 4 = MSK_250_kb; 5 = MSK_500_kb; 6 = OOK_4_8_kb
  cc1100.set_ISM(0x02);                    //set frequency 1=315MHz; 2=433MHz; 3=868MHz; 4=915MHz
  cc1100.set_channel(0x01);                //set channel
  cc1100.set_output_power_level(0);        //set PA level in dbm
  cc1100.set_myaddr(0x01);                 //set my own address
  
  cc1100.receive();                        //set to RECEIVE mode

  enableInterrupt(GDO2, rf_available_int, RISING);
  
  Serial.println("CC1101 LED telecomand demo: ");   //welcome message
  Serial.println("Escribir 1 para encender y 2 para apagar el led");
}

void loop() {
  String str = Serial.readStringUntil('\n');
  int comando = str.toInt();

  Rx_addr = 0x02;
  Pktlen = 0x04;
  
  switch (comando){
    case 1:
      Serial.println("Transmitiendo paquete con el telecomando de encendido ...");
      Tx_fifo[3] = 0xFF;

      detachPinChangeInterrupt(GDO2);
      cc1100.sent_packet(My_addr, Rx_addr, Tx_fifo, Pktlen, 40, TRUE);
      attachPinChangeInterrupt(GDO2, rf_available_int, RISING);

      Serial.println("Telecomando enviado!");
      
      break;
      
    case 2:
      Serial.println("Transmitiendo paquete con el telecomando de apagado ...");
      Tx_fifo[3] = 0x00;
      
      detachPinChangeInterrupt(GDO2);
      cc1100.sent_packet(My_addr, Rx_addr, Tx_fifo, Pktlen, 40, TRUE);
      attachPinChangeInterrupt(GDO2, rf_available_int, RISING);

      Serial.println(F("Telecomando enviado!"));
      
      break;
  }
}

void rf_available_int(void) 
{
  disableInterrupt(GDO2);
  
  if(cc1100.packet_available() == TRUE){
    if(cc1100.get_payload(Rx_fifo, pktlen, rx_addr, sender, rssi_dbm, lqi) == TRUE) //stores the payload data to Rx_fifo
    {
        cc1101_packet_available = TRUE;                                //set flag that a package is in RX buffer
    }
    else
    {
        cc1101_packet_available = FALSE;                               //set flag that an package is corrupted
    }
  }
  
  enableInterrupt(GDO2, rf_available_int, RISING); 
}
