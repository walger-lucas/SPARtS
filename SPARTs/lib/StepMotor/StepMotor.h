#ifndef STEPMOTOR_H
#define STEPMOTOR_H
#include <AccelStepper.h>

#define STEP_PIN_X 25
#define DIR_PIN_X 33

#define STEP_PIN_Y 27
#define DIR_PIN_Y 26

#define STEP_PIN_R 13
#define DIR_PIN_R 14

#define STEPS_TO_BIN_H 100
#define STEPS_TO_BIN_V 150

#define MAX_SPEED 1000
#define ACCELERATION 200

#define STEPS_TO_UP_BIN 50

#define RETRIEVAL_POS_X 0
#define RETRIEVAL_POS_Y 5

#define END_X_PIN 34
#define END_Y_PIN 35

class StepMotor
{
private:
    int current_x_position;
    int current_y_position;
    bool end_x;
    bool end_y;
    AccelStepper motorX;
    AccelStepper motorY;
    AccelStepper motorConveyor;
public:
    StepMotor();
    ~StepMotor();
    void begin();
    void goToBin(int x_position, int y_position);
    void goToRetrieval();
    void upBin();
    void downBin();
    void goToStartPosition();

};


#endif




