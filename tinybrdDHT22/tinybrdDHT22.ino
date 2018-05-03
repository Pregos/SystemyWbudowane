#include <SPI.h>
#include <Radio.h>
#include <Battery.h>


#define RETRIES_LIMIT 5
#define SLEEP_TIME 1000

struct Data 
{
  byte tinyId;
  unsigned long batteryCharge;
  float temperature;
  float humidity;
  byte retries;  //number of retries
  unsigned long lostData;  //how many Data structures lost
} data;

//address of master NRF24L01
byte remoteAddress[5] = {0, 0, 3};

//should be unique for every tinybrd ???
byte localAddress[5] = {1, 0, 0};

void setup() 
{
  //should be unique for every tinybrd
  data.tinyId = 100;
  
  data.lostData = 0;
  
  //NRF24L01 initialization
  Radio.begin(localAddress, 100); 
}


void sendData (struct Data &data, byte retries = 0)
{
  if (retries == RETRIES_LIMIT) 
  {
    //failed transmission
    data.lostData++;
    return;
  }
  data.retries = retries;
  Radio.write(remoteAddress, data);
  while(true)
  {
    switch(Radio.flush()){ //waiting until data transmition confirmed or data transmition lost
      case RADIO_SENT:
        //data sent
        return;
      case RADIO_LOST:
        sendData(data, retries++);
        return;
    }
  } 
}

void loop() 
{
  data.batteryCharge = batteryRead(); //read battery load
  
  //sending data;
  sendData(data);
  
  //go to power saving mode
  Radio.off();
  sleep(SLEEP_TIME);

}




