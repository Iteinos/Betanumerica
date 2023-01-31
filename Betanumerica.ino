#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 2
#define CLK_PIN   14
#define DATA_PIN  13
#define CS_PIN    15
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
#if defined(ESP32) //ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#elif defined(ESP8266) //ESP8266
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
WiFiClient wifiClient;
#else
#error "Please check your mode setting,it must be esp8266 or esp32."
#endif
 
#include <ArduinoJson.h>
#include <Ticker.h>
 
// 定时器
Ticker timer;
boolean miseajour = true;
 
// JSON
DynamicJsonDocument jsonBuffer(256); // ArduinoJson V5

// WiFi 名称与密码
const char *ssid = "CMCC2"; //这里填你家中的wifi名
const char *password = "84817200";//这里填你家中的wifi密码
// B 站 API 网址: follower, view, likes
String UID  = "35713665";  //改成自己的UID
String followerUrl = "http://api.bilibili.com/x/relation/stat?vmid=" + UID;   // 粉丝数
String viewAndLikesUrl = "http://api.bilibili.com/x/space/upstat?mid=" + UID; // 播放数、点赞数 //obsolete
 
long follower = 0;   // 粉丝数
long view = 0;       // 播放数
long likes = 0;      // 获赞数
 
void setup()
{
  ESP.wdtDisable();
  *((volatile uint32_t*) 0x60000900) &= ~(1); 
  P.begin();
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 24);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    if (P.displayAnimate()){
       P.displayText("WIFI CONNECTING", PA_CENTER, 30, 0, PA_SCROLL_LEFT,PA_SCROLL_LEFT);
      //if(millis()>15000) P.displayText("CONNEXION TIMEOUT. PLEASE VISIT @192.168.4.1 TO CONFIGURE INTERNET ACCESS PARAMETERS", PA_CENTER, 30, 0, PA_SCROLL_LEFT,PA_SCROLL_LEFT);
    }
    Serial.print(".");
    //ESP.wdtFeed();
  }
  Serial.println("WiFi connected");
  timer.attach(300, timerCallback); // 每隔1min
  miseajour=0;
}
 
void loop()
{
  if(!miseajour){
  getFollower(followerUrl);
  char c[8];
  String(follower).toCharArray(c,7);
  P.displayText((char*)c, PA_CENTER, 100, 100, PA_PRINT);
  P.displayAnimate();
  miseajour=1;
  }
}
// 定时器回调函数
void timerCallback()
{
  miseajour = 0;
}
// 获取 B 站粉丝数
void getFollower(String url)
{
  HTTPClient http;
  http.begin(wifiClient, url);
 
  int httpCode = http.GET();
  Serial.printf("[HTTP] GET... code: %dn", httpCode);
 
  if (httpCode == 200)
  {
    Serial.println("Get OK");
    String resBuff = http.getString();
    // ---------- ArduinoJson V5 ----------
    //JsonObject &root = jsonBuffer.parseObject(resBuff);
    if (deserializeJson(jsonBuffer, resBuff))
    {
      Serial.println("parseObject() failed");
      return;
    }
    follower = jsonBuffer["data"]["follower"];
    Serial.print("Fans: ");
    Serial.println(follower);
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %dn", httpCode);
  }
 
  http.end();
}
 
// 获取 B 站播放数与获赞数 //inutile
void getViewAndLikes(String url)
{
  HTTPClient http;
  http.begin(wifiClient,url);
 
  int httpCode = http.GET();
  Serial.printf("[HTTP] GET... code: %dn", httpCode);
 
  if (httpCode == 200)
  {
    Serial.println("Get OK");
    String resBuff = http.getString();
 
    // ---------- ArduinoJson V5 ----------
    //JsonObject &root = jsonBuffer.parseObject(resBuff);
    if (deserializeJson(jsonBuffer, resBuff))
    {
      Serial.println("parseObject() failed");
      return;
    }
    likes = jsonBuffer["data"]["likes"];
    view = jsonBuffer["data"]["archive"]["view"];
    Serial.print("Likes: ");
    Serial.println(likes);
    Serial.print("View: ");
    Serial.println(view);
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %dn", httpCode);
  }
 
  http.end();
}
 
// OLED 显示数据
void display(long follower, long likes, long view)
{

}
