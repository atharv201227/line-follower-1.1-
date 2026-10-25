#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// ============================================================
//                 PIN CONFIGURATION
// ============================================================

// ---------- TB6612FNG ----------
#define PWMA 25
#define AIN1 26
#define AIN2 27

#define PWMB 14
#define BIN1 32
#define BIN2 33

#define STBY 13

// ---------- 5 IR SENSORS ----------
#define S1 34   // Far Left
#define S2 35   // Left
#define S3 36   // Center  
#define S4 39   // Right
#define S5 4    // Far Right


// ============================================================
//                 WIFI CONFIGURATION
// ============================================================

const char* AP_SSID = "LineFollower";
const char* AP_PASSWORD = "12345678";

WebServer server(80);


// ============================================================
//                 SETTINGS
// ============================================================

struct Settings {

  // Motor
  int baseSpeed;
  int maxSpeed;
  int minSpeed;

  int sharpInsideSpeed;
  int sharpOutsideSpeed;

  int lostInsideSpeed;
  int lostOutsideSpeed;

  int leftTrim;
  int rightTrim;

  // PID
  float kp;
  float ki;
  float kd;

  float integralLimit;
  float derivativeFilter;

  // Sharp turn
  float sharpThreshold;

  // Sensor weights
  float w1;
  float w2;
  float w3;
  float w4;
  float w5;

  // Sensor logic
  bool lineStateHigh;

} cfg;


// ============================================================
//                 DEFAULT SETTINGS
// ============================================================

void loadDefaults() {

  cfg.baseSpeed = 90;
  cfg.maxSpeed = 255;
  cfg.minSpeed = 45;

  cfg.sharpInsideSpeed = 35;
  cfg.sharpOutsideSpeed = 210;

  cfg.lostInsideSpeed = 30;
  cfg.lostOutsideSpeed = 150;

  cfg.leftTrim = 0;
  cfg.rightTrim = 0;

  cfg.kp = 18.0;
  cfg.ki = 0.0;
  cfg.kd = 10.0;

  cfg.integralLimit = 30.0;
  cfg.derivativeFilter = 0.70;

  cfg.sharpThreshold = 3.0;

  cfg.w1 = -4.0;
  cfg.w2 = -2.0;
  cfg.w3 = 0.0;
  cfg.w4 = 2.0;
  cfg.w5 = 4.0;

  // LOW = sensor detects line
  cfg.lineStateHigh = false;
}


// ============================================================
//                 EEPROM-LIKE FLASH STORAGE
// ============================================================

Preferences preferences;


void saveSettings() {

  preferences.begin("linefollow", false);

  preferences.putInt("base", cfg.baseSpeed);
  preferences.putInt("max", cfg.maxSpeed);
  preferences.putInt("min", cfg.minSpeed);

  preferences.putInt("sharpIn", cfg.sharpInsideSpeed);
  preferences.putInt("sharpOut", cfg.sharpOutsideSpeed);

  preferences.putInt("lostIn", cfg.lostInsideSpeed);
  preferences.putInt("lostOut", cfg.lostOutsideSpeed);

  preferences.putInt("leftTrim", cfg.leftTrim);
  preferences.putInt("rightTrim", cfg.rightTrim);

  preferences.putFloat("kp", cfg.kp);
  preferences.putFloat("ki", cfg.ki);
  preferences.putFloat("kd", cfg.kd);

  preferences.putFloat("iLimit", cfg.integralLimit);
  preferences.putFloat("dFilter", cfg.derivativeFilter);

  preferences.putFloat("sharpTh", cfg.sharpThreshold);

  preferences.putFloat("w1", cfg.w1);
  preferences.putFloat("w2", cfg.w2);
  preferences.putFloat("w3", cfg.w3);
  preferences.putFloat("w4", cfg.w4);
  preferences.putFloat("w5", cfg.w5);

  preferences.putBool("lineHigh", cfg.lineStateHigh);

  preferences.end();
}


void loadSettings() {

  loadDefaults();

  preferences.begin("linefollow", true);

  cfg.baseSpeed =
    preferences.getInt("base", cfg.baseSpeed);

  cfg.maxSpeed =
    preferences.getInt("max", cfg.maxSpeed);

  cfg.minSpeed =
    preferences.getInt("min", cfg.minSpeed);

  cfg.sharpInsideSpeed =
    preferences.getInt("sharpIn", cfg.sharpInsideSpeed);

  cfg.sharpOutsideSpeed =
    preferences.getInt("sharpOut", cfg.sharpOutsideSpeed);

  cfg.lostInsideSpeed =
    preferences.getInt("lostIn", cfg.lostInsideSpeed);

  cfg.lostOutsideSpeed =
    preferences.getInt("lostOut", cfg.lostOutsideSpeed);

  cfg.leftTrim =
    preferences.getInt("leftTrim", cfg.leftTrim);

  cfg.rightTrim =
    preferences.getInt("rightTrim", cfg.rightTrim);

  cfg.kp =
    preferences.getFloat("kp", cfg.kp);

  cfg.ki =
    preferences.getFloat("ki", cfg.ki);

  cfg.kd =
    preferences.getFloat("kd", cfg.kd);

  cfg.integralLimit =
    preferences.getFloat("iLimit", cfg.integralLimit);

  cfg.derivativeFilter =
    preferences.getFloat("dFilter", cfg.derivativeFilter);

  cfg.sharpThreshold =
    preferences.getFloat("sharpTh", cfg.sharpThreshold);

  cfg.w1 =
    preferences.getFloat("w1", cfg.w1);

  cfg.w2 =
    preferences.getFloat("w2", cfg.w2);

  cfg.w3 =
    preferences.getFloat("w3", cfg.w3);

  cfg.w4 =
    preferences.getFloat("w4", cfg.w4);

  cfg.w5 =
    preferences.getFloat("w5", cfg.w5);

  cfg.lineStateHigh =
    preferences.getBool("lineHigh", cfg.lineStateHigh);

  preferences.end();
}


// ============================================================
//                 MOTOR CONTROL
// ============================================================

void motorLeft(int speed) {

  speed = constrain(speed, -255, 255);

  if (speed > 0) {

    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);

    analogWrite(PWMA, speed);

  } 
  else if (speed < 0) {

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);

    analogWrite(PWMA, -speed);

  } 
  else {

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);

    analogWrite(PWMA, 0);
  }
}


void motorRight(int speed) {

  speed = constrain(speed, -255, 255);

  if (speed > 0) {

    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);

    analogWrite(PWMB, speed);

  } 
  else if (speed < 0) {

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);

    analogWrite(PWMB, -speed);

  } 
  else {

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);

    analogWrite(PWMB, 0);
  }
}


void stopMotors() {

  motorLeft(0);
  motorRight(0);
}


// ============================================================
//                 SENSOR VARIABLES
// ============================================================

bool sensor1;
bool sensor2;
bool sensor3;
bool sensor4;
bool sensor5;

float error = 0;
float lastError = 0;

float integral = 0;
float derivative = 0;
float filteredDerivative = 0;

int leftPWM = 0;
int rightPWM = 0;

bool lineDetected = false;

int lastDirection = 0;


// ============================================================
//                 RUN CONTROL
// ============================================================

bool robotRunning = false;

unsigned long lastControlMicros = 0;

const unsigned long CONTROL_PERIOD_US = 2000;


// ============================================================
//                 READ SENSORS
// ============================================================

void readSensors() {

  int lineValue =
    cfg.lineStateHigh ? HIGH : LOW;

  sensor1 = (digitalRead(S1) == lineValue);
  sensor2 = (digitalRead(S2) == lineValue);
  sensor3 = (digitalRead(S3) == lineValue);
  sensor4 = (digitalRead(S4) == lineValue);
  sensor5 = (digitalRead(S5) == lineValue);
}


// ============================================================
//                 CALCULATE LINE ERROR
// ============================================================

