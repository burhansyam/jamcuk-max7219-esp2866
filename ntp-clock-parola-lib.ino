#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <time.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "font.h"
#include "DHT.h"
#define DHTTYPE DHT11 
const int DHTPin = 2;
DHT dht(DHTPin, DHTTYPE);
#define TIMEDHT 1000
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
//set PIN Max7219
#define CLK_PIN   14 // or SCK
#define DATA_PIN  13 // or MOSI
#define CS_PIN    0 // or SS
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

//sprite def
const uint8_t F_PMAN1 = 6;
const uint8_t W_PMAN1 = 8;
const uint8_t PROGMEM pacman1[F_PMAN1 * W_PMAN1] =  // gobbling pacman animation
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
};
const uint8_t F_PMAN2 = 6;
const uint8_t W_PMAN2 = 18;
const uint8_t PROGMEM pacman2[F_PMAN2 * W_PMAN2] =  // pacman pursued by a ghost
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
};

//set global
#define SPEED_TIME  45
#define PAUSE_TIME  2500
#define MAX_MESG  200



const char *ssid = "POJOK_BKAD";
const char *password = "semangat";

int timezone = 7;
int dst = 0;
uint16_t  h, m, s;
uint8_t dow;
int  day;
uint8_t month;
String  year;
uint8_t weton;

uint32_t timerDHT = TIMEDHT;
float humidity, celsius, fahrenheit;

// Global variables
char szTime[9];    // mm:ss\0
char szsecond[4];    // ss
char cok[MAX_MESG+1] = "200";

//simbol derajat
uint8_t degC[] = { 6, 3, 3, 56, 68, 68, 68 }; // Deg C
uint8_t degF[] = { 6, 3, 3, 124, 20, 20, 4 }; // Deg F

//Ambil Bulan
char *mon2str(uint8_t mon, char *psz, uint8_t len)
{
  static const char str[][4] PROGMEM =
  {
    "Jan", "Feb", "Mar", "Apr", "Mei", "Jun",
    "Jul", "Agu", "Sep", "Okt", "Nov", "Des"
  };

  *psz = '\0';
  mon--;
  if (mon < 12)
  {
    strncpy_P(psz, str[mon], len);
    psz[len] = '\0';
  }

  return(psz);
}

//Ambil Hari
char *dow2str(uint8_t code, char *psz, uint8_t len)
{
  static const char str[][10] PROGMEM =
  {
    "Ahad", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"
  };

  *psz = '\0';
  code--;
  if (code < 7)
  {
    strncpy_P(psz, str[code], len);
    psz[len] = '\0';
  }

  return(psz);
}

// Ambil Jam Menit
void getTime(char *psz, bool f = true)
{
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
      h = p_tm->tm_hour;
      m = p_tm->tm_min;
      s = p_tm->tm_sec;
  sprintf(psz, "%02d%c%02d", h, (f ? ':' : ' '), m);
}
// Ambil Detik
void getsec(char *psz)
{
  sprintf(psz, "%02d", s);
}

// Ambil Tanggal
void getDate(char *psz)
{
  char  szBuf[10];
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
      dow = p_tm->tm_wday+1;
      day = p_tm->tm_mday;
      month = p_tm->tm_mon + 1;
      year = p_tm->tm_year + 1900;      
  sprintf(psz, "%d %s %04d", day, mon2str(month, szBuf, sizeof(szBuf)-1), (p_tm->tm_year + 1900)); //%04d
 
}

