//--------------------[ Tx BER ]------------------------

#include <avr/sleep.h>
#include <EnableInterrupt.h>
#include <cc1100_arduino.h>

uint8_t Tx_fifo[FIFOBUFFER], Rx_fifo[FIFOBUFFER];
uint8_t My_addr, Tx_addr, Rx_addr, Pktlen, Lqi, Rssi;
uint8_t rx_addr,sender,lqi, pktlen;
 int8_t rssi_dbm;
volatile uint8_t cc1101_packet_available;
uint16_t paquetes_enviados = 0;
uint8_t continuar = FALSE;

uint16_t num_paquetes = 200;

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

  //enableInterrupt(GDO2, rf_available_int, RISING);

  Rx_addr = 0x02;
  Serial.println(F("Set up finalizado"));
  
}

void loop() {
  //detachPinChangeInterrupt(GDO2);
  if(cc1100.send_tc_ber(My_addr, Rx_addr, num_paquetes))
  {
    Serial.println(F("Telecomando aceptado, procediendo a enviar paquetes"));
    continuar = TRUE;
  }
  else{
    Serial.println(F("Telecomando denegado o no recibido, se repitira el proceso"));
  }
  //attachPinChangeInterrupt(GDO2, rf_available_int, RISING);

  Pktlen = 60;

  delay(2000);
  
  if(continuar == TRUE){
    while(paquetes_enviados<num_paquetes)
    {
      //detachPinChangeInterrupt(GDO2);
      cc1100.send_ber_packet(Pktlen);
      //attachPinChangeInterrupt(GDO2, rf_available_int, RISING);
  
      
  
      delay(300);
      
      Serial.print(F("Enviado paquete: "));Serial.println(paquetes_enviados);
      
      paquetes_enviados++;
    }
    continuar = FALSE;
  }

  //exit(0);
  
}

//void rf_available_int(void) 
//{
//  disableInterrupt(GDO2);
//  
//  if(cc1100.packet_available() == TRUE){
//    if(cc1100.get_payload(Rx_fifo, pktlen, rx_addr, sender, rssi_dbm, lqi) == TRUE) //stores the payload data to Rx_fifo
//    {
//        cc1101_packet_available = TRUE;                                //set flag that a package is in RX buffer
//    }
//    else
//    {
//        cc1101_packet_available = FALSE;                               //set flag that an package is corrupted
//    }
//  }
//  
//  enableInterrupt(GDO2, rf_available_int, RISING); 
//}
