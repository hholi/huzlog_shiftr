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
#define OLED 1
#define SUBSCRIBE 1

// SET to 1 only if you want a blink each time sensors are read
#define SENSOR_BLINK 1

#define MQTT_BROKER             "mqtt.hallgeirholien.no"
#define MQTT_BROKER_PORT        8883
// Only change this if you have a newer certificate fingerprint
// Get certificate using this:
// openssl s_client -servername mqtt.hallgeirholien.no -connect mqtt.hallgeirholien.no:8883 < /dev/null 2>/dev/null | openssl x509 -fingerprint -noout -in /dev/stdin
const char* fingerprint = "07:67:B4:D9:CC:33:53:52:36:14:D4:6E:9B:AD:3E:10:27:CD:A4:48";

//Sensors
#include <Wire.h>
#include "Adafruit_HDC1000.h"

//OLED display
// add the library U8g2 by oliver
#include "U8g2lib.h" 


//WPA EAP
#define WPAEAP 1

extern "C" {
  #include "user_interface.h"
  #include "wpa2_enterprise.h"
}
/*******************************************************************/
#define SERIAL_BAUD_RATE      57600
#define STARTUP_DELAY_MS      1000
/*******************************************************************/
// SSID to connect to
//static const char* ssid = "ssidname";
// Username for authentification
//static const char* username = "identity";
// Password for authentification
//static const char* password = "password";


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
MQTTClient mqttClient;

#if OLED
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA);
#endif

unsigned long lastMillis = 0;

char buf[100];

void connect(); // <- predefine connect() for setup()

bool hdcExist = false;

const char* subscribeToken = "owntracks/#";

void showStatus(const char* msg) {
	Serial.println(msg);
#if OLED
	u8g2.begin();
	u8g2.firstPage();

	u8g2.setFont(u8g2_font_ncenB08_tr);
	u8g2.drawStr(1,8,msg);
	u8g2.nextPage();
#endif

}

void setup() {

#if SENSOR_BLINK
    pinMode(2, OUTPUT);     
    pinMode(3, OUTPUT);
#endif
    
#if ESP8266
  Wire.pins(4, 5);   // ESP8266 can use any two pins, such as SDA to #4 and SCL to #5  
#endif 
  
  Serial.begin(115000);
  showStatus("connecting...");
  WiFi.begin(ssids[ssidIndex][0], ssids[ssidIndex][1]);
  mqttClient.begin("mqtt.hallgeirholien.no", 8883, net); // MQTT brokers usually use port 8883 for secure connections

  connect();

  showStatus("ok");
  delay(500);
  showStatus("HDC100x test...");
//#if OLED
//  u8g2.clear();
//  u8g2.drawStr(1,8,"ok: ");
//  u8g2.nextPage();
//  delay(500);
//
//  u8g2.clear();
//  u8g2.drawStr(1,8,"HDC100x test...");
//  u8g2.nextPage();
//#endif

//  Serial.println("HDC100x test");

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
  if(hdcExist) {
	  u8g2.drawStr(1,8,"HDC100x ok");
  } else {
	  u8g2.drawStr(1,8,"Simulate sensors");
  }
  u8g2.nextPage();
  delay(500);
#endif

}

void verifySecure() {

	WiFiClientSecure client;

  const char* host = MQTT_BROKER;
  String msg = "Connecting to ";
  msg.concat(host);
  showStatus(msg.c_str());
//  Serial.println(host);

  if (! client.connect(host, MQTT_BROKER_PORT)) {
	  showStatus("Connection failed. Halting execution.");
	  delay(2000);
	  while(1);
  }

  // Shut down connection if host identity cannot be trusted.
  if (client.verify(fingerprint, host)) {
	  showStatus("Connection secure.");
	  delay(2000);
  } else {
	  showStatus("Connection insecure! Halting execution.");
	  delay(2000);
    while(1);
  }
}

