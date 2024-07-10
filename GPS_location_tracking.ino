#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <ThingerESP8266.h>
#include <TimeLib.h> // Include TimeLib for time manipulation

#define USERNAME "joshua805080"
#define DEVICE_ID "ESP8266_location_tracker"
#define DEVICE_CREDENTIAL "_3PI8Hlu&DzRF-L7"
#define SSID "iQOO_Z3_5G"
#define SSID_PASSWORD "nethaksko"

ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

static const int RXPin = D2, TXPin = D1;
static const uint32_t GPSBaud = 9600;

// The TinyGPSPlus object
TinyGPSPlus gps;
// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void setup()
{
  Serial.begin(115200);
  ss.begin(GPSBaud);
  //code written by joshua805080
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println();
  Serial.println(F("Sats HDOP  Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Chars Sentences Checksum"));
  Serial.println(F("           (deg)      (deg)       Age                      Age  (m)    --- from GPS ----  RX    RX        Fail"));
  Serial.println(F("-----------------------------------------------------------------------------------------------------------------"));

  // Set WiFi credentials
  thing.add_wifi(SSID, SSID_PASSWORD);

  // Define the Thinger.io resources
  thing["gps_data"] >> [](pson &out) {
    out["latitude"] = gps.location.lat();
    out["longitude"] = gps.location.lng();
    out["altitude"] = gps.altitude.meters();
    out["speed"] = gps.speed.kmph();//code written by joshua805080
    out["course"] = gps.course.deg();
    out["satellites"] = gps.satellites.value();
    out["hdop"] = gps.hdop.hdop();
  };
}

void loop()
{
  thing.handle();

  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  printInt(gps.location.age(), gps.location.isValid(), 5);
  printDateTime(gps.date, gps.time);//code written by joshua805080
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
  printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  printStr(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.deg()) : "*** ", 6);

  printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);
  Serial.println();
  
  smartDelay(1000);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}
//code written by joshua805080
// This custom version of delay() ensures that the gps object is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    // Convert UTC to Indian Standard Time (IST) UTC+5:30
    time_t utcTime = t.hour() * 3600 + t.minute() * 60 + t.second();
    time_t istTime = utcTime + 5.5 * 3600; // Add 5 hours 30 minutes
    //code written by joshua805080
    // Convert to hours, minutes, seconds
    int hours = (istTime % 86400L) / 3600;
    int minutes = (istTime % 3600) / 60;
    int seconds = istTime % 60;

    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",/* code written by joshua805080 */ d.month(), d.day(), d.year(), hours, minutes, seconds);
    Serial.print(sz);
  }
  
  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}
