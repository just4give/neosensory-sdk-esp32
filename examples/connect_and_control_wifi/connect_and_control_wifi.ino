#include <neosensory_esp32.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>

NeosensoryESP32 NeoESP32;

int motor = 0;
float intensity = 0;
float **rumble_frames;
char ssid[] = "your_wifi_ssid";
char pass[] = "your_wifi_password";
bool jsonStarted_ = false;
String jsonMessage_;

// Set web server port number to 80
AsyncWebServer server(80);


String motor0Slider = "0";
String motor1Slider = "0";
String motor2Slider = "0";
String motor3Slider = "0";
int battery = 0;
String deviceId = "";
String serialNo = "";

const char* PARAM_INPUT_VALUE = "value";
const char* PARAM_INPUT_INDEX = "index";


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html>
<head><meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
.button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
.slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #FFD65C;outline: none; -webkit-transition: .2s; transition: opacity .2s;}
.slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
.slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; }
.button2 {background-color: #555555;} .bat {color: #ffffff;border: 1px solid black;padding: 10px;background: #4caf50;}</style></head>
<body><h1>Neosensory Buzz</h1>
<div class="bat"><h3>Device ID: %DEVICEID%</h3><h3>SERIAL NO: %SERIALNO%</h3></div>
<h3 class="bat">BAT <span>%BATTERY%</span>&#37;</h3>
<p><span>Motor 0</span><input type="range" min="0" max="255" value="%SLIDERVALUE0%" class="slider" id="motor0"  onchange="updateSlider('0')"><span id="motor0Slider">%SLIDERVALUE0%</span></p>
<p><span>Motor 1</span><input type="range" min="0" max="255" value="%SLIDERVALUE1%" class="slider" id="motor1"  onchange="updateSlider('1')"><span id="motor1Slider">%SLIDERVALUE1%</span></p>
<p><span>Motor 2</span><input type="range" min="0" max="255" value="%SLIDERVALUE2%" class="slider" id="motor2"  onchange="updateSlider('2')"><span id="motor2Slider">%SLIDERVALUE2%</span></p>
<p><span>Motor 3</span><input type="range" min="0" max="255" value="%SLIDERVALUE3%" class="slider" id="motor3"  onchange="updateSlider('3')"><span id="motor3Slider">%SLIDERVALUE3%</span></p>
<script>function updateSlider(indx){
  var sliderValue=document.getElementById("motor"+indx).value;
  document.getElementById("motor"+indx+"Slider").innerHTML = sliderValue; 
  console.log(indx);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/slider?value="+sliderValue+"&index="+indx, true);
  xhr.send();
}
</script></body></html>
)rawliteral";



const size_t capacity = JSON_OBJECT_SIZE(2) + 20;
DynamicJsonDocument doc(capacity);
DynamicJsonDocument deviceInfo(JSON_OBJECT_SIZE(21) + 440);

String processor(const String& var){
  if (var == "SLIDERVALUE0"){
    return motor0Slider;
  }else if (var == "SLIDERVALUE1"){
    return motor1Slider;
  }else if (var == "SLIDERVALUE2"){
    return motor2Slider;
  }else if (var == "SLIDERVALUE3"){
    return motor3Slider;
  }else if(var == "BATTERY"){
    return String(battery);
  }else if(var == "DEVICEID"){
    return deviceId;
  }else if(var == "SERIALNO"){
    return serialNo;
  }else{
    return String();
  }
  
}

void setup() {
  
  Serial.begin(115200);
  
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html,processor);
  });

  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_VALUE)) {
      inputMessage = request->getParam(PARAM_INPUT_VALUE)->value();
      String i = request->getParam(PARAM_INPUT_INDEX)->value();

      if(i=="0"){
        motor0Slider = inputMessage;
      }else if(i=="1"){
        motor1Slider = inputMessage;
      }else if(i=="2"){
        motor2Slider = inputMessage;
      }else{
        motor3Slider = inputMessage;
      }
      
      
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
  // Start server
  server.begin();

  
  NeoESP32.setConnectedCallback(onConnected);
  NeoESP32.setReadNotifyCallback(onReadNotify);
  NeoESP32.setDisconnectedCallback(onDisconnected);
  NeoESP32.begin();
  while (!NeoESP32.isConnected() || !NeoESP32.isAuthorized()) {}
  Serial.println("Developer API access granted...");
  NeoESP32.deviceInfo();
  NeoESP32.deviceBattery();
  

}

void loop() {


    
    if (NeoESP32.isConnected() && NeoESP32.isAuthorized()) {
      //Serial.printf("%d,%d,%d,%d\n", motor0Slider.toInt(),motor1Slider.toInt(),motor2Slider.toInt(),motor3Slider.toInt());
      NeoESP32.vibrateMotor(0, map(motor0Slider.toInt(),0,255,0,100)/100.0);
      NeoESP32.vibrateMotor(1, map(motor1Slider.toInt(),0,255,0,100)/100.0);
      NeoESP32.vibrateMotor(2, map(motor2Slider.toInt(),0,255,0,100)/100.0);
      NeoESP32.vibrateMotor(3, map(motor3Slider.toInt(),0,255,0,100)/100.0);
    }
    
    delay(50);
}



void onConnected(bool success) {
  if (!success) {
    Serial.println("Attempted connection but failed.");
    return;
  }
  Serial.println("Successfully connected to Neosensory Buzz...");
  NeoESP32.authorizeDeveloper();
  NeoESP32.acceptTermsAndConditions();
  NeoESP32.stopAlgorithm();
  
  
  

}

void onDisconnected() {
  Serial.println("onDisconnected callback");
}



  
void onReadNotify(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *data, size_t len, bool isNotify) {
  
  
  
     for (int i = 0; i < len; i++)
    {
        if (data[i] == '{')
        {
            jsonStarted_ = true;
            jsonMessage_ = "";
        }
        if (jsonStarted_)
        {
            jsonMessage_ += (char)data[i];
        }
        if (data[i] == '}')
        {
            jsonStarted_ = false;
            handleJSON(jsonMessage_);
            
        }
    }


    
}



void handleJSON(String jsonMessage){
  Serial.println(jsonMessage);

  if (jsonMessage.indexOf("battery_soc") != -1)
    {
        deserializeJson(doc, jsonMessage);
        
        
        battery = doc["battery_soc"];
        
    }

  if (jsonMessage.indexOf("device_id") != -1)
    {
        deserializeJson(deviceInfo, jsonMessage);
        serializeJson(deviceInfo, Serial);
        
        deviceId = deviceInfo["device_id"].as<String>();
        serialNo = deviceInfo["serial_number"].as<String>();
        
    }
    

}
