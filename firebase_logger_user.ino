#include "WiFi.h"
#include "FirebaseESP32.h"
#include "time.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
//ganti
#define WIFI_SSID "AGUS GONDRONG"
#define WIFI_PASSWORD "AGUSG0NDR0NG"

// Insert Firebase project API Key
//ganti
#define API_KEY "AIzaSyCXCjDHUKVPnnuA89A6akryxY2P8xPqM10"

// Insert Authorized Email and Corresponding Password
//ganti
#define USER_EMAIL "wmktefaternakpolije@gmail.com"
#define USER_PASSWORD "wmktefaternak21"

// Insert RTDB URLefine the RTDB URL
//ganti
#define DATABASE_URL "https://contoh-database-dc882-default-rtdb.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)// sesuaikan folder database
String databasePath;
// Database child nodes
String tempPath = "/Suhu";
String humPath = "/Kelembapan";
String Datetime = "/DateTime";
//String nh3Path = "/nh3";
//String fanPath = "/fan";
//String heaterPath = "/heater";
//String lampPath = "/lamp";
//String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

char datetime[80]; // Buffer untuk menyimpan string datetime

int timestamp;
int timestamp1;


FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

float temperature;
float humidity;
float pressure;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 10000;
unsigned long timerDelay2 = 1000;

float randomNumberHumidity;
float randomNumberTemperature;

int IntValueHeater;
int IntValueFan;
int IntValueMotor;

int intValue;
int floatValue;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Deklarasi fungsi convert UnixCode
void unixToDateTime(long int unixTime, char* buffer) {
  // Deklarasi variabel
  time_t rawTime = unixTime;
  tm* timeInfo;

  // Mengubah UNIX code ke struct tm
  timeInfo = localtime(&rawTime);

  // Memformat waktu ke string
  strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeInfo);
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup(){
  Serial.begin(115200);

  initWiFi();
  //NTP Server time 
  configTime(0, 0, ntpServer);
  
  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path (sesuaikan)
  databasePath = "/DemoDatabase/Monitoring/";//"/UsersData/" + uid + "/readings";
}

void loop(){
  //ganti masukan nilai dari variabel sensor
  randomNumberHumidity = random(99.1278354) + 0.25;
  randomNumberTemperature = random(49.120983456) + 0.25;
  delay(6000);
  Serial.print("Kelembaban: ");
  Serial.println(randomNumberHumidity);
  Serial.print("Suhu: ");
  Serial.println(randomNumberTemperature);
 
  //read data from firebase 
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay2 || sendDataPrevMillis == 0)) {
    //read data dari firebase
    if (Firebase.RTDB.getInt(&fbdo, "/Controlling/SetPointUp")) {
      if (fbdo.dataType() == "int") {
        intValue = fbdo.intData();
        Serial.print("Set Poin Atas :");
        Serial.println(intValue);
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
    
    if (Firebase.RTDB.getFloat(&fbdo, "/Controlling/SetPointDown")) {
      if (fbdo.dataType() == "int") {
        floatValue = fbdo.floatData();
        Serial.print("Set Poin Bawah :");
        Serial.println(floatValue);
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }

  // Send new readings to database(mengirim data ke firebase)
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    

    //Get current timestamp// timestamp gak usah 
    timestamp = getTime();
    timestamp1 = timestamp + 25200;
    Serial.print ("time: ");
    Serial.println (timestamp1);

    // Memanggil fungsi untuk mengubah UNIX code ke format datetime
    unixToDateTime(timestamp1, datetime);
    Serial.print("Datetime: ");
    Serial.println(datetime);

    //sesuaikan
    parentPath= databasePath + "data/"+ String(timestamp1);
    //gunakan path yang sudah di inisialisasi
    json.set(tempPath.c_str(), (randomNumberTemperature));
    json.set(humPath.c_str(), (randomNumberHumidity));
    json.set(Datetime.c_str(),(datetime));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}
}