// Ambil Data Sensor DHT
void getTemperature()
{
  // Wait for a time between measurements
  if ((millis() - timerDHT) > TIMEDHT) {
    // Update the timer
    timerDHT = millis();
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    celsius = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    fahrenheit = dht.readTemperature(true);
    // Check if any reads failed and exit early (to try again)
    if (isnan(humidity) || isnan(celsius) || isnan(fahrenheit)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  }
}

//Custom Text 1
void getJancuk1(char *psz)
{
  sprintf(psz, "TiTIB");
}
//Custom Text 2
void getJancuk2(char *psz)
{
  sprintf(psz, ".BKAD.");
}

//Custom Text 3
void getJancuk3(char *psz)
{
  sprintf(psz, "HUT GK");
}
//Custom Text 4
void getJancuk4(char *psz)
{
  sprintf(psz, "ke 200 dengan tema 'Cancut Taliwanda'");
}


void setup(void)
{
    Serial.begin(115200);
    delay(10);
    
    //Mencoba konek ke Akses Poin Wifi
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  delay(3000);
  
  //ambil ntp
  getTimentp();
  P.begin(3);
  P.setInvert(false);
  P.setIntensity(0,7); 
  P.setZone(0, 0, 3);
  P.setZone(1, 1, 4);
  P.setSpriteData(pacman2, W_PMAN2, F_PMAN2, pacman2, W_PMAN2, F_PMAN2);
  P.displayZoneText(0, cok, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
//  P.displayZoneText(1, szTime, PA_LEFT, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.addChar('$', degC);
  P.addChar('&', degF);
  
  dht.begin();
  getTemperature();
  getDate(cok);
  getTime(cok);
  getJancuk1(cok);
  getJancuk2(cok);
  getJancuk3(cok);
  getJancuk4(cok);    
}

void loop(void)
{
  static uint32_t lastTime = 0; // millis() memory
  static uint8_t  display = 0;  // current display mode
  static bool flasher = false;  // seconds passing flasher
  static uint8_t i = 0;
  P.displayAnimate();

  if (P.getZoneStatus(0))
  {
    switch (display)
    {
      case 0: // Hari
        P.setTextEffect(0, PA_CLOSING_CURSOR, PA_CLOSING_CURSOR);
        display++;
        dow2str(dow, cok, MAX_MESG);
        break;

      case 1: // tanggal
        P.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display++;
        getDate(cok);
        break;

      case 2: // suhu c
        P.setTextEffect(0, PA_CLOSING_CURSOR, PA_CLOSING_CURSOR);
        display++;
        dtostrf(celsius, 3, 1, cok);
        strcat(cok, "$");
        break;
        
      case 3: // Suhu f
        P.setTextEffect(0, PA_CLOSING_CURSOR, PA_CLOSING_CURSOR);
        display++;
        dtostrf(fahrenheit, 3, 1, cok);
        strcat(cok, "&");
        break;
        
      case 4: // Kelembaban
        P.setTextEffect(0, PA_CLOSING_CURSOR, PA_CLOSING_CURSOR);
        display++;
        dtostrf(humidity, 3, 0, cok);
        strcat(cok, "%Rh");
        break;
      
      case 5: // Teks
        P.setTextEffect(0, PA_SCAN_HORIZ, PA_SCAN_HORIZ);
        display++;
        getJancuk1(cok);
        break;  

      case 6: // Teks
        P.setTextEffect(0, PA_GROW_DOWN, PA_GROW_DOWN);
        display++;
        getJancuk2(cok);
        break;  
               
      case 7: // Teks
        P.setTextEffect(0, PA_GROW_DOWN, PA_SCROLL_RIGHT);
        display++;
        getJancuk3(cok);
        break;  
               
      case 8: // Teks
        P.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display++;
        getJancuk4(cok);
        break;  
               
      case 9:  // Jam
        P.setTextEffect(0, PA_SPRITE, PA_NO_EFFECT);
        display++;
        getTime(cok);
        break;      

      case 10:  // Jam
        P.setTextEffect(0, PA_PRINT, PA_NO_EFFECT);
        display++;
        getTime(cok);
        break; 
             
      case 11:  // Jam
        P.setTextEffect(0, PA_FADE, PA_NO_EFFECT);
        display++;
        getTime(cok);
        break;      

      case 12:  // Jam
        P.setTextEffect(0, PA_PRINT, PA_NO_EFFECT);
        display++;
        getTime(cok);
        break; 
             
      case 13:  // Jam
        P.setTextEffect(0, PA_FADE, PA_NO_EFFECT);
        display++;
        getTime(cok);
        break;      

      case 14:  // Jam
        P.setTextEffect(0, PA_PRINT, PA_NO_EFFECT);
        display++;
        getTime(cok);
        break; 
        
      case 15:  // Jam
        P.setTextEffect(0, PA_BLINDS, PA_NO_EFFECT);
        display++;
        getTime(cok);
        break;  
        
      default:  // Jam
//        P.setFont(0, numeric7Se);
        P.setTextEffect(0, PA_PRINT, PA_SPRITE);
        display = 0;
        getTime(cok);
        break;               

    }

    P.displayReset(0);
  }

  // Finally, adjust the time string if we have to
  if (millis() - lastTime >= 1000)
  {
    lastTime = millis();
    getsec(szsecond);
    getTime(szTime, flasher);
    flasher = !flasher;

//    P.displayReset(1);
  }
}

//setting NTP
void getTimentp()
{

  configTime(timezone * 3600, dst, "id.pool.ntp.org","time.nist.gov");

  while(!time(nullptr)){
        delay(500);
        Serial.print(".");
  }
        Serial.print("Time Update");
}
