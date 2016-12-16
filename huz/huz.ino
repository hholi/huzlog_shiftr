// This example uses an Adafruit Huzzah ESP8266 or NodeMCU
// to connect to shiftr.io.
//
// See messages here: https://shiftr.io/hholi/house-surveillance


// Build for board NodeMCU 1.0
// or Feather Huzzah 
//WiFi and MQTT
// add the library MQTT by Joel Gaewhiler
#include <ESP8266WiFi.h>
#include <MQTTClient.h>

// The secrets contained in separate file
#include "secrets.h"

// SET to 1 for Huzzah ESP8266 breakout, when connected SDA=#4, SCL=#5
#define ESP8266 1

// SET to 1 only when deep sleeping 30 sec between
// Then pin for chip reset must also be connected!
#define DEEPSLEEP 0
#define SLEEP_SEC 3600

// SET TO 0 for SENSOR BOARD !!!!
#define OLED 0
#define SUBSCRIBE 0

// SET to 1 only if you want a blink each time sensors are read
#define SENSOR_BLINK 1

//Sensors
#include <Wire.h>
#include "Adafruit_HDC1000.h"

//OLED display
// add the library U8g2 by oliver
#include "U8g2lib.h" 

// Connect Vin to 3-5VDC
// Connect GND to ground
// Connect SCL to I2C clock pin (A5 on UNO, D1 on NodeMCU)
// Connect SDA to I2C data pin (A4 on UNO, D2 on NodeMCU)

// OLED will not work without terminating like this
// Connect SCL via 10k to Vin
// Connect SDA via 10k to Vin

Adafruit_HDC1000 hdc = Adafruit_HDC1000();


int ssidIndex = 0;
int timer = 0;


WiFiClientSecure net;
MQTTClient client;

#if OLED
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA);
#endif

unsigned long lastMillis = 0;

char buf[100];

void connect(); // <- predefine connect() for setup()

bool hdcExist = false;


void setup() {

#if SENSOR_BLINK
    pinMode(2, OUTPUT);     
    pinMode(3, OUTPUT);
#endif
    
#if ESP8266
  Wire.pins(4, 5);   // ESP8266 can use any two pins, such as SDA to #4 and SCL to #5  
#endif 
  
  Serial.begin(9600);
#if OLED
  u8g2.begin();
  u8g2.firstPage();

  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(1,8,"connecting...");
  u8g2.nextPage();
#endif
  
  WiFi.begin(ssids[ssidIndex][0], ssids[ssidIndex][1]);
  client.begin("broker.shiftr.io", 8883, net); // MQTT brokers usually use port 8883 for secure connections

  connect();

#if OLED
  u8g2.clear();
  u8g2.drawStr(1,8,"ok: ");
  u8g2.nextPage();
  delay(500);
  
  u8g2.clear();
  u8g2.drawStr(1,8,"HDC100x test...");
  u8g2.nextPage();
#endif

  Serial.println("HDC100x test");

  int retries = 0;
  while (!hdc.begin() && retries++ < 5) {
    Serial.println("Couldn't find sensor!");
    //while (1);
    yield(); // Do (almost) nothing -- yield to allow ESP8266 background functions
    delay(500);
  }
  if (5 > retries) {
    hdcExist = true;
  }else {
    Serial.println("Simulating sensors");    
  }

#if OLED
  u8g2.clear();
  u8g2.drawStr(1,8,"HDC100x ok");
  u8g2.nextPage();
  delay(500);
#endif

}

void connect() {
  Serial.print("checking wifi ...");
  Serial.print(ssids[ssidIndex][0]);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    timer++;
    if(timer >= 10) {
      WiFi.disconnect();
      Serial.print("Trying next ssid: ");
      ssidIndex++;
      ssidIndex = ssidIndex % MAX_SSID;
      Serial.print(ssids[ssidIndex][0]);
      WiFi.begin(ssids[ssidIndex][0], ssids[ssidIndex][1]);
      timer=0;
    }
    
  }

  Serial.print("\nconnecting...");
  while (!client.connect("ClientId", "user", "password")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

#if SUBSCRIBE
  client.subscribe("/hholi/smibakken/house1/floor0/temperature");
#endif

}

void loop() {
  client.loop();
  delay(10); // <- fixes some issues with WiFi stability

  if(!client.connected()) {
    connect();
  }

  delay(2000);
  
  if(millis() - lastMillis > 10000) {
    lastMillis = millis();

    float h;
    float t;
    if (hdcExist) {
      // Read the sensors
      h = hdc.readHumidity();
      t = hdc.readTemperature();
    } else {
      // Simulate sensors
      h = 49.9;
      t = 42.42;
    }

#if SENSOR_BLINK
    digitalWrite(2, LOW);  // switch LED on 
    digitalWrite(3, HIGH);  // switch LED on 
    delay(40);            // keep LED on for 40 millis
    digitalWrite(2, HIGH);   // switch LED off
    digitalWrite(3, LOW);   // switch LED off
#endif
    
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    if (hdcExist) {

#if OLED
      u8g2.clear();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(1,8,"Telemetry active");
      u8g2.setFont(u8g2_font_ncenB12_tr);
      String message = "Temp: " + String(t);
      message.toCharArray(buf, 100);
      u8g2.drawStr(0,28,buf);
      message = "Hum: " + String(h);
      message.toCharArray(buf, 100);
      u8g2.drawStr(0,44,buf);
      u8g2.nextPage();
      
#endif

      client.publish("/hholi/smibakken/house1/floor0/temperature", String(t));
      client.publish("/hholi/smibakken/house1/floor0/humidity", String(h));
      client.publish("/hholi/smibakken/house1/floor0/smoke", "no");
      client.publish("/hholi/smibakken/house2/floor1/temperature", String(t));
      client.publish("/hholi/smibakken/house2/floor1/humidity", String(h));
    }

#if DEEPSLEEP
    Serial.print("Sleeping ");
    ESP.deepSleep(1000000 * SLEEP_SEC);
#endif
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();

  if (!hdcExist) {
#if OLED
    u8g2.clear();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(1,8,"Monitoring mode");
    String message = topic;
    message.toCharArray(buf, 100);
    u8g2.drawStr(0,24,buf);
    message =  payload;
    message.toCharArray(buf, 100);
    u8g2.setFont(u8g2_font_ncenB12_tr);
    u8g2.drawStr(0,40,buf);
    
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    u8g2.nextPage();
#endif    
  }
  
}