/* ======================================================================
Function: httpPost
Purpose : Do a http post
Input   : hostname
          port
          url
Output  : true if received 200 OK
Comments: -
====================================================================== */
//boolean httpPost(char * host, uint16_t port, char * url)
//{
//  HTTPClient http;
//  bool ret = false;
//
//  unsigned long start = millis();
//
//  // configure target server and url
//  http.begin(host, port, url, port==443 ? true : false);
//  //http.begin("http://emoncms.org/input/post.json?node=20&apikey=apikey&json={PAPP:100}");
//
//  Debugf("http%s://%s:%d%s => ", port==443?"s":"", host, port, url);
//
//  // start connection and send HTTP header
//  int httpCode = http.GET();
//  if(httpCode) {
//      // HTTP header has been send and Server response header has been handled
//      Debug(httpCode);
//      Debug(" ");
//      // file found at server
//      if(httpCode == 200) {
//        String payload = http.getString();
//        Debug(payload);
//        ret = true;
//      }
//  } else {
//      DebugF("failed!");
//  }
//  Debugf(" in %d ms\r\n",millis()-start);
//  return ret;
//}

void connect() {
//#if WPAEAP
  showStatus("checking wifi EAP ...");
  Serial.begin(SERIAL_BAUD_RATE);
  delay(STARTUP_DELAY_MS);

  // Setting ESP into STATION mode only (no AP mode or dual mode)
  wifi_set_opmode(STATION_MODE);

  struct station_config wifi_config;

  memset(&wifi_config, 0, sizeof(wifi_config));
  strcpy((char*)wifi_config.ssid, ssid);

  wifi_station_set_config(&wifi_config);

  wifi_station_clear_cert_key();
  wifi_station_clear_enterprise_ca_cert();

  wifi_station_set_wpa2_enterprise_auth(1);
  wifi_station_set_enterprise_username((uint8*)username, strlen(username));
  wifi_station_set_enterprise_password((uint8*)password, strlen(password));

  wifi_station_connect();

  // Wait for connection AND IP address from DHCP
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    counter++;
    showStatus(String(counter).c_str());
    Serial.print(".");
  }

  // Now we are connected
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  showStatus(WiFi.localIP().toString().c_str());
  delay(4000);
//#else
//  showStatus("checking wifi ...");
//  Serial.print(ssids[ssidIndex][0]);
//  while (WiFi.status() != WL_CONNECTED) {
//    Serial.print(".");
//    delay(1000);
//    timer++;
//    if(timer >= 10) {
//      WiFi.disconnect();
//      Serial.print("Trying next ssid: ");
//      ssidIndex++;
//      ssidIndex = ssidIndex % MAX_SSID;
//      Serial.print(ssids[ssidIndex][0]);
//      WiFi.begin(ssids[ssidIndex][0], ssids[ssidIndex][1]);
//      timer=0;
//    }
//
//  }
//#endif

  // happymeter
  // CURL -X POST http://localhost:3000/api/storehappydocument -d {"happystatus": "above", "tags": "in the morning"}

//  httpPost();

  verifySecure();

  Serial.print("\nconnecting...");
  while (!mqttClient.connect("ClientId", MQTT_USER, MQTT_TOKEN)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

#if SUBSCRIBE
  mqttClient.subscribe(subscribeToken);
#endif

}

void loop() {
  mqttClient.loop();
  delay(50); // <- fixes some issues with WiFi stability

  if(!mqttClient.connected()) {
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

        mqttClient.publish("rv90b/garage", "{portopen: true}");
//    client.publish("/hholi/site1/house1/floor0/temperature", String(t));
//    client.publish("/hholi/site1/house1/floor0/humidity", String(h));
//    client.publish("/hholi/site1/house1/floor0/smoke", "no");
//    client.publish("/hholi/site1/house2/floor1/temperature", String(t));
//    client.publish("/hholi/site1/house2/floor1/humidity", String(h));
    

#if DEEPSLEEP
    Serial.print("Sleeping ");
    ESP.deepSleep(1000000 * SLEEP_SEC);
#endif
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
//  Serial.print(" - ");
//  Serial.print(payload);
  Serial.println();

  if (!hdcExist) {
#if OLED
    u8g2.clear();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(1,8,"Monitoring mode");
    u8g2.drawStr(0,24,topic.c_str());
    
//    String message = topic;
//    message.toCharArray(buf, 100);
//    u8g2.drawStr(0,24,buf);
//    message =  payload;
//    message.toCharArray(buf, 100);
//    u8g2.setFont(u8g2_font_ncenB12_tr);
//    u8g2.drawStr(0,40,buf);
//
//    u8g2.setFont(u8g2_font_unifont_t_symbols);
//    u8g2.nextPage();
#endif    
  }
  
}

