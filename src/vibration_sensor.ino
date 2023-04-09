/*
    Vibration Sensor for clothes dryer alert.

    20230408 seems to be working.

    (cc) 2023 Bob Glicksman, Jim Schrempp Attribution-NonCommercial-ShareAlike 4.0 International
    https://creativecommons.org/licenses/by-nc-sa/4.0/

*/

#include "Arduino.h"

// How long to wait between reading the sensor. The sensor can be read as
// frequently as you like, but the results only change at about 5FPS, so
// waiting for 200ms is reasonable.
const int32_t SAMPLE_DELAY_MS = 10;

Logger mainLog("app.main");

SerialLogHandler logHandler(LOG_LEVEL_WARN, { // Logging level for non-application messages
    { "app", LOG_LEVEL_INFO }, // Default logging level for all application messages
});

/* ------------------------------ */
// function to move the terminal cursor back up to overwrite previous data printout
void moveTerminalCursorUp(int numlines) {
    String cursorUp = String("\033[") + String(numlines) + String("A");
    Serial.print(cursorUp);
    Serial.print("\r");
}

/* ------------------------------ */
// function to move the terminal cursor down to get past previous data printout - used on startup
void moveTerminalCursorDown(int numlines) {
    String cursorUp = String("\033[") + String(numlines) + String("B");
    Serial.print(cursorUp);
    Serial.print("\r");
}



void setup() {
    pinMode(D7, INPUT);

    Serial.begin(9600);

    moveTerminalCursorDown(5);

    Serial.println("setup almost done");

    mainLog.info("setup complete");
}

void loop() {
    const system_tick_t MS_FOR_VIBRATION_RESET = 1500; // if we haven't sensed vibration in this long, then report no vibration.
    static system_tick_t vibrationDetectedMS = millis();
    static system_tick_t vibrationStateChangeMS = millis();
    static bool vibrationStatePrevious = false;

    delay(SAMPLE_DELAY_MS);

    // read the state of the vibration sensor:
    bool currentVibration = digitalRead(D7);

    bool vibrationState = true;
    if (currentVibration) {
        // We are vibrating
        vibrationDetectedMS = millis();
    } else {
        if (millis() - vibrationDetectedMS > MS_FOR_VIBRATION_RESET) {
            // we have stopped vibrating for a while
            vibrationState = false;
        } 
    }

    // report on vibration state changes
    if (vibrationState != vibrationStatePrevious){

        vibrationStatePrevious = vibrationState;
        vibrationStateChangeMS = millis();

        if (vibrationState) {
            mainLog.info("Started sensing vibration");
        } else {
            mainLog.info("Stopped sensing vibration");
        }
    }

    const system_tick_t MIN_TIME_FOR_LONG_DUR_RUN = 5000; // long enough to ignore spurious jolts
    const system_tick_t MIN_TIME_FOR_LONG_DUR_QUIET = 10000; // long enough to ride out short quiet times
    static system_tick_t longDurationStateChangeMS = millis();
    static bool longDurationState = false;

    // see if we have a state change for long duration vibration
    bool longDurationStateNew = longDurationState;
    if (vibrationState){
        if (millis() - vibrationStateChangeMS > MIN_TIME_FOR_LONG_DUR_RUN) {
            // we have been vibrating a while
            longDurationStateNew = true;
        }
    } else {
        if (millis() - vibrationStateChangeMS > MIN_TIME_FOR_LONG_DUR_QUIET) {
            // we have been still for a while
            longDurationStateNew = false;    
        }
    }

    if (longDurationState != longDurationStateNew) {
        // transition to the new state for long duration
        longDurationState = longDurationStateNew;
        longDurationStateChangeMS = millis();
        if (longDurationState){
            mainLog.info("Long Duration Started");
        } else {
            mainLog.info("Long Duration Stopped");
        }
    }



    // determine and report on the state of the dryer
    const system_tick_t MIN_LONGVIB_FOR_DRYER_TO_RUN = 60000;   // must vibrate for one minute
    const system_tick_t MIN_LONGCALM_FOR_DRYER_TO_STOP = 30000;  // stops vibrating this long, dryer is off
    static bool dryerRunningState = false;

    // see if we have a state change for the dryer
    bool dryerRunningStateNew = dryerRunningState;
    if (longDurationState) {
        if (millis() - longDurationStateChangeMS > MIN_LONGVIB_FOR_DRYER_TO_RUN) {
            // we have been vibrating long enough to decide the dryer is running
            dryerRunningStateNew = true;
        }
    } else {
        if (millis() - longDurationStateChangeMS > MIN_LONGCALM_FOR_DRYER_TO_STOP){
            // we have been still long enough to decide the dryer has stopped
            dryerRunningStateNew = false;
        }
    }

    if (dryerRunningState != dryerRunningStateNew) {
        // transition to new state for dryer running
        dryerRunningState = dryerRunningStateNew;
        if (dryerRunningState) {
            mainLog.info("Dryer Started");
        } else {
            mainLog.info("Dryer Stopped");
        }
    }

}


