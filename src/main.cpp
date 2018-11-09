//#define DEBUG true // Dumps out all log messages to console

#ifdef DEBUG
#define BLYNK_DEBUG true   // Optional, this enables more detailed prints
#define BLYNK_PRINT Serial // Enables Serial Monitor
#endif

#ifdef ESP8266
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#include <BlynkSimpleEsp32_SSL.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#define LED_BUILTIN 2
#endif

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WidgetRTC.h>
#include <fauxmoESP.h>
#include <time.h>

#include "config.h" // edit this file with your secrets

void blynk_connected();
void connect_blynk();
void connect_wifi();
void log_line(String text, bool blynk = true, bool serial = true, bool timestamp = false);
void loop();
void init_alexa();
void init_common();
void init_ota();
void init_wifi();

#define ALEXA true
fauxmoESP alexa; // Using version 2.3.0, 3.0.0 doesn't work

#define V_RSSI V10
#define V_UPTIME V11
#define V_HEAP V12
#define V_SERIAL V13

BlynkTimer blynk_timer;
WidgetTerminal blynk_terminal(V_SERIAL);
WidgetRTC rtc;
WiFiClientSecure wifiClient;

#define STATS_INTERVAL 1000L
#define WDT_INTERVAL 10000L

// logs to Serial and Blynk with timestamp
void log_line(String text, bool blynk, bool serial, bool timestamp) {
    String text_with_ts;

    String current_time = String(hour()) + ":" + minute() + ":" + second();
    String current_date = String(month()) + "-" + day() + "-" + year();

    if (timestamp) {
        text_with_ts = current_date + " " + current_time + ": " + text; //getFullFormattedTime() + ": " + text;
    } else {
        text_with_ts = text;
    }

    if (serial) {
        Serial.println(text_with_ts);
    }

    if (blynk) {
        blynk_terminal.println(text_with_ts);
        blynk_terminal.flush();
    }
}

void init_alexa() {
    alexa.addDevice(alexa_name);
    alexa.enable(true);

    log_line("Alexa initialized");
}

void init_ota() {
    log_line("Initializing OTA...");

    ArduinoOTA.setHostname(hostname);

    ArduinoOTA.begin();
}

void init_common() {
    Serial.begin(115200);
#ifdef DEBUG
    Serial.setDebugOutput(true);
#endif

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    log_line("💥💥💥 Initializing network code...");

    Blynk.begin(auth, ssid, pass);

    setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)

    init_ota();

    init_alexa();

    log_line("Initialized!");

#ifdef ESP8266
    //log_line("Enabling watchdog...");
    //ESP.wdtDisable();
#endif
}

void loop() {
    Blynk.run();

    blynk_timer.run();

    ArduinoOTA.handle();

#ifdef ALEXA
    alexa.handle();
#endif
}

// ******************* HARDWARE-SPECIFIC CODE BELOW  ******************* //

#define PIN_MOTION_SENSOR 12 // D6
#define PIN_AUTO_ON 5        // D1
#define PIN_SERVO 14         // D5

#define V_ENGAGE_ON_MOTION V0
#define V_WAND_POSITION V1
#define V_MOTION_MODE V2
#define V_PLAY V3
#define V_MOTION_SENSED V4

#define WAND_LOWEST 55
#define WAND_HIGHEST 180
#define WAND_START 150
#define WAND_TOY_ON_GROUND 88
#define WAND_TOY_OFF_GROUND 92

#define WIGGLE_FREQ 250L
#define TEASE_FREQ 1000L
#define SLOW_FREQ 250L
#define SERVO_FREQ 50

bool engage_on_motion = false;
int wand_position = 0;
int motion_mode = 0;
bool playing = false;
int timer_motion_timer = 0;
int direction = 1;

void set_wand_position(int new_wand_position) {
    log_line("Setting wand position to " + String(new_wand_position));

    analogWrite(PIN_SERVO, new_wand_position);

    wand_position = new_wand_position;

    Blynk.virtualWrite(V_WAND_POSITION, new_wand_position);
}

// go nuts!
void random_wiggle() {
    int new_wand_position = random(WAND_LOWEST, WAND_HIGHEST);

    set_wand_position(new_wand_position);
}

// alternate between on and off ground, ringing the bell
void tease() {
    if (wand_position == WAND_TOY_ON_GROUND) {
        set_wand_position(WAND_TOY_OFF_GROUND);
    } else if (wand_position == WAND_TOY_OFF_GROUND) {
        set_wand_position(WAND_TOY_ON_GROUND);
    } else {
        set_wand_position(WAND_TOY_OFF_GROUND);
    }
}

// go up and down slowly and quietly
void slow_roll() {
    if (wand_position >= WAND_HIGHEST) {
        direction = -1;
    }
    if (wand_position <= WAND_TOY_ON_GROUND) {
        direction = 1;
    }

    wand_position += direction;
    set_wand_position(wand_position);
}

void set_playing_mode(bool playing) {
    if (playing) {
        log_line("Now playing");

        blynk_timer.deleteTimer(timer_motion_timer);
        switch (motion_mode) {
        case 1:
            timer_motion_timer = blynk_timer.setInterval(WIGGLE_FREQ, random_wiggle);
            break;
        case 2:
            timer_motion_timer = blynk_timer.setInterval(TEASE_FREQ, tease);
            break;
        case 3:
            timer_motion_timer = blynk_timer.setInterval(SLOW_FREQ, slow_roll);
            break;
        }
    } else {
        log_line("Stopping play");
        blynk_timer.deleteTimer(timer_motion_timer);
    }
}

BLYNK_WRITE(V_ENGAGE_ON_MOTION) {
    engage_on_motion = (param.asInt() == 1);

    log_line("Got engage on motion value " + String(engage_on_motion));

    if (!engage_on_motion) {
        set_playing_mode(false);
    }
}

BLYNK_WRITE(V_MOTION_MODE) {
    motion_mode = param.asInt();

    log_line("Got motion mode value " + String(motion_mode));

    set_playing_mode(false);
}

BLYNK_WRITE(V_PLAY) {
    bool playing = (param.asInt() == 1);

    log_line("Got play engage value " + String(playing));

    set_playing_mode(playing);
}

BLYNK_WRITE(V_WAND_POSITION) {
    wand_position = param.asInt();

    set_wand_position(wand_position);
}

void get_motion_sensor_value() {
    bool motion_sensed = digitalRead(PIN_MOTION_SENSOR) == 1;

    log_line("Got motion sensor value " + String(motion_sensed));

    Blynk.virtualWrite(V_MOTION_SENSED, motion_sensed);

    if (engage_on_motion && motion_sensed) {
        Blynk.notify("😼 Motion sensed, playing with cat!");

        Blynk.virtualWrite(V_PLAY, 1);
        set_playing_mode(true);
    } else if (engage_on_motion && !motion_sensed) {
        Blynk.virtualWrite(V_PLAY, 0);
        set_playing_mode(false);
    }
}

void handle_motion_sensor_change() {
    blynk_timer.setTimeout(50L, get_motion_sensor_value);
}

void init_pins() {
    analogWriteFreq(SERVO_FREQ);

    pinMode(PIN_MOTION_SENSOR, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_MOTION_SENSOR), handle_motion_sensor_change, CHANGE);
}

void setup() {
    init_pins();

    init_common();

    alexa.onSetState([](unsigned char device_id, const char *device_name, bool state) {
        log_line("Alexa invoked with value " + String(state));

        set_playing_mode(state);
    });

    Blynk.syncAll();
}