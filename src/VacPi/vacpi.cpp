#include <Arduino.h>
#include <SPI.h>
#include <math.h>
#include "AFMotor.h"
#include "Shell.h"
#include "VacPi.h"

#define CS_SENSORS 1
#define SENSOR_LEFT 6
#define SENSOR_RIGHT 7
#define SENSOR_CENTER 5
#define SENSOR_FRONT 4
#define SENSOR_VACUUM 3
#define SENSOR_THRES 100
#define A 1.86 // x = 100, y = 255
#define B 20 // x = 0, y = 20
#define C 1.55 // x = 50, y = 180
#define SPIRAL_FKT(x) A*pow(x,C)+B

#define MIN_SPEED 20
#define SLOW 50
#define MID 65
#define FAST 100
#define BRUSH_SPEED 100
#define MAX_SPEED 255
#define FOURTY_FIVE_DEG_DELAY 750

bool debug = false;
bool test = false;

SPISettings SENSORS(2000000, MSBFIRST, SPI_MODE0);

class ArduinoMovements: public VacPi::Movements {
public:
    ArduinoMovements () {}
    void setup() {
        engine_left.setSpeed(FAST);
        engine_right.setSpeed(FAST);
        engine_brushes.setSpeed(BRUSH_SPEED);
        engine_vacuum.setSpeed(MAX_SPEED);
    }
    void stopAll() {
            engine_left.run(RELEASE);
            engine_right.run(RELEASE);
            engine_brushes.run(RELEASE);
            engine_vacuum.run(RELEASE);
    }
    void startVacuum() {
        engine_brushes.run(FORWARD);
        engine_vacuum.run(FORWARD);
    }
    void stopVacuum() {
        engine_brushes.run(RELEASE);
        engine_vacuum.run(RELEASE);
    }
    void moveStraight(VacPi::Direction direction) {
        engine_left.setSpeed(MAX_SPEED);;
        engine_right.setSpeed(MAX_SPEED);;
        if (direction == VacPi::Forward) {
            engine_left.run(FORWARD);
            engine_right.run(FORWARD);
        } else {
            engine_left.run(BACKWARD);
            engine_right.run(BACKWARD);
        }
    }
    void rotate45(VacPi::Turn turn) {
        engine_left.setSpeed(MAX_SPEED);;
        engine_right.setSpeed(MAX_SPEED);;
        if (turn == VacPi::LEFT) {
            engine_left.run(BACKWARD);
            engine_right.run(FORWARD);
        } else {
            engine_left.run(FORWARD);
            engine_right.run(BACKWARD);
        }
        delay(FOURTY_FIVE_DEG_DELAY);
    }
    void spiral(VacPi::Turn turn, int timer) {
        int spiral_slow = SPIRAL_FKT(timer);
        if (spiral_slow > MAX_SPEED) spiral_slow = MAX_SPEED;
        if (turn == VacPi::RIGHT) {
            engine_left.setSpeed(MAX_SPEED);;
            engine_right.setSpeed(spiral_slow);;
        } else {
            engine_left.setSpeed(spiral_slow);;
            engine_right.setSpeed(MAX_SPEED);;
        }
        engine_left.run(FORWARD);
        engine_right.run(FORWARD);
    }
    void curve(VacPi::Direction direction, VacPi::Turn turn, VacPi::Speed speed) {
        int engineSpeed;
        if (speed == VacPi::Slow) {
            engineSpeed = SLOW;
        } else {
            engineSpeed = FAST;
        }
        if (turn == VacPi::RIGHT) {
            engine_left.setSpeed(MAX_SPEED);
            engine_right.setSpeed(engineSpeed);
        } else {
            engine_left.setSpeed(engineSpeed);
            engine_right.setSpeed(MAX_SPEED);
        }
        if (direction == VacPi::Forward) {
            engine_left.run(FORWARD);
            engine_right.run(FORWARD);
        } else {
            engine_left.run(BACKWARD);
            engine_right.run(BACKWARD);
        }
    }
private:
    AF_DCMotor engine_left = AF_DCMotor(3);
    AF_DCMotor engine_right = AF_DCMotor(1);
    AF_DCMotor engine_brushes = AF_DCMotor(2);
    AF_DCMotor engine_vacuum = AF_DCMotor(4);
};

ArduinoMovements movements;
VacPi::Looper looper(&movements);

int sensor_read(int input_sel) {
    int val = analogRead(input_sel);
    if (debug) {
        Serial.print(input_sel);
        Serial.print("\t");
        Serial.println(val);
    }
    return val <= SENSOR_THRES;
}

VacPi::Obstruction check_sensors() {
    VacPi::Obstruction state;
    state.dustLevel = false;// sensor_read(SENSOR_VACUUM);
    state.left = sensor_read(SENSOR_LEFT);
    state.right = sensor_read(SENSOR_RIGHT);
    state.center = sensor_read(SENSOR_CENTER);
    state.front = sensor_read(SENSOR_FRONT);
    return state;
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting VacPi:");
    movements.setup();
    Serial.flush();
}


void loop() {
    //Invoke shell
    if (Serial.available() > 0) {
        char buffer[255];
        int length = Serial.readBytesUntil(';', buffer, 255);
        buffer[length] = '\0';
        Shell::Command command = Shell::evaluate(buffer);
        if (command == Shell::DEBUG) debug = !debug;
        else if (command == Shell::TEST) test = !test;
    }
    Serial.flush();
    if (test) {
        movements.stopAll();
        Serial.println("Sensor Test:");
        Serial.print("Vacuum\tLeft\tRight\tCenter\tFront");
        for (int i = 0; i < 10; ++i) {
            VacPi::Obstruction obstruction = check_sensors();
            Serial.print(obstruction.dustLevel);
            Serial.print('\t');
            Serial.print(obstruction.left);
            Serial.print('\t');
            Serial.print(obstruction.right);
            Serial.print('\t');
            Serial.print(obstruction.center);
            Serial.print('\t');
            Serial.print(obstruction.front);
            delay(500);
            Serial.println();
        }
        Serial.flush();

        Serial.println("Engine test:");
        Serial.println("Vacuum:");
        Serial.flush();
        movements.startVacuum();
        delay(5000);
        movements.stopAll();
        Serial.println("Left/Right Forward");
        Serial.flush();
        movements.moveStraight(VacPi::Forward);
        delay(5000);
        movements.stopAll();
        Serial.println("Left Forward, Right Backward (Rotate right)");
        Serial.flush();
        movements.rotate45(VacPi::RIGHT);
        delay(5000);
        movements.stopAll();
        Serial.println("Tests Done");
        Serial.flush();
        delay(10000);
    } else {
        //Read Sensors
        VacPi::Obstruction obstruction = check_sensors();

        //Run VacPi loop
        looper.loop(obstruction);

        delay(100);
    }
}
