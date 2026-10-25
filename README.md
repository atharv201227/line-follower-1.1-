# ESP32 5-Sensor Line Follower Robot

A high-speed, adjustable line-following robot built using an ESP32, five digital IR sensors, a TB6612FNG motor driver, and two DC gear motors.

The robot includes a **Wi-Fi-based web dashboard** that allows you to tune motor speeds, PID values, sensor settings, and turning behavior directly from a phone, tablet, or computer browser.

You don't need to install a separate mobile application or connect the ESP32 to your home Wi-Fi network.

---

## Features

- **5-sensor line detection:** Reads five IR sensors to estimate the robot's position relative to the line.
- **ESP32 control:** Processes sensor readings and controls the motors.
- **TB6612FNG motor driver:** Independently controls the left and right motors.
- **Wi-Fi access point:** The ESP32 creates its own Wi-Fi network.
- **Web-based tuning dashboard:** Adjust control parameters from a browser.
- **Live monitoring:** View sensor readings, line error, and motor PWM values.
- **Adjustable base speed:** Control the robot's normal driving speed.
- **PID tuning:** Adjust proportional, integral, and derivative gains.
- **Sharp-turn handling:** Configure separate motor speeds for sharp turns.
- **Line-loss recovery:** Adjust motor behavior when the sensors lose the line.
- **Left/right motor trimming:** Compensate for differences between motors.
- **Adjustable sensor weights:** Change how strongly each sensor affects the calculated line position.
- **Save settings:** Store configuration values in ESP32 nonvolatile memory.
- **Reset settings:** Restore the default tuning parameters.
- **RUN/STOP controls:** Start or stop the robot from the dashboard.

---

## Hardware Required

| Component | Quantity | Purpose |
|---|---:|---|
| ESP32 development board | 1 | Main controller |
| TB6612FNG dual motor driver | 1 | Controls two DC motors |
| Digital IR line sensors | 5 | Detect the line |
| DC gear motors | 2 | Drive the robot |
| Robot chassis | 1 | Holds the electronics and motors |
| Suitable battery | 1 | Powers the robot |
| Regulated DC-DC buck converter | 1 | Supplies the correct motor voltage |
| Connecting wires | As required | Electrical connections |
| USB data cable | 1 | Programming the ESP32 |

### Recommended motor setup

The project was discussed with N20 6 V, approximately 600 RPM gear motors.

The actual motor speed depends on the load, battery supply, wheel diameter, and chassis weight.

**Important:** Do not connect an 11.1 V battery directly to 6 V motors. Use a properly rated buck converter adjusted to the motor's permitted voltage.

---

## Pin Connections

### 1. ESP32 to TB6612FNG

| TB6612FNG Pin | ESP32 Pin | Function |
|---|---|---|
| PWMA | GPIO 25 | Left motor speed control |
| AIN1 | GPIO 26 | Left motor direction |
| AIN2 | GPIO 27 | Left motor direction |
| PWMB | GPIO 14 | Right motor speed control |
| BIN1 | GPIO 32 | Right motor direction |
| BIN2 | GPIO 33 | Right motor direction |
| STBY | GPIO 13 | Enables the motor driver |
| VCC | 3.3 V | Driver logic supply |
| GND | GND | Common ground |
| VM | Regulated motor supply | Motor power |

Connect the left motor to **AO1 and AO2**.

Connect the right motor to **BO1 and BO2**.

The motor-driver supply voltage must be appropriate for both the driver and the motors.

### 2. ESP32 to IR Sensors

| Sensor | Position | ESP32 GPIO |
|---|---|---|
| S1 | Far left | GPIO 34 |
| S2 | Left | GPIO 35 |
| S3 | Center | GPIO 36 |
| S4 | Right | GPIO 39 |
| S5 | Far right | GPIO 4 |

For each sensor, connect:

- VCC to its appropriate supply voltage.
- GND to common ground.
- OUT to the corresponding ESP32 GPIO.

**Electrical warning:** ESP32 GPIO pins are not 5 V tolerant. If a sensor produces a 5 V output, use an appropriate level shifter or voltage divider before connecting it to the ESP32.

GPIO 34, 35, 36, and 39 are input-only pins on the classic ESP32. They are suitable for sensor inputs.

### 3. Common Ground

Connect the grounds of the following components together:

- ESP32
- TB6612FNG
- IR sensor array
- Motor power supply
- Buck converter output

Do not power the motors directly from the ESP32's GPIO pins.

---

## Power Supply and Safety

The motor power supply must match the motors and driver.

For 6 V N20 motors:

1. Connect the battery to a suitable buck converter.
2. Adjust the converter output to the motor's permitted voltage, approximately 6 V for the discussed motors.
3. Connect the regulated output to the TB6612FNG VM and GND.
4. Power the ESP32 using a suitable regulated supply.
5. Connect all grounds together.

Check the buck converter's current rating carefully. DC motors can draw much more current during startup, sudden direction changes, or a stall.

A battery's C rating does not mean its voltage is safe for the motors.

**Before testing, raise the robot's wheels off the ground.** Confirm motor directions and test the STOP control before placing the robot on a track.

---

## Software Requirements

- Arduino IDE
- ESP32 board support package
- USB data cable
- A phone, tablet, or computer with a Wi-Fi-capable browser

The firmware uses these Arduino libraries:

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
```

These libraries are normally included with the ESP32 Arduino core.

---

## Uploading the Firmware

1. Open Arduino IDE.
2. Select the correct ESP32 board.
3. Select the correct serial port.
4. Open the line-follower sketch.
5. Compile the sketch.
6. Upload it to the ESP32.

If uploading fails with a flash communication error:

- Set **Upload Speed** to `115200`.
- Disconnect external sensors and motor-driver wiring temporarily.
- Close Serial Monitor while uploading.
- Use a reliable USB data cable.
- Check the selected board and serial port.
- If necessary, hold the BOOT button while starting the upload and release it when uploading begins.

Reconnect the hardware after a successful upload.

---

## Connecting to the Web Dashboard

After the firmware starts, the ESP32 creates its own Wi-Fi access point.

### Wi-Fi details

| Setting | Value |
|---|---|
| Network name (SSID) | `LineFollower` |
| Password | `12345678` |
| Dashboard address | `http://192.168.4.1` |

### Connection procedure

1. Power on the ESP32.
2. Open Wi-Fi settings on your phone or computer.
3. Connect to `LineFollower`.
4. Enter the password `12345678`.
5. Open a web browser.
6. Enter `http://192.168.4.1`.

The dashboard should open if the firmware is running correctly.

Your phone may report that the Wi-Fi network has no internet access. This is normal because the ESP32 provides a local network rather than an internet connection.

---

## Web Dashboard Controls

The dashboard provides controls for adjusting the robot's behavior.

### Base Speed

Controls the normal motor speed used while following the line.

- Lower values make testing easier.
- Higher values can increase speed but may cause overshooting.
- Start with a moderate value and increase it gradually.

### Maximum Speed

Sets the upper limit for motor PWM commands.

Increasing this value allows more aggressive motor control, but it does not guarantee that the motors can reach the corresponding speed.

### Minimum Speed

Sets a lower speed limit used by the control logic.

This can help prevent the motors from slowing down so much that they stop responding, depending on the firmware's motor-control conditions.

### Sharp Inside Speed

Controls the motor on the inside of a sharp turn.

Reducing the inside motor's speed can help the robot rotate more tightly.

### Sharp Outside Speed

Controls the motor on the outside of a sharp turn.

A higher outside speed can help the robot negotiate a turn, but excessive speed may cause it to leave the track.

### Lost-Line Inside Speed

Controls the inside motor when the line is lost.

### Lost-Line Outside Speed

Controls the outside motor when the line is lost.

These settings affect the recovery maneuver. Test them at low speed because incorrect recovery behavior can make the robot turn away from the line.

### Left Trim and Right Trim

Adjust the motor outputs to compensate for differences between the left and right motors.

If the robot consistently drifts to one side on a straight line, motor trim may help correct it.

First check wheel alignment, wiring, and motor direction before relying on trim.

---

## PID Control

The firmware includes three PID parameters.

### Kp — Proportional Gain

Controls how strongly the robot reacts to the current line-position error.

- Too low: the robot may respond slowly and drift away from the line.
- Too high: the robot may oscillate or make aggressive corrections.

### Ki — Integral Gain

Compensates for persistent error over time.

- A value of zero disables integral correction.
- Excessive integral gain can cause overshoot and instability.

Begin with `Ki = 0` while tuning proportional and derivative control.

### Kd — Derivative Gain

Responds to changes in error and can help reduce oscillation.

- Too low: the robot may wobble around the line.
- Too high: motor commands may become excessively sensitive to sensor changes.

The appropriate values depend on motor characteristics, wheel grip, chassis geometry, and operating speed.

### Integral Limit

Limits the accumulated integral error to reduce excessive integral buildup.

This parameter matters primarily when integral control is enabled.

### Derivative Filter

Controls filtering of the derivative response.

Filtering can reduce sensitivity to rapid changes in sensor readings, although too much filtering can make the response slower.

---

## Sensor Configuration

### Sensor Weights

The default sensor weights are:

| Sensor | Default Weight |
|---|---:|
| S1 — Far left | -4 |
| S2 — Left | -2 |
| S3 — Center | 0 |
| S4 — Right | 2 |
| S5 — Far right | 4 |

Negative values represent positions to the left of the center. Positive values represent positions to the right.

The firmware uses these weights to estimate the line's position from the sensor readings.

### Line Polarity

The firmware includes a line-detection polarity setting.

Depending on the sensor module and track, the sensor may report LOW or HIGH when it detects the line.

If the robot reacts as though the line and background are reversed, check the sensor outputs and adjust the line-polarity setting.

Do not assume every IR sensor module uses the same output polarity.

### Sharp-Turn Threshold

Controls the error threshold at which the firmware switches to its special sharp-turn behavior.

A lower threshold activates sharp-turn handling more often.

A higher threshold restricts it to more extreme errors.

---

## Default Tuning Values

The following values are the initial settings used by the discussed firmware.

| Parameter | Default |
|---|---:|
| Base speed | 90 |
| Maximum speed | 255 |
| Minimum speed | 45 |
| Sharp inside speed | 35 |
| Sharp outside speed | 210 |
| Lost-line inside speed | 30 |
| Lost-line outside speed | 150 |
| Left trim | 0 |
| Right trim | 0 |
| Kp | 18 |
| Ki | 0 |
| Kd | 10 |
| Integral limit | 30 |
| Derivative filter | 0.70 |
| Sharp-turn threshold | 3 |
| Sensor weights | -4, -2, 0, 2, 4 |
| Initial line polarity | LOW |

These are starting values, not guaranteed optimal settings. Actual results depend on the hardware and track.

---

## Recommended Tuning Procedure

Tune the robot in stages rather than changing every parameter at once.

### Step 1: Check Sensor Readings

Lift the robot and move a line beneath each sensor.

Confirm that the dashboard displays the expected sensor states.

If the readings are reversed, check line polarity.

If a sensor never changes state, inspect its wiring, power supply, and output voltage.

### Step 2: Check Motor Directions

With the wheels off the ground, run the motors at low speed.

Confirm that the left and right motors turn in the intended direction.

If a motor runs backward, correct its wiring or motor-direction logic.

### Step 3: Begin at Low Speed

Set a conservative base speed.

Place the robot on a straight section of the track and verify that it follows the line.

Do not begin testing at maximum speed.

### Step 4: Tune Kp

Increase Kp gradually until the robot follows the line more decisively.

If it begins to oscillate strongly, reduce Kp.

### Step 5: Tune Kd

Increase Kd gradually if the robot oscillates around the line.

If corrections become noisy or erratic, reduce Kd or adjust the derivative filter.

### Step 6: Tune Ki Only If Necessary

Keep Ki at zero initially.

Consider integral correction only if the robot has a persistent bias that cannot be corrected through sensor alignment or motor trimming.

### Step 7: Tune Sharp Turns

Test a sharp turn at low speed.

Adjust the sharp-turn threshold and the inside/outside motor speeds.

Make small changes and repeat the test.

### Step 8: Tune Line-Loss Recovery

Test how the robot behaves when it temporarily loses the line.

Adjust recovery settings carefully. A recovery maneuver that is too aggressive may make the robot move farther away from the track.

### Step 9: Increase Speed Gradually

Once the robot is reliable at low speed, increase the base speed in small increments.

Retest straight sections, curves, and sharp turns after every significant change.

### Step 10: Save the Final Settings

Use the dashboard's SAVE function after finding settings that work reliably.

Keep a written copy of successful settings so you can restore them if later experiments make the robot less stable.

---

## Dashboard Actions

### RUN

Starts the line-following control process.

### STOP

Stops the robot's motor commands.

For physical safety, keep a reachable power switch available. Do not rely exclusively on a browser button as an emergency stop.

### APPLY

Applies the edited dashboard values to the running firmware.

### SAVE

Saves the configuration to nonvolatile ESP32 storage so the values can persist across restarts.

### RESET

Restores the firmware's default tuning parameters.

### Settings Text

The firmware can generate a text representation of the settings. Keep a copy of your final values for future reference.

---

## Troubleshooting

### The ESP32 Wi-Fi network does not appear

- Check that the ESP32 is powered.
- Check that the firmware uploaded successfully.
- Restart the board.
- Confirm that the board is running the correct sketch.

### The dashboard does not open

- Connect to the `LineFollower` Wi-Fi network.
- Open `http://192.168.4.1`.
- Disable mobile data temporarily if your device switches away from Wi-Fi.
- Restart the ESP32 and reconnect.

### The dashboard opens, but sensor readings are incorrect

- Check the sensor power and ground connections.
- Confirm the GPIO assignments.
- Test the sensors over both the line and the background.
- Check line polarity.
- Verify that sensor output voltages are safe for ESP32 GPIO pins.

### The robot moves backward

Check motor direction wiring and the direction-control logic.

### The robot turns the wrong way

Check sensor order, sensor weights, motor directions, and the sign of the calculated line error.

### The robot stops when the line reaches an outer sensor

- Check the outer sensor readings.
- Verify that the sensor order matches the wiring table.
- Review the sharp-turn threshold.
- Reduce base speed while testing.
- Review the line-loss recovery behavior.

### The robot oscillates

- Reduce Kp slightly.
- Adjust Kd gradually.
- Check that the sensor array is firmly mounted.
- Check wheel alignment and mechanical play.
- Reduce speed while tuning.

### The robot cannot make sharp turns

- Reduce base speed.
- Review the sharp-turn threshold.
- Adjust the inside and outside motor speeds.
- Verify that the outer sensors detect the line correctly.
- Check whether the chassis has enough traction.

### Motors do not move

- Confirm that STBY is driven to the enabled state by the firmware.
- Check VM, VCC, and GND.
- Verify motor wiring.
- Check the battery and buck converter.
- Confirm that the motor driver can handle the motors' startup and stall current.

### ESP32 upload fails

If you see a message such as:

`Failed to communicate with the flash chip`

Try:

1. Set the upload speed to `115200`.
2. Disconnect external hardware temporarily.
3. Use a known-good USB data cable.
4. Close Serial Monitor.
5. Confirm the selected board and port.
6. Retry the upload.

---

## Circuit Diagram

Keep the circuit diagram PDF alongside this README for quick reference.

Suggested filename:

`esp32_line_follower_circuit_diagram.pdf`

The diagram should be used together with the pin tables in this document. Verify the actual labels on your hardware before applying power.

---

## Important Limitations

- Performance depends on sensor quality, track contrast, lighting, wheel traction, motor response, and chassis design.
- The dashboard values are tuning controls, not guarantees of a particular speed or lap time.
- A digital five-sensor array provides discrete readings rather than a continuous measurement of line position.
- The ESP32 GPIO pins must never be exposed to voltages above their permitted input range.
- Motor stall current can damage an undersized driver, converter, battery, or wiring.
- Test at low speed before attempting high-speed operation.

---

## Future Improvements

Possible future improvements include:

- A calibration routine for the five IR sensors.
- Automatic speed reduction before sharp turns.
- Improved line-loss detection and recovery.
- Lap counting and timing.
- A live graph of line-position error.
- A track-specific tuning profile.
- Improved motor control using hardware PWM and a more advanced control loop.
- A physical emergency-stop switch.

---

## License

This project is intended for educational and DIY robotics use.

Add your preferred license here if you plan to publish or distribute the firmware.
