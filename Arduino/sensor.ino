#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <WiFi.h>
#include <HTTPClient.h>

#include <secrets.h>

//const char* ssid = "WIFI_SSID";
//const char* pass ="WIFI_PASS";

const char* ssid = "Wokwi-GUEST";
const char* pass = "";
const char* discordWebHookUrl = DISCORD_WEBHOOK_URL;
const char* backendHost = BACKEND_HOST;


class sensor {
  public: 
    const char* ALERT;
    const char* alertMessages[2] = {"DOOR OPEN", "LOUD NOISE IN AREA"};
    
    
    virtual void printAlert() {
      Serial.println(ALERT);
    }

     virtual ~sensor() = default;
};

class motionSensor : public sensor {
  private:
    int distanceCm = 0;
    bool close = false;
    bool wasClose = false;

  public:
    const int MOTION_TRIG = 6;  
    const int MOTION_ECHO = 7;

    // setters 
    void setDistanceCm() {
      distanceCm = pulseIn(this->MOTION_ECHO, HIGH) * 0.0343 / 2;
    }

    void setWasClose(bool wasClose) {
      this->wasClose = wasClose;
    }

    void setClose() {
      close = distanceCm < 20;
    }
    
    // getters
    bool getClose() {
      return close;
    }

    int getDistanceCm(){
      return distanceCm;
    }

    bool getWasClose() {
      return wasClose;
    }

    const int getTrig() {
      return MOTION_TRIG;
    }

    const int getEcho() {
      return MOTION_ECHO;
    }

    void printAlert() override {
      if (close && !wasClose){
      ALERT = alertMessages[0];
      Serial.println(ALERT);
      }
      
    }

};

class soundDetector : public sensor {
  private:
    int soundValue = 0;
    bool loud = false;
    bool wasLoud = false;
  
  public:
    const int SOUND_SIM = 4; 

    // setters
    void setSoundValue() {
      soundValue = analogRead(this->SOUND_SIM);
    }

    void setLoud() {
      loud =  soundValue > 2000;
    }

    void setWasLoud(bool wasLoud) {
      this->wasLoud = wasLoud;
    }

    // getters
    const int getSoundSim() {
      return SOUND_SIM;
    }
    int getSoundVal() {
      return soundValue;
    }
wowo
    bool getLoud() {
      return loud;
    }

    bool getWasLoud() {
      return wasLoud;
    }

    void printAlert() override {
      if (loud && !wasLoud) {
      ALERT = alertMessages[1];
      Serial.println("Loudness: " + String(soundValue));
      Serial.println(ALERT);
      }
    }
    
};

motionSensor door;
soundDetector mic;

TaskHandle_t taskOne; 
TaskHandle_t taskTwo;

// alert event struct passed to queue
struct alertEvent {
  const char* message;
  int soundValue;
};

// 4 events can sit in queue with the size of one alert event
QueueHandle_t alertQueue = xQueueCreate(4, sizeof(alertEvent));

// task 1 read sensor
void readSensorTask(void* parameter) {
  while (true) {
    Serial.print(millis());
    Serial.println(" ms | sensorTask tick");

    // task body — never returns
    // Start a new measurement:
    digitalWrite(door.MOTION_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(door.MOTION_TRIG, LOW);

    // initialize distance and sound
    door.setDistanceCm();
    mic.setSoundValue();

    door.setClose();
    mic.setLoud();

    // check id door open or sound above threshold 
    if (door.getClose() && !door.getWasClose()) {
      alertEvent evt = {door.alertMessages[0], -1};
      xQueueSend(alertQueue, &evt, 0);
    }
    else if (mic.getLoud() && !mic.getWasLoud()) {
      alertEvent evt = {mic.alertMessages[1], mic.getSoundVal()};
      xQueueSend(alertQueue, &evt, 0);
    }
    // updates flag to prevent multiple prints
    door.setWasClose(door.getClose()) ;
    mic.setWasLoud(mic.getLoud());
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// task 2 send alert
void alertTask(void* parameter) {
  alertEvent evt;
  while (true) {
    
    if (xQueueReceive(alertQueue, &evt, portMAX_DELAY)) {
      // FIX ME we should pass the class to event so we can use built in print?
      if (evt.soundValue != -1) {
        String discordMessage = evt.message;
        Serial.print("Loudness: ");
        Serial.println(evt.soundValue);

      } 
      Serial.println(evt.message);    
      sendDiscordAlert(evt.message);
      sendBackendAlert(evt.message, evt.soundValue);


    }
  }
}

// send alert to discord webhook
void sendDiscordAlert(const char* message) {
  HTTPClient http;
  http.begin(discordWebHookUrl);
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"content\": \"" + String(message) + "\"}";
  int responseCode = http.POST(payload);

  Serial.print("Discord resposne: ");
  Serial.println(responseCode); // 204 sucessful

  http.end();
}

// send alert to backend
void sendBackendAlert(const char* message, int soundValue) {
  HTTPClient http;
  http.begin("http://" + String(BACKEND_HOST) + ":5238/alert");
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"message\": \"" + String(message) + "\", \"soundValue\": " + String(soundValue) + "}";
  int responseCode = http.POST(payload);

  Serial.print("Backend response: ");
  Serial.println(responseCode);

  http.end();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(door.MOTION_TRIG, OUTPUT);
  pinMode(door.MOTION_ECHO, INPUT);
  pinMode(mic.SOUND_SIM, INPUT);

  // wifi connect 
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Conencted, IP: ");
  Serial.println(WiFi.localIP());


  // create task
  xTaskCreate(readSensorTask, "ReadSensor", 2048, NULL, 2, &taskOne);
  xTaskCreate(alertTask, "Alert", 4096, NULL, 1, &taskTwo);

  // check free
  Serial.println(uxTaskGetStackHighWaterMark(taskOne));
  Serial.println(uxTaskGetStackHighWaterMark(taskTwo));


}

void loop() {
  // put your main code here, to run repeatedly:

}
