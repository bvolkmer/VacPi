#include <Arduino.h>
#include <SPI.h>
#include <math.h>
#include "AFMotor.h"
#include "Shell.h"

#define CS_SENSORS 1
#define SENSOR_LEFT 6
#define SENSOR_RIGHT 7
#define SENSOR_CENTER 5
#define SENSOR_FRONT 4
#define SENSOR_VACUUM 3 
#define SENSOR_THRES 100
#define SLOW 50
#define MID 65
#define FAST 100
#define BRUSH_SPEED 100
#define MIN_SPEED 20
#define MAX_SPEED 255
#define COUNTER_THRES 150
#define A 5.5 // x = 100, y = 60
#define B 20 // x = 0, y = 20
#define C 0.43 // x = 20, y = 40
#define SPIRAL_FKT(x) A*pow(x,C)+B
#define SPIRAL_THRES 120
#define QUARTER_DEGREE_DELAY 2000
#define FORWARD_RIGHT_DELAY 500

#define HAS_OBSTACLE(state) state.right || state.left || state.front || state.center
#define IS_FLYING(state) state.right && state.left && state.center

bool debug = false;

AF_DCMotor engine_left(2);
AF_DCMotor engine_right(1);
AF_DCMotor engine_brushes(3);
AF_DCMotor engine_vacuum(4);
SPISettings SENSORS(2000000, MSBFIRST, SPI_MODE0);
int loop_counter = 0;
bool circling = true;
bool circling_clockwise = true;
int spiral_slow = MIN_SPEED;

#define GLOBAL_TIMER_THRES 500
#define LOCAL_TIMER_THRES 150
#define LOCAL_TIMER_EDGE_THRES 10
int global_timer = 0;
int local_timer = 0;

#define MODE_EDGE 0
#define MODE_STRAIGHT 1
#define MODE_CIRCLE 2
int mode = MODE_EDGE;

#define MODE_EDGE_STRAIGHT 0
#define MODE_EDGE_TURN 0 
int edge_mode = MODE_EDGE_STRAIGHT;

struct sensor_state_t {
    bool left;
    bool right;
    bool center;
    bool front;
};

void setVacuum(bool on) {
    if (on) {
        engine_brushes.run(FORWARD);
        engine_vacuum.run(FORWARD);
    } else {
        engine_brushes.run(RELEASE);
        engine_vacuum.run(RELEASE);
    }
}

int sensor_read(int input_sel) {
    int val = analogRead(input_sel);
    if (debug) {
        Serial.print(input_sel);
        Serial.print("\t");
        Serial.println(val);
    }
    return val <= SENSOR_THRES;
}

struct sensor_state_t check_sensors() {
    struct sensor_state_t state;
    state.left = sensor_read(SENSOR_LEFT);
    state.right = sensor_read(SENSOR_RIGHT);
    state.center = sensor_read(SENSOR_CENTER);
    state.front = sensor_read(SENSOR_FRONT);
    return state;
}

void setup() {
    Serial.begin(9600);

    Serial.println("Starting VacPi:");

    //Engine init
    engine_left.setSpeed(FAST);
    engine_right.setSpeed(FAST);
    engine_brushes.setSpeed(BRUSH_SPEED);
    engine_vacuum.setSpeed(MAX_SPEED);

    setVacuum(true);
}

void drive(int dir_right, int dir_left, int speed_right, int speed_left) {
    engine_right.run(dir_right);
    engine_left.run(dir_left);
    engine_right.setSpeed(speed_right);
    engine_left.setSpeed(speed_left);
}

void run_in_circle() {
    spiral_slow = SPIRAL_FKT(loop_counter/2);
    if (spiral_slow > SPIRAL_THRES) {
        spiral_slow = MIN_SPEED;
        loop_counter = 0;
    }
    drive(FORWARD, FORWARD, FAST, spiral_slow);
}

void run_straight() {
    drive(FORWARD, FORWARD, FAST, FAST);
}

void run_forward_right() {
    drive(FORWARD, FORWARD, SLOW, FAST);
}

void run_back_right() {
    drive(BACKWARD, BACKWARD, SLOW, FAST);
    delay(500);
}

void turn_45() {
    drive(FORWARD, BACKWARD, FAST, FAST);
    delay(QUARTER_DEGREE_DELAY);
}

void stop() {
    drive(RELEASE, RELEASE, FAST, FAST);
}

void setMode(int mode) {
    switch(mode) {
        case MODE_EDGE:
            mode = MODE_EDGE;
            local_timer = 0;
            edge_mode = MODE_EDGE_STRAIGHT;
            break;
        case MODE_STRAIGHT:
            mode = MODE_STRAIGHT;
            local_timer = 0;
            break;
        case MODE_CIRCLE:
            mode = MODE_CIRCLE;
            spiral_slow = MIN_SPEED;
            break;
    }
}

void loop() {
    if (Serial.available() > 0) {
        char buffer[255];
        int length = Serial.readBytesUntil(';', buffer, 255);
        buffer[length] = '\0';
        Shell::Command command = Shell::evaluate(buffer);
        if (command == Shell::Command::DEBUG) debug = !debug;
    }
    global_timer++;
    struct sensor_state_t state = check_sensors();
    if (IS_FLYING(state)) {
        setVacuum(false);
        stop();
    } else  {
        setVacuum(true);
        if (global_timer > GLOBAL_TIMER_THRES) {
            if (mode == MODE_EDGE) setMode(MODE_STRAIGHT);
            else setMode(MODE_EDGE);
            global_timer = 0;
        }
        switch(mode) {
            case MODE_EDGE:
                local_timer++;
                if (HAS_OBSTACLE(state)) {
                    run_back_right();
                    local_timer = 0;
                    edge_mode = MODE_EDGE_STRAIGHT;
                }
                else {
                    if (edge_mode == MODE_EDGE_STRAIGHT) {
                        if (local_timer > LOCAL_TIMER_EDGE_THRES) edge_mode = MODE_EDGE_TURN;
                        else run_straight();
                    } else {
                        run_forward_right();
                    }
                }
                break;
            case MODE_STRAIGHT:
                local_timer++;
                if(HAS_OBSTACLE(state)) turn_45();
                else {
                    if (local_timer > LOCAL_TIMER_THRES) setMode(MODE_CIRCLE);
                    else run_straight();
                }
                break;
            case MODE_CIRCLE:
                if (HAS_OBSTACLE(state)) setMode(MODE_STRAIGHT);
                else run_in_circle();
        }
    }
}
