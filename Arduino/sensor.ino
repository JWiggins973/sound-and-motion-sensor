#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <WiFi.h>
#include <HTTPClient.h>


#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* pass = WIFI_PASS;

//const char* ssid = "Wokwi-GUEST";
//const char* pass = "";
const char* discordWebHookUrl = DISCORD_WEBHOOK_URL;
const char* backendHost = BACKEND_HOST;

const unsigned long sampleWindow = 10;
const float V_REF = 3.3;
const float ADC_MAX = 4095.0;



class sensor {
  public: 
    const char* ALERT;
    const char* alertMessages[2] = {"DOOR OPEN", "LOUD NOISE IN AREA"};

     virtual ~sensor() = default;
};

class motionSensor : public sensor {
  private:
    int distanceCm = 0;
    bool close = false;
    bool wasClose = false;

  public:
    const int MOTION_TRIG = A3;  
    const int MOTION_ECHO = A5;

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

};

class soundDetector : public sensor {
  private:
    float soundValue = 0;
    float rawPeak = 0;
    bool loud = false;
    bool wasLoud = false;
    int loudSound = 65;
  
  public:
    const int SOUND_SIM = A0; 

    // setters
    void setSoundValue() {
      unsigned long startMillis = millis();
      unsigned int signalMax = 0;
      unsigned int signalMin = 4095;

      while (millis() - startMillis < sampleWindow) {
        unsigned int sample = analogRead(this->SOUND_SIM);

        if (sample < 4096) {
          if (sample > signalMax) {
            signalMax = sample;
          }

          if (sample < signalMin) {
            signalMin = sample;
          }
        }
      }

      unsigned int peakToPeak = signalMax - signalMin;
      rawPeak = peakToPeak;

      if (peakToPeak > 0) {
        float voltagePeakToPeak = (peakToPeak * V_REF) / ADC_MAX;
        soundValue = 20.0 * log10(voltagePeakToPeak / 0.001);
      } 
      else {
        soundValue = 0;
      }
    }

    void setLoud() {
      loud =  soundValue > loudSound;
    }

    void setWasLoud(bool wasLoud) {
      this->wasLoud = wasLoud;
    }

    // getters
    const int getSoundSim() {
      return SOUND_SIM;
    }
    float getSoundVal() {
      return soundValue;
    }

    float getRawPeak(){ return rawPeak;}
    
    bool getLoud() {
      return loud;
    }

    bool getWasLoud() {
      return wasLoud;
    }
    
};

motionSensor door;
soundDetector mic;

TaskHandle_t taskOne;
TaskHandle_t taskTwo;
TaskHandle_t taskThree;

// alert event struct passed to queue
struct alertEvent {
  const char* message;
  float soundValue;
  float rawPeak;
};

// 4 events can sit in queue with the size of one alert event
QueueHandle_t alertQueue = xQueueCreate(4, sizeof(alertEvent));

// task 1 read sensor
void readSensorTask(void* parameter) {
  while (true) {
   // Serial.print(millis());
    // Serial.println(" ms | sensorTask tick");

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

    // FIXME create local variable to ensure values are updated and not lost!!!! door events frequently happening

    // check id door open or sound above threshold 
    if (door.getClose() && !door.getWasClose()) {
      alertEvent evt = {door.alertMessages[0], -1, -1};
      xQueueSend(alertQueue, &evt, 0);
    }
    if (mic.getLoud() && !mic.getWasLoud()) {
      alertEvent evt = {mic.alertMessages[1], mic.getSoundVal(), mic.getRawPeak()};
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
        Serial.print("Loudness: ");
        Serial.println(evt.soundValue);

      } 
      Serial.println(evt.message);    
      sendDiscordAlert(evt.message);
      sendBackendAlert(evt.message, evt.soundValue, evt.rawPeak);

    }
  }
}

// send live status to backend every second
void liveStatusTask(void* parameter) {
  while (true) {
    sendBackendLiveStatus(door.getClose(), mic.getSoundVal());
    vTaskDelay(pdMS_TO_TICKS(1000));
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

// send live status to backend
void sendBackendLiveStatus(bool doorOpen, float soundValue) {
  HTTPClient http;
  http.begin("http://" + String(backendHost) + ":5238/update-live-status");
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"DoorOpen\": " + String(doorOpen ? "true" : "false") + ", \"SoundValue\": " + String(soundValue, 1) + "}";

  Serial.println(payload);
  int responseCode = http.POST(payload);

  Serial.print("Backend response: ");
  Serial.println(responseCode);

  http.end();
}

// send alert to backend
void sendBackendAlert(const char* message, float soundValue, float rawPeak) {
  HTTPClient http;
  http.begin("http://" + String(backendHost) + ":5238/alert");

  http.addHeader("Content-Type", "application/json");

  // String payload = "{\"message\": \"" + String(message) + "\", \"soundValue\": " + String(soundValue, 1) +  ", \"rawPeakToPeak\": " + String(rawPeak) + "}";
  String payload = "{\"message\": \"" + String(message) + "\", \"soundValue\": " + String(soundValue, 1) + "}";

  int responseCode = http.POST(payload);

  Serial.print("Backend response: ");
  Serial.println(responseCode);

  http.end();
}

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);

  Serial.print("Reset reason: ");
  Serial.println(esp_reset_reason());

  pinMode(door.MOTION_TRIG, OUTPUT);
  pinMode(door.MOTION_ECHO, INPUT);
  pinMode(mic.SOUND_SIM, INPUT);

  // wifi connect 
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.println(WiFi.status());
  }
  Serial.println();
  Serial.println("Conencted, IP: ");
  Serial.println(WiFi.localIP());

  // create task
  xTaskCreate(readSensorTask, "ReadSensor", 8192, NULL, 3, &taskOne);
  xTaskCreate(alertTask, "Alert", 8192, NULL, 2, &taskTwo);
  xTaskCreate(liveStatusTask, "LiveStatus", 8192, NULL, 1, &taskThree);

  // check free
  Serial.println(uxTaskGetStackHighWaterMark(taskOne));
  Serial.println(uxTaskGetStackHighWaterMark(taskTwo));
  Serial.println(uxTaskGetStackHighWaterMark(taskThree));


}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("ALIVE");
  delay(1000);

}
