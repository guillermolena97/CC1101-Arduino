#include <avr/sleep.h>
#include <EnableInterrupt.h>
#include <cc1100_arduino.h>

uint8_t rxbuffer[FIFOBUFFER];
uint8_t pktlen, My_addr, sender;
uint16_t num_paquetes;
volatile uint8_t cc1101_packet_available;

//--------------------------[class constructors]-----------------------------
//init CC1100 constructor
CC1100 cc1100;

void setup() {
  //cc1100.reset();
  // put your setup code here, to run once:
  Serial.begin(9600);Serial.println("Iniciando set up ... ");
  
  // init CC1101 RF-module and get My_address from EEPROM
  cc1100.begin(My_addr);                   //inits RF module with main default settings
  
  cc1100.sidle();                          //set to ILDE first
  cc1100.set_mode(0x04);                   //set modulation mode 1 = GFSK_1_2_kb; 2 = GFSK_38_4_kb; 3 = GFSK_100_kb; 4 = MSK_250_kb; 5 = MSK_500_kb; 6 = OOK_4_8_kb
  cc1100.set_ISM(0x02);                    //set frequency 1=315MHz; 2=433MHz; 3=868MHz; 4=915MHz
  cc1100.set_channel(0x01);                //set channel
  cc1100.set_output_power_level(0);        //set PA level in dbm
  cc1100.set_myaddr(0x02);                 //set my own address
  
  cc1100.receive();                        //set to RECEIVE mode

  sender = 0x01;


  Serial.println(" ... set up finalizado");
  enableInterrupt(GDO2, rf_available_int, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Comprobando si se reciven paquetes en el interrupt");

  delay(5000);

}

void rf_available_int(void) 
{
  disableInterrupt(GDO2);
  Serial.println("packet available");

  if(cc1100.packet_available() == TRUE)
  {
    //cc1100.check_tc_ber(rxbuffer, pktlen, My_addr, sender, num_paquetes);
    if(cc1100.rx_payload_burst(rxbuffer, pktlen) == TRUE) //stores the payload data to Rx_fifo
    {
        cc1101_packet_available = TRUE;                                //set flag that a package is in RX buffer
        cc1100.sent_acknowledge(My_addr, rxbuffer[2]);
    }
    else
    {
        cc1101_packet_available = FALSE;                               //set flag that an package is corrupted
    }
  }
  
  enableInterrupt(GDO2, rf_available_int, RISING); 
}
