#include "SparkFun_SHTC3.h"

#define ABP_PERIOD   (10000)
#define CFM false 
#define RETY 0


/* uncoment to set keys when programming*/
//#define ABP_DEVADDR  {0x, 0x, 0x, 0x}
//#define ABP_APPSKEY  {0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x}
//#define ABP_NWKSKEY  {0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x, 0x}

SHTC3 rak1901;

uint8_t ABP_BAND = 6;
uint16_t ABP_MASK = 0x0001;

float Temp, Hum;

/** Packet buffer for sending */
uint8_t collected_data[64] = { 0 };

void recvCallback (SERVICE_LORA_RECEIVE_T * data)
{
  if (data->BufferSize > 0)
  {
    Serial.println ("Something received!");
    for (int i = 0; i < data->BufferSize; i++)
    {
      Serial.printf ("%x", data->Buffer[i]);
    }
    Serial.print ("\r\n");
  }
}

void sendCallback (int32_t status)
{
  if (status == 0)
  {
    Serial.println ("Successfully sent");
  }
  else
  {
    Serial.println ("Sending failed");
  }
}

void setup_lorawan()
{
  Serial.begin(115200, RAK_AT_MODE);
  delay(300);

  Serial.println ("RAKwireless LoRaWan ABP Example");
  Serial.println ("------------------------------------------------------");

  /* uncoment to set keys when programming*/
  /*
  uint8_t node_dev_addr[4] = ABP_DEVADDR;
  uint8_t node_app_skey[16] = ABP_APPSKEY;
  uint8_t node_nwk_skey[16] = ABP_NWKSKEY;

  api.lorawan.daddr.set(node_dev_addr, 4);
  api.lorawan.appskey.set(node_app_skey, 16);
  api.lorawan.nwkskey.set(node_nwk_skey, 16);
*/

  api.lorawan.band.set(ABP_BAND);
  api.lorawan.mask.set(&ABP_MASK);
  api.lorawan.deviceClass.set(RAK_LORA_CLASS_A);
  api.lorawan.njm.set(RAK_LORA_ABP);

  api.lorawan.adr.set(true);
  api.lorawan.dr.set(5);
  api.lorawan.cfm.set(CFM);
  api.lorawan.rety.set(RETY);

  /** Check LoRaWan Status*/
  Serial.printf ("Duty cycle is %s\r\n", api.lorawan.dcs.get ()? "ON" : "OFF");	// Check Duty Cycle status
  Serial.printf ("Packet is %s\r\n", api.lorawan.cfm.get ()? "CONFIRMED" : "UNCONFIRMED");	// Check Confirm status
  uint8_t assigned_dev_addr[4] = { 0 };
  api.lorawan.daddr.get (assigned_dev_addr, 4);
  Serial.printf ("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]);	// Check Device Address
  Serial.printf ("Uplink period is %ums\r\n", ABP_PERIOD);
  Serial.println ("");
  api.lorawan.registerRecvCallback (recvCallback);
  api.lorawan.registerSendCallback (sendCallback);
}

void errorDecoder(SHTC3_Status_TypeDef message)                             // The errorDecoder function prints "SHTC3_Status_TypeDef" resultsin a human-friendly way
{
  switch(message)
  {
    case SHTC3_Status_Nominal : Serial.println("OK"); break;
    case SHTC3_Status_Error : Serial.println("Error"); break;
    case SHTC3_Status_CRC_Fail : Serial.println("CRC Fail"); break;
    default : Serial.println("Unknown return code"); break;
  }
}

void setup ()
{

  Wire.begin();
  errorDecoder(rak1901.begin());
  setup_lorawan();

    if (api.system.timer.create(RAK_TIMER_0, (RAK_TIMER_HANDLER)uplink_routine, RAK_TIMER_PERIODIC))
  {
    api.system.timer.start(RAK_TIMER_0, ABP_PERIOD, NULL);
  }

    delay(1000);

}

void uplink_routine ()
{
  /** Payload of Uplink */
  uint8_t data_len = 0;
  uint8_t tx_buff[30]; 

  rak1901.update();

  Temp = rak1901.toDegC();
  Hum = rak1901.toPercent();
  
  data_len = sprintf ((char*)tx_buff, "T= %.2f H= %.2f",Temp, Hum);

  Serial.println("Data Packet:");
 
  for (int i = 0; i < data_len; i++)
  {
    Serial.printf("0x%02X ", tx_buff[i]);
  }
  Serial.println("");

  /** Send the data package */
  if (api.lorawan.send (data_len, (uint8_t *) &tx_buff, 2))
  {
    Serial.println ("Sending is requested");
  }
  else
  {
    Serial.println ("Sending failed");
  }
}

void loop ()
{

  api.system.sleep.all();

}
