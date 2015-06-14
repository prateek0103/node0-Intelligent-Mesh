#include <MySensor.h>  
#include <SPI.h>
#include <DHT.h>

unsigned long SLEEP_TIME = 10000; // Sleep time between reports (in milliseconds)
#define DIGITAL_INPUT_SENSOR 2   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
#define INTERRUPT DIGITAL_INPUT_SENSOR-2 // Usually the interrupt = pin -2 (on uno/nano anyway)
#define HUMIDITY_SENSOR_DIGITAL_PIN 3
#define LIGHT_SENSOR_ANALOGUE_PIN A0

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID 2   
#define CHILD_ID_LIGHT 3

MySensor gw;
DHT dht;
float lastTemp;
float lastHum;
float lastLight;
boolean metric = true;
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msg(CHILD_ID, V_TRIPPED);
MyMessage msgLight(CHILD_ID_LIGHT, V_LIGHT);

void setup()  
{  
  gw.begin();
dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN);
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Climate", "1.1");
  //gw.sendSketchInfo("Motion Sensor", "1.0");

  pinMode(DIGITAL_INPUT_SENSOR, INPUT);      // sets the motion sensor digital pin as input
  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID, S_MOTION);
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
  
  metric = gw.getConfig().isMetric;
}

void loop()     
{     
  // Read digital motion value
  delay(dht.getMinimumSamplingPeriod());
  float temperature = dht.getTemperature();
   if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    gw.send(msgTemp.set(temperature, 1));
    Serial.print("T: ");
    Serial.println(temperature);
  }
  
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
      lastHum = humidity;
      gw.send(msgHum.set(humidity, 1));
      Serial.print("H: ");
      Serial.println(humidity);
  }
  boolean tripped = digitalRead(DIGITAL_INPUT_SENSOR) == HIGH; 
        
  Serial.println(tripped);
  gw.send(msg.set(tripped?"1":"0"));  // Send tripped value to gw 
 
  int currentLight=analogRead(LIGHT_SENSOR_ANALOGUE_PIN);
  if (lastLight!=currentLight){
    gw.send(msgLight.set(currentLight,1));
    lastLight=currentLight;
  }
  // Sleep until interrupt comes in on motion sensor. Send update every two minute. 
  gw.sleep(INTERRUPT,CHANGE, SLEEP_TIME);
}


