#include <Stepper.h>

const int DEFAULT_STEPS_PER_REV = 2048;    // default theoretical value
Stepper myStepper(DEFAULT_STEPS_PER_REV, 8, 10, 9, 11);

String inputString = "";
bool stringComplete = false;

float stepsPerRev = DEFAULT_STEPS_PER_REV; // can be changed via command (calibration)
long currentPositionSteps = 0;              // keeps track of position (in steps)

void setup() {
  Serial.begin(9600);
  myStepper.setSpeed(10); // default RPM
  Serial.println("Ready. Examples:");
  Serial.println("rotate 90 degrees clockwise");
  Serial.println("rotate 45 degrees counterclockwise");
  Serial.println("rpm 12");
  Serial.println("home");
  Serial.println("set_steps_per_rev 2032   <-- calibrate");
  Serial.println("position");
}

void loop() {
  if (stringComplete) {
    stringComplete = false;
    String cmd = inputString;
    cmd.trim();
    cmd.toLowerCase();

    if (cmd.startsWith("rotate")) {
      // find degree token (accept "degrees", "degree", "deg")
      int degIndex = cmd.indexOf("degrees");
      if (degIndex < 0) degIndex = cmd.indexOf("degree");
      if (degIndex < 0) degIndex = cmd.indexOf("deg");

      if (degIndex > 0) {
        String numStr = cmd.substring(7, degIndex); // between "rotate " and "degrees"
        numStr.trim();
        int degrees = numStr.toInt();

        if (degrees > 0) {
          // detect direction: check "counterclockwise" first to avoid substring bug
          int dir = 0; // +1 = clockwise, -1 = counterclockwise
          if (cmd.indexOf("clockwise") >= 0 || cmd.indexOf("ccw") >= 0) dir = +1;
          else if (cmd.indexOf("counterclockwise") >= 0 || cmd.indexOf("cw") >= 0) dir = -1;

          if (dir == 0) {
            Serial.println("Specify direction: clockwise or counterclockwise (or cw/ccw)");
          } else {
            long steps = (long)(( (float)degrees * stepsPerRev / 360.0 ) + 0.5); // rounded
            myStepper.step(dir * steps);
            currentPositionSteps += (dir * steps);
            // keep position in range [0, stepsPerRev)
            long spr = (long)stepsPerRev;
            currentPositionSteps %= spr;
            if (currentPositionSteps < 0) currentPositionSteps += spr;

            Serial.print("Rotated ");
            Serial.print(degrees);
            Serial.print(" degrees ");
            Serial.println(dir > 0 ? "clockwise" : "counterclockwise");
          }
        } else {
          Serial.println("Invalid degree value (must be > 0).");
        }
      } else {
        Serial.println("Format: rotate <number> degrees clockwise/counterclockwise");
      }
    }
    else if (cmd.startsWith("rpm")) {
      // set speed
      String val = cmd.substring(3);
      val.trim();
      int r = val.toInt();
      if (r > 0) {
        myStepper.setSpeed(r);
        Serial.print("RPM set to ");
        Serial.println(r);
      } else {
        Serial.println("Invalid rpm value!");
      }
    }
    else if (cmd == "home") {
      currentPositionSteps = 0;
      Serial.println("Home set (position = 0°).");
    }
    else if (cmd.startsWith("set_steps_per_rev")) {
      String val = cmd.substring(String("set_steps_per_rev").length());
      val.trim();
      float newVal = val.toFloat();
      if (newVal >= 100.0) {
        stepsPerRev = newVal;
        Serial.print("stepsPerRev set to ");
        Serial.println(stepsPerRev);
      } else {
        Serial.println("Invalid value. Give measured steps per full 360° (e.g. 2038)");
      }
    }
    else if (cmd == "position") {
      float angle = ((float)currentPositionSteps / stepsPerRev) * 360.0;
      if (angle < 0) angle += 360.0;
      Serial.print("Position: ");
      Serial.print(currentPositionSteps);
      Serial.print(" steps  =  ");
      Serial.print(angle, 2);
      Serial.println(" degrees");
    }
    else {
      Serial.println("Unknown command. Try: rotate / rpm / home / set_steps_per_rev / position");
    }

    inputString = "";
  }
}

// Serial input capture
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else if (inChar != '\r') {
      inputString += inChar;
    }
  }
}
