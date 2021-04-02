#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <avr/sleep.h>
#include <EnableInterrupt.h>
#include <cc1100_arduino.h>


uint8_t Tx_fifo[FIFOBUFFER], Rx_fifo[FIFOBUFFER];
uint8_t My_addr, Tx_addr, Rx_addr, Pktlen, Lqi, Rssi;
uint8_t rx_addr,sender,lqi, pktlen;
 int8_t rssi_dbm;
volatile uint8_t cc1101_packet_available;


/*
 * Pines del sensor (de izq a derch) -- arduino
 * pin 1 -> Digital 2 (data)
 * pin 2 -> 5V (Vcc)
 * pin 3 -> GND (masa)
*/
 
#define DEBUG(a) Serial.println(a);
// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11

//--------------------------[class constructors]-----------------------------
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

//init CC1100 constructor
CC1100 cc1100;

void setup() {
  // put your setup code here, to run once:
    // put your setup code here, to run once:
  Serial.begin(9600);Serial.println();
  
  // Comenzamos el sensor DHT
  dht.begin();

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

}

void loop() {
  Rx_addr = 0x02;
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();

  // Comprobamos si ha habido algún error en la lectura
  if (isnan(t)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }

  Serial.println("***********************************");
  Serial.println("Temperatura: " + String(t));
  Serial.println("***********************************");

  String auxString = String(t, 2);
  int aux_len = auxString.length();
  byte payload[sizeof(float)];
  
  auxString.getBytes(payload, aux_len + 1);
  
  for (int i = 0; i < sizeof(float); i++){
    Tx_fifo[i+3] = payload[i];
  }

  Pktlen = sizeof(float) + 3;

  Serial.print("El mensaje a enviar es: ");
  for(int i=0; i < sizeof(float); i++){
    Serial.print(payload[i], HEX);
  }
  Serial.println();
  for(int i=0; i < sizeof(float); i++){
    Serial.print(payload[i]);
  }
  Serial.println();
 
  detachPinChangeInterrupt(GDO2);
  cc1100.sent_packet(My_addr, Rx_addr, Tx_fifo, Pktlen, 40);
  attachPinChangeInterrupt(GDO2, rf_available_int, RISING);  
  
  delay(5000);
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
