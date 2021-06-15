#include <avr/sleep.h>
#include <EnableInterrupt.h>
#include <cc1100_arduino.h>


//--------------------------[Global CC1100 variables]------------------------
uint8_t Tx_fifo[FIFOBUFFER], Rx_fifo[FIFOBUFFER];
uint8_t My_addr, Tx_addr, Rx_addr, Pktlen, pktlen, Lqi, Rssi;
uint8_t rx_addr,sender,lqi;
 int8_t rssi_dbm;
volatile uint8_t cc1101_packet_available;
uint8_t PER_START;
uint32_t num_paquetes, num_secuencia, num_secuencia_esperado;
int paquetes_perdidos, paquetes_erroneos, paquetes_recibidos, RSSI_total, contador_timeout;
float PER, RSSI_media, RSSI_desv;


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
  cc1100.set_myaddr(0x02);                 //set my own address
  
  cc1100.receive();                        //set to RECEIVE mode

  //Inicializar variables:
  PER_START = FALSE;
  cc1101_packet_available = FALSE;
  num_secuencia_esperado = 1;
  paquetes_recibidos = 0;
  paquetes_perdidos = 0;
  paquetes_erroneos = 0;
  pktlen = 8;
  RSSI_total = 0;

  //Init interrrupt function for available packet
  enableInterrupt(GDO2, rf_available_int, RISING); 

  Serial.println(" ... set up finalizado");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(PER_START == TRUE)
  {
    while(contador_timeout<PER_TIMEOUT)
    {
      if(cc1101_packet_available == TRUE) //packet recieved?
      {
        if(Rx_fifo[0] == pktlen) //packet len ok?
        {
          if(Rx_fifo[1] && Rx_fifo[2]) //Rx y Tx addr ok?
          {
            if(cc1100.check_crc(Rx_fifo[pktlen + 2]) == 0x80)
            {
              num_secuencia = (Rx_fifo[3]<<24) | (Rx_fifo[4]<<16) | (Rx_fifo[5]<<8) | (Rx_fifo[6]);
              rssi_dbm = cc1100.rssi_convert(Rx_fifo[pktlen + 1]);
              RSSI_total = RSSI_total + rssi_dbm;
              
              if(num_secuencia >= num_secuencia_esperado)
              {
                if(num_secuencia == num_secuencia_esperado) //num de secuencia ok
                {
                  //Actualizar datos PER
                  num_secuencia_esperado++;
                  paquetes_recibidos++;  
                  
                }
                else //num de secuencia mayor de lo esperado
                {
                  //Actualizar datos PER
                  paquetes_perdidos = num_secuencia - num_secuencia_esperado;
                  num_secuencia_esperado = num_secuencia + 1;
                  paquetes_recibidos++;
                } 
              }
              else
              {
                //new burst?
                num_secuencia_esperado = num_secuencia + 1;
                paquetes_perdidos = num_secuencia - 1;
                paquetes_recibidos = 1;
                
              }
            }
            else
            {
              //Actualizar datos PER
              paquetes_erroneos++;
              paquetes_recibidos++;
              num_secuencia_esperado++;
              
            }
          }
        }
        cc1101_packet_available = FALSE;
      }
      
      contador_timeout++;
      delay(1); //ms
    }
  }
  PER_START = FALSE;
  cc1101_packet_available = FALSE;
  PER = 100*(paquetes_perdidos + paquetes_erroneos)/(paquetes_perdidos + paquetes_recibidos);
  RSSI_media = RSSI_total/paquetes_recibidos;


  Serial.println("****************************************************************************");
  Serial.println("****************************************************************************");
  Serial.print("PER = ");Serial.print(PER);Serial.println(" %");
  Serial.print("RSSI media = ");Serial.print(RSSI_media);Serial.println(" [dBm]");
  //Serial.print("RSSI desviación típica = ");Serial.print(RSSI_desv);Serial.println(" [dBm]");
  Serial.println("****************************************************************************");
  Serial.println("****************************************************************************");
}

void rf_available_int(void) 
{
  disableInterrupt(GDO2);

  if(cc1100.packet_available() == TRUE)
  {
    if(cc1100.get_payload(Rx_fifo, pktlen, rx_addr, sender, rssi_dbm, lqi) == TRUE) //stores the payload data to Rx_fifo
    {
      if(Rx_fifo[3] == 'P' && Rx_fifo[4] == 'E' && Rx_fifo[5] == 'R' && PER_START == FALSE)
      {
        PER_START = TRUE;
        cc1100.sent_acknowledge(My_addr, Rx_fifo[2]);
        num_paquetes = (Rx_fifo[6]<<24) | (Rx_fifo[7]<<16) | (Rx_fifo[8]<<8) | (Rx_fifo[9]);
        Serial.print("El numero de paquetes a recibir es: ");Serial.println(num_paquetes);
      }
      else
      {
        cc1101_packet_available = TRUE;
      }
    }
    else
    {
      cc1101_packet_available = FALSE;
    }
  }
  
  enableInterrupt(GDO2, rf_available_int, RISING); 
}