void calculateError() {

  float weightedSum = 0;
  int activeCount = 0;

  if (sensor1) {
    weightedSum += cfg.w1;
    activeCount++;
  }

  if (sensor2) {
    weightedSum += cfg.w2;
    activeCount++;
  }

  if (sensor3) {
    weightedSum += cfg.w3;
    activeCount++;
  }

  if (sensor4) {
    weightedSum += cfg.w4;
    activeCount++;
  }

  if (sensor5) {
    weightedSum += cfg.w5;
    activeCount++;
  }


  // ----------------------------------------------------------
  // LINE FOUND
  // ----------------------------------------------------------

  if (activeCount > 0) {

    lineDetected = true;

    error = weightedSum / activeCount;

    if (error < 0) {
      lastDirection = -1;
    }

    if (error > 0) {
      lastDirection = 1;
    }

  }

  // ----------------------------------------------------------
  // LINE LOST
  // ----------------------------------------------------------

  else {

    lineDetected = false;

    error = lastError;

    // Reset integral to prevent windup
    integral = 0;
  }
}


// ============================================================
//                 PID CONTROL
// ============================================================

void calculatePID() {

  float dt =
    CONTROL_PERIOD_US / 1000000.0;

  // ----------------------------------------------------------
  // LINE LOST
  // ----------------------------------------------------------

  if (!lineDetected) {

    if (lastDirection < 0) {

      // Search left
      leftPWM = cfg.lostInsideSpeed;
      rightPWM = cfg.lostOutsideSpeed;

    }
    else {

      // Search right
      leftPWM = cfg.lostOutsideSpeed;
      rightPWM = cfg.lostInsideSpeed;
    }

    leftPWM += cfg.leftTrim;
    rightPWM += cfg.rightTrim;

    leftPWM =
      constrain(leftPWM, 0, cfg.maxSpeed);

    rightPWM =
      constrain(rightPWM, 0, cfg.maxSpeed);

    return;
  }


  // ----------------------------------------------------------
  // SHARP LEFT
  // ----------------------------------------------------------

  if (error <= -cfg.sharpThreshold) {

    leftPWM = cfg.sharpInsideSpeed;
    rightPWM = cfg.sharpOutsideSpeed;

    integral = 0;
  }


  // ----------------------------------------------------------
  // SHARP RIGHT
  // ----------------------------------------------------------

  else if (error >= cfg.sharpThreshold) {

    leftPWM = cfg.sharpOutsideSpeed;
    rightPWM = cfg.sharpInsideSpeed;

    integral = 0;
  }


  // ----------------------------------------------------------
  // NORMAL PID
  // ----------------------------------------------------------

  else {

    // Integral
    integral += error * dt;

    integral =
      constrain(
        integral,
        -cfg.integralLimit,
        cfg.integralLimit
      );


    // Derivative
    float rawDerivative =
      (error - lastError) / dt;


    // Filter derivative
    filteredDerivative =
      cfg.derivativeFilter * filteredDerivative
      +
      (1.0 - cfg.derivativeFilter) * rawDerivative;


    derivative = filteredDerivative;


    // PID
    float correction =
      (cfg.kp * error)
      +
      (cfg.ki * integral)
      +
      (cfg.kd * derivative);


    // Motor speeds
    float left =
      cfg.baseSpeed + correction;

    float right =
      cfg.baseSpeed - correction;


    // --------------------------------------------------------
    // MINIMUM MOTOR SPEED
    // Prevent motor from stopping during normal correction
    // --------------------------------------------------------

    if (left > 0 && left < cfg.minSpeed) {
      left = cfg.minSpeed;
    }

    if (right > 0 && right < cfg.minSpeed) {
      right = cfg.minSpeed;
    }


    leftPWM = (int)left;
    rightPWM = (int)right;
  }


  // ----------------------------------------------------------
  // MOTOR TRIM
  // ----------------------------------------------------------

  leftPWM += cfg.leftTrim;
  rightPWM += cfg.rightTrim;


  // ----------------------------------------------------------
  // LIMIT PWM
  // ----------------------------------------------------------

  leftPWM =
    constrain(leftPWM, 0, cfg.maxSpeed);

  rightPWM =
    constrain(rightPWM, 0, cfg.maxSpeed);


  lastError = error;
}


// ============================================================
//                 CONTROL LOOP
// ============================================================

