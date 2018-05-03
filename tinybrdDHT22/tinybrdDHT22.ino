#include <SPI.h>
#include <Radio.h>
#include <Battery.h>
#include <dht.h>

dht DHT22;

#define RETRIES_LIMIT 5
#define SLEEP_TIME 60000
#define DHT22PIN 1
struct Data 
{
  byte tinyId;
  unsigned long batteryCharge;
  double temperature;
  double humidity;
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
  delay(1000);
  int chk = DHT22.read(DHT22PIN);
  switch (chk)
  {
    case DHTLIB_OK:
      data.temperature = DHT22.temperature;
      data.humidity = DHT22.humidity;
      break;
    case DHTLIB_ERROR_CHECKSUM:
      data.temperature = -1;
      data.humidity = -1;
      break;
    case DHTLIB_ERROR_TIMEOUT:
      data.temperature = -2;
      data.humidity = -2;
      break;
    default:
      data.temperature = -3;
      data.humidity = -3;
      break;
  }  

  
  //sending data;
  sendData(data);
  
  //go to power saving mode
  Radio.off();
  sleep(SLEEP_TIME);

}




