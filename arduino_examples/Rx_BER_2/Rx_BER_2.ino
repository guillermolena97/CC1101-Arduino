//--------------------[ Rx BER ]------------------------
#define DEBUG(a) Serial.println(a);

#include <avr/sleep.h>
#include <EnableInterrupt.h>
#include <cc1100_arduino.h>

uint8_t Tx_fifo[FIFOBUFFER], Rx_fifo[FIFOBUFFER];
uint8_t My_addr, Tx_addr, Rx_addr, Pktlen, Lqi, Rssi;
uint8_t rx_addr,sender,lqi, pktlen;
 int8_t rssi_dbm;
volatile uint8_t calc_ber_ready;
int contador = 0;
int i, j;
uint8_t referencia = 0b01010101;
float ber;
uint16_t num_paquetes;
int paquetes_recibidos;
int contador_bits_erroneos;

//--------------------------[class constructors]-----------------------------
//init CC1100 constructor
CC1100 cc1100;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);Serial.println();
  
  // init CC1101 RF-module and get My_address from EEPROM
  cc1100.begin(My_addr);                   //inits RF module with main default settings

  Serial.println(F("cc1100.begin() terminado"));

  cc1100.sidle();                          //set to ILDE first
  cc1100.set_mode(0x04);                   //set modulation mode 1 = GFSK_1_2_kb; 2 = GFSK_38_4_kb; 3 = GFSK_100_kb; 4 = MSK_250_kb; 5 = MSK_500_kb; 6 = OOK_4_8_kb
  cc1100.set_ISM(0x02);                    //set frequency 1=315MHz; 2=433MHz; 3=868MHz; 4=915MHz
  cc1100.set_channel(0x01);                //set channel
  cc1100.set_output_power_level(0);        //set PA level in dbm
  cc1100.set_myaddr(0x02);                 //set my own address
  
  cc1100.receive();                        //set to RECEIVE mode

  enableInterrupt(GDO2, rf_available_int, RISING); 
  
  paquetes_recibidos = 0;
  contador_bits_erroneos = 0;
  Serial.println(F("Set up terminado"));
}

void loop() {
  
  if(calc_ber_ready == TRUE){
    disableInterrupt(GDO2);
    while(paquetes_recibidos<num_paquetes)
    {
      if(cc1100.packet_available() == TRUE)
      {
        cc1100.rx_fifo_erase(Rx_fifo);
        cc1100.rx_payload_burst(Rx_fifo, pktlen);
        
        for(int i =1; i<pktlen; i++)
        {
          for(int j=0; j<8; j++)
          {
            if( ((Rx_fifo[i]>>j ) & 0b00000001 ) != ((referencia>>j ) & 0b00000001 ))
            {
              contador_bits_erroneos++;
            }  
          } //loop for para comparar bit a bit
        }//loop for para comparar bit a bit
        paquetes_recibidos++;
      }
    }

    ber = contador_bits_erroneos/1000000;
    Serial.print(F("La ber calculada es: "));Serial.println(ber);
    
    calc_ber_ready = FALSE;
  } 
  enableInterrupt(GDO2, rf_available_int, RISING);
}

void rf_available_int(void) 
{
  disableInterrupt(GDO2);
  
  if(cc1100.packet_available() == TRUE)
  {
    if(cc1100.check_tc_ber(Rx_fifo, pktlen, rx_addr, sender, num_paquetes) == TRUE) //stores the payload data to Rx_fifo
    {
        calc_ber_ready = TRUE;
    }
    else
    {
        calc_ber_ready = FALSE;
    }
  }
  enableInterrupt(GDO2, rf_available_int, RISING); 
}