void controlLoop() {

  unsigned long now = micros();

  if (now - lastControlMicros <
      CONTROL_PERIOD_US) {

    return;
  }

  lastControlMicros = now;


  // Robot stopped
  if (!robotRunning) {

    stopMotors();

    leftPWM = 0;
    rightPWM = 0;

    return;
  }


  readSensors();

  calculateError();

  calculatePID();

  motorLeft(leftPWM);

  motorRight(rightPWM);
}


// ============================================================
//                 RESET PID
// ============================================================

void resetPID() {

  error = 0;
  lastError = 0;

  integral = 0;

  derivative = 0;
  filteredDerivative = 0;

  lastDirection = 0;
}


// ============================================================
//                 HTML PAGE
// ============================================================

const char MAIN_PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta name="viewport"
content="width=device-width,initial-scale=1">

<title>ESP32 Line Follower Tuner</title>

<style>

body {
  font-family: Arial, sans-serif;
  background:#111;
  color:#eee;
  margin:0;
  padding:15px;
}

h1 {
  text-align:center;
  font-size:25px;
}

h2 {
  margin-bottom:8px;
}

.card {
  background:#1d1d1d;
  padding:15px;
  margin:12px 0;
  border-radius:12px;
}

.row {
  display:flex;
  align-items:center;
  justify-content:space-between;
  gap:10px;
  margin:9px 0;
}

label {
  flex:1;
}

input {
  width:105px;
  padding:8px;
  border-radius:6px;
  border:1px solid #555;
  background:#292929;
  color:white;
}

button {
  border:0;
  padding:12px 15px;
  border-radius:8px;
  font-weight:bold;
  margin:4px;
  cursor:pointer;
}

.run {
  background:#19a974;
  color:white;
}

.stop {
  background:#d63031;
  color:white;
}

.apply {
  background:#0984e3;
  color:white;
}

.save {
  background:#6c5ce7;
  color:white;
}

.reset {
  background:#636e72;
  color:white;
}

.sensor {
  display:inline-block;
  width:50px;
  height:50px;
  line-height:50px;
  text-align:center;
  border-radius:8px;
  margin:4px;
  background:#333;
  font-weight:bold;
}

.active {
  background:#00b894;
}

.inactive {
  background:#444;
}

.value {
  font-size:22px;
  font-weight:bold;
}

textarea {
  width:100%;
  height:130px;
  box-sizing:border-box;
  background:#080808;
  color:#00ff88;
  border:1px solid #444;
  border-radius:8px;
  padding:10px;
}

.status {
  padding:10px;
  border-radius:8px;
  text-align:center;
  margin-bottom:10px;
}

.running {
  background:#064e3b;
}

.stopped {
  background:#4a1111;
}

</style>

</head>


<body>

<h1>ESP32 LINE FOLLOWER</h1>

<div id="status"
class="status stopped">

STOPPED

</div>


<div class="card">

<h2>Robot Control</h2>

<button class="run"
onclick="runRobot()">

RUN

</button>

<button class="stop"
onclick="stopRobot()">

STOP

</button>

<button class="apply"
onclick="applySettings()">

APPLY

</button>

<button class="save"
onclick="saveSettings()">

SAVE

</button>

<button class="reset"
onclick="resetSettings()">

RESET

</button>

</div>


<div class="card">

<h2>Live Sensors</h2>

<div style="text-align:center">

<div id="s1" class="sensor">S1</div>
<div id="s2" class="sensor">S2</div>
<div id="s3" class="sensor">S3</div>
<div id="s4" class="sensor">S4</div>
<div id="s5" class="sensor">S5</div>

</div>

</div>


<div class="card">

<h2>Live Data</h2>

<p>
Error:
<span id="error" class="value">0</span>
</p>

<p>
Left PWM:
<span id="leftPWM" class="value">0</span>
</p>

<p>
Right PWM:
<span id="rightPWM" class="value">0</span>
</p>

<p>
Line:
<span id="line" class="value">NO</span>
</p>

</div>


<div class="card">

<h2>Motor Settings</h2>

<div class="row">
<label>Base Speed</label>
<input id="base" type="number" min="0" max="255">
</div>

<div class="row">
<label>Maximum Speed</label>
<input id="max" type="number" min="0" max="255">
</div>

<div class="row">
<label>Minimum Motor Speed</label>
<input id="min" type="number" min="0" max="255">
</div>

<div class="row">
<label>Sharp Inside Speed</label>
<input id="sharpIn" type="number" min="0" max="255">
</div>

<div class="row">
<label>Sharp Outside Speed</label>
<input id="sharpOut" type="number" min="0" max="255">
</div>

<div class="row">
<label>Lost Line Inside Speed</label>
<input id="lostIn" type="number" min="0" max="255">
</div>

<div class="row">
<label>Lost Line Outside Speed</label>
<input id="lostOut" type="number" min="0" max="255">
</div>

<div class="row">
<label>Left Motor Trim</label>
<input id="leftTrim" type="number" min="-100" max="100">
</div>

<div class="row">
<label>Right Motor Trim</label>
<input id="rightTrim" type="number" min="-100" max="100">
</div>

</div>


<div class="card">

<h2>PID Settings</h2>

<div class="row">
<label>Kp</label>
<input id="kp" type="number"
step="0.1">
</div>

<div class="row">
<label>Ki</label>
<input id="ki" type="number"
step="0.01">
</div>

<div class="row">
<label>Kd</label>
<input id="kd" type="number"
step="0.1">
</div>

<div class="row">
<label>Integral Limit</label>
<input id="iLimit"
type="number"
step="0.1">
</div>

<div class="row">
<label>Derivative Filter</label>
<input id="dFilter"
type="number"
min="0"
max="0.99"
step="0.01">
</div>

<div class="row">
<label>Sharp Turn Threshold</label>
<input id="sharpTh"
type="number"
step="0.1">
</div>

</div>


<div class="card">

<h2>Sensor Weights</h2>

<div class="row">
<label>S1 Far Left</label>
<input id="w1"
type="number"
step="0.1">
</div>

<div class="row">
<label>S2 Left</label>
<input id="w2"
type="number"
step="0.1">
</div>

<div class="row">
<label>S3 Center</label>
<input id="w3"
type="number"
step="0.1">
</div>

<div class="row">
<label>S4 Right</label>
<input id="w4"
type="number"
step="0.1">
</div>

<div class="row">
<label>S5 Far Right</label>
<input id="w5"
type="number"
step="0.1">
</div>

</div>


<div class="card">

<h2>Sensor Logic</h2>

<div class="row">

<label>
Line detected when sensor is HIGH
</label>

<input id="lineHigh"
type="checkbox">

</div>

</div>


<div class="card">

<h2>Current Settings</h2>

<textarea id="settingsText"
readonly></textarea>

<br>

<button class="apply"
onclick="generateSettings()">

GENERATE SETTINGS

</button>

</div>


<script>

const fields = [

"base",
"max",
"min",
"sharpIn",
"sharpOut",
"lostIn",
"lostOut",
"leftTrim",
"rightTrim",

"kp",
"ki",
"kd",

"iLimit",
"dFilter",
"sharpTh",

"w1",
"w2",
"w3",
"w4",
"w5"

];


function loadData(data) {

  fields.forEach(function(id) {

    if(data[id] !== undefined) {

      document.getElementById(id).value =
        data[id];

    }

  });


  document.getElementById("lineHigh").checked =
    data.lineHigh;


  updateSettingsText();
}


function applySettings() {

  let url = "/set?";

  fields.forEach(function(id) {

    url +=
      id + "=" +
      encodeURIComponent(
        document.getElementById(id).value
      ) +
      "&";

  });


  url +=
    "lineHigh=" +
    (
      document.getElementById("lineHigh").checked
      ? "1"
      : "0"
    );


  fetch(url)
    .then(() => {
      generateSettings();
    });

}


function saveSettings() {

  let url = "/save?";

  fields.forEach(function(id) {

    url +=
      id + "=" +
      encodeURIComponent(
        document.getElementById(id).value
      ) +
      "&";

  });


  url +=
    "lineHigh=" +
    (
      document.getElementById("lineHigh").checked
      ? "1"
      : "0"
    );


  fetch(url)
    .then(() => {

      alert("Settings saved");

      generateSettings();

    });

}


function resetSettings() {

  if(!confirm(
    "Reset all tuning values?"
  )) return;


  fetch("/reset")
    .then(() => {
      location.reload();
    });

}


function runRobot() {

  fetch("/run");

}


function stopRobot() {

  fetch("/stop");

}


function generateSettings() {

  let text = "";

  fields.forEach(function(id) {

    text +=
      id + "=" +
      document.getElementById(id).value +
      "\n";

  });


  text +=
    "lineHigh=" +
    (
      document.getElementById("lineHigh").checked
      ? "1"
      : "0"
    );


  document.getElementById(
    "settingsText"
  ).value = text;

}


function updateLive() {

  fetch("/data")

    .then(response => response.json())

    .then(data => {

      document.getElementById("error")
        .innerText =
        Number(data.error).toFixed(3);

      document.getElementById("leftPWM")
        .innerText =
        data.leftPWM;

      document.getElementById("rightPWM")
        .innerText =
        data.rightPWM;


      document.getElementById("line")
        .innerText =
        data.line
        ? "YES"
        : "NO";


      updateSensor(
        "s1",
        data.s1
      );

      updateSensor(
        "s2",
        data.s2
      );

      updateSensor(
        "s3",
        data.s3
      );

      updateSensor(
        "s4",
        data.s4
      );

      updateSensor(
        "s5",
        data.s5
      );


      let status =
        document.getElementById("status");


      if(data.running) {

        status.innerText =
          "RUNNING";

        status.className =
          "status running";

      }
      else {

        status.innerText =
          "STOPPED";

        status.className =
          "status stopped";

      }

    })

    .catch(() => {});

}


function updateSensor(id, active) {

  let element =
    document.getElementById(id);


  if(active) {

    element.className =
      "sensor active";

  }
  else {

    element.className =
      "sensor inactive";

  }

}


setInterval(updateLive, 100);

setInterval(function() {

  if(
    document.activeElement.tagName
    !== "INPUT"
  ) {

    generateSettings();

  }

}, 1000);


fetch("/config")
  .then(r => r.json())
  .then(loadData);

</script>

</body>

</html>

)rawliteral";


// ============================================================
//                 WEB HELPER FUNCTIONS
// ============================================================

int getIntArg(
  const char* name,
  int current
) {

  if(server.hasArg(name)) {

    return server.arg(name).toInt();

  }

  return current;
}


float getFloatArg(
  const char* name,
  float current
) {

  if(server.hasArg(name)) {

    return server.arg(name).toFloat();

  }

  return current;
}


bool getBoolArg(
  const char* name,
  bool current
) {

  if(server.hasArg(name)) {

    return server.arg(name).toInt() != 0;

  }

  return current;
}


// ============================================================
//                 APPLY WEB SETTINGS
// ============================================================

void readWebSettings() {

  cfg.baseSpeed =
    getIntArg("base", cfg.baseSpeed);

  cfg.maxSpeed =
    getIntArg("max", cfg.maxSpeed);

  cfg.minSpeed =
    getIntArg("min", cfg.minSpeed);

  cfg.sharpInsideSpeed =
    getIntArg(
      "sharpIn",
      cfg.sharpInsideSpeed
    );

  cfg.sharpOutsideSpeed =
    getIntArg(
      "sharpOut",
      cfg.sharpOutsideSpeed
    );

  cfg.lostInsideSpeed =
    getIntArg(
      "lostIn",
      cfg.lostInsideSpeed
    );

  cfg.lostOutsideSpeed =
    getIntArg(
      "lostOut",
      cfg.lostOutsideSpeed
    );

  cfg.leftTrim =
    getIntArg(
      "leftTrim",
      cfg.leftTrim
    );

  cfg.rightTrim =
    getIntArg(
      "rightTrim",
      cfg.rightTrim
    );


  cfg.kp =
    getFloatArg(
      "kp",
      cfg.kp
    );

  cfg.ki =
    getFloatArg(
      "ki",
      cfg.ki
    );

  cfg.kd =
    getFloatArg(
      "kd",
      cfg.kd
    );


  cfg.integralLimit =
    getFloatArg(
      "iLimit",
      cfg.integralLimit
    );

  cfg.derivativeFilter =
    getFloatArg(
      "dFilter",
      cfg.derivativeFilter
    );

  cfg.sharpThreshold =
    getFloatArg(
      "sharpTh",
      cfg.sharpThreshold
    );


  cfg.w1 =
    getFloatArg(
      "w1",
      cfg.w1
    );

  cfg.w2 =
    getFloatArg(
      "w2",
      cfg.w2
    );

  cfg.w3 =
    getFloatArg(
      "w3",
      cfg.w3
    );

  cfg.w4 =
    getFloatArg(
      "w4",
      cfg.w4
    );

  cfg.w5 =
    getFloatArg(
      "w5",
      cfg.w5
    );


  cfg.lineStateHigh =
    getBoolArg(
      "lineHigh",
      cfg.lineStateHigh
    );


  // Safety limits
  cfg.baseSpeed =
    constrain(
      cfg.baseSpeed,
      0,
      255
    );

  cfg.maxSpeed =
    constrain(
      cfg.maxSpeed,
      0,
      255
    );

  cfg.minSpeed =
    constrain(
      cfg.minSpeed,
      0,
      255
    );

  cfg.sharpInsideSpeed =
    constrain(
      cfg.sharpInsideSpeed,
      0,
      255
    );

  cfg.sharpOutsideSpeed =
    constrain(
      cfg.sharpOutsideSpeed,
      0,
      255
    );

  cfg.lostInsideSpeed =
    constrain(
      cfg.lostInsideSpeed,
      0,
      255
    );

  cfg.lostOutsideSpeed =
    constrain(
      cfg.lostOutsideSpeed,
      0,
      255
    );

  cfg.derivativeFilter =
    constrain(
      cfg.derivativeFilter,
      0.0,
      0.99
    );

  cfg.sharpThreshold =
    constrain(
      cfg.sharpThreshold,
      0.1,
      10.0
    );
}


// ============================================================
//                 WEB: ROOT
// ============================================================

void handleRoot() {

  server.send_P(
    200,
    "text/html",
    MAIN_PAGE
  );
}


// ============================================================
//                 WEB: APPLY
// ============================================================

void handleSet() {

  readWebSettings();

  resetPID();

  server.send(
    200,
    "text/plain",
    "OK"
  );
}


// ============================================================
//                 WEB: SAVE
// ============================================================

void handleSave() {

  readWebSettings();

  saveSettings();

  resetPID();

  server.send(
    200,
    "text/plain",
    "SAVED"
  );
}


// ============================================================
//                 WEB: RESET
// ============================================================

void handleReset() {

  loadDefaults();

  saveSettings();

  resetPID();

  server.send(
    200,
    "text/plain",
    "RESET"
  );
}


// ============================================================
//                 WEB: RUN
// ============================================================

void handleRun() {

  resetPID();

  robotRunning = true;

  digitalWrite(STBY, HIGH);

  server.send(
    200,
    "text/plain",
    "RUNNING"
  );
}


// ============================================================
//                 WEB: STOP
// ============================================================

void handleStop() {

  robotRunning = false;

  stopMotors();

  resetPID();

  server.send(
    200,
    "text/plain",
    "STOPPED"
  );
}


// ============================================================
//                 WEB: CONFIG
// ============================================================

void handleConfig() {

  String json = "{";

  json += "\"base\":" +
          String(cfg.baseSpeed) + ",";

  json += "\"max\":" +
          String(cfg.maxSpeed) + ",";

  json += "\"min\":" +
          String(cfg.minSpeed) + ",";

  json += "\"sharpIn\":" +
          String(cfg.sharpInsideSpeed) + ",";

  json += "\"sharpOut\":" +
          String(cfg.sharpOutsideSpeed) + ",";

  json += "\"lostIn\":" +
          String(cfg.lostInsideSpeed) + ",";

  json += "\"lostOut\":" +
          String(cfg.lostOutsideSpeed) + ",";

  json += "\"leftTrim\":" +
          String(cfg.leftTrim) + ",";

  json += "\"rightTrim\":" +
          String(cfg.rightTrim) + ",";


  json += "\"kp\":" +
          String(cfg.kp, 3) + ",";

  json += "\"ki\":" +
          String(cfg.ki, 3) + ",";

  json += "\"kd\":" +
          String(cfg.kd, 3) + ",";

  json += "\"iLimit\":" +
          String(cfg.integralLimit, 3) + ",";

  json += "\"dFilter\":" +
          String(cfg.derivativeFilter, 3) + ",";

  json += "\"sharpTh\":" +
          String(cfg.sharpThreshold, 3) + ",";


  json += "\"w1\":" +
          String(cfg.w1, 3) + ",";

  json += "\"w2\":" +
          String(cfg.w2, 3) + ",";

  json += "\"w3\":" +
          String(cfg.w3, 3) + ",";

  json += "\"w4\":" +
          String(cfg.w4, 3) + ",";

  json += "\"w5\":" +
          String(cfg.w5, 3) + ",";


  json += "\"lineHigh\":" +
          String(
            cfg.lineStateHigh
            ? "true"
            : "false"
          );

  json += "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// ============================================================
//                 WEB: LIVE DATA
// ============================================================

void handleData() {

  readSensors();

  String json = "{";

  json += "\"s1\":" +
          String(sensor1 ? 1 : 0) + ",";

  json += "\"s2\":" +
          String(sensor2 ? 1 : 0) + ",";

  json += "\"s3\":" +
          String(sensor3 ? 1 : 0) + ",";

  json += "\"s4\":" +
          String(sensor4 ? 1 : 0) + ",";

  json += "\"s5\":" +
          String(sensor5 ? 1 : 0) + ",";

  json += "\"error\":" +
          String(error, 3) + ",";

  json += "\"leftPWM\":" +
          String(leftPWM) + ",";

  json += "\"rightPWM\":" +
          String(rightPWM) + ",";

  json += "\"line\":" +
          String(
            lineDetected
            ? "true"
            : "false"
          ) + ",";

  json += "\"running\":" +
          String(
            robotRunning
            ? "true"
            : "false"
          );

  json += "}";


  server.send(
    200,
    "application/json",
    json
  );
}


// ============================================================
//                 SETUP
// ============================================================

void setup() {

  Serial.begin(115200);


  // ----------------------------------------------------------
  // Motor pins
  // ----------------------------------------------------------

  pinMode(PWMA, OUTPUT);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);


  // ----------------------------------------------------------
  // Sensor pins
  // ----------------------------------------------------------

  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(S5, INPUT);


  // ----------------------------------------------------------
  // Safety
  // ----------------------------------------------------------

  digitalWrite(STBY, HIGH);

  stopMotors();


  // ----------------------------------------------------------
  // Load settings
  // ----------------------------------------------------------

  loadSettings();


  // ----------------------------------------------------------
  // Wi-Fi Access Point
  // ----------------------------------------------------------

  WiFi.mode(WIFI_AP);

  WiFi.softAP(
    AP_SSID,
    AP_PASSWORD
  );


  Serial.println();
  Serial.println("==============================");
  Serial.println("ESP32 LINE FOLLOWER");
  Serial.println("==============================");

  Serial.print("WiFi SSID: ");
  Serial.println(AP_SSID);

  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);

  Serial.print("Web address: ");
  Serial.println(
    WiFi.softAPIP()
  );


  // ----------------------------------------------------------
  // Web server
  // ----------------------------------------------------------

  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );

  server.on(
    "/set",
    HTTP_GET,
    handleSet
  );

  server.on(
    "/save",
    HTTP_GET,
    handleSave
  );

  server.on(
    "/reset",
    HTTP_GET,
    handleReset
  );

  server.on(
    "/run",
    HTTP_GET,
    handleRun
  );

  server.on(
    "/stop",
    HTTP_GET,
    handleStop
  );

  server.on(
    "/config",
    HTTP_GET,
    handleConfig
  );

  server.on(
    "/data",
    HTTP_GET,
    handleData
  );


  server.begin();

  Serial.println(
    "Web server started."
  );

  Serial.println(
    "Robot is STOPPED."
  );
}


// ============================================================
//                 MAIN LOOP
// ============================================================

void loop() {

  server.handleClient();

  controlLoop();

}