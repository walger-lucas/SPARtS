#include "StepMotor.h"


StepMotor::StepMotor(/* args */)
{
    current_x_position = 0;
    current_y_position = 0;
}

StepMotor::~StepMotor()
{
}

void StepMotor::begin()
{
    motorX = AccelStepper(AccelStepper::DRIVER, STEP_PIN_X, DIR_PIN_X);
    motorY = AccelStepper(AccelStepper::DRIVER, STEP_PIN_Y, DIR_PIN_Y);
    motorConveyor = AccelStepper(AccelStepper::DRIVER, STEP_PIN_R, DIR_PIN_R);  
    motorX.setMaxSpeed(MAX_SPEED);
    motorX.setAcceleration(ACCELERATION);
    motorY.setMaxSpeed(MAX_SPEED);
    motorY.setAcceleration(ACCELERATION);
    motorConveyor.setMaxSpeed(MAX_SPEED);
    motorConveyor.setAcceleration(ACCELERATION);

    pinMode(END_X_PIN, INPUT_PULLUP);
    pinMode(END_Y_PIN, INPUT_PULLUP);
}

//Leva os motores para o sensor de fim de curso
void StepMotor::goToStartPosition(){
    //diminui a aceleração e velocidade para evitar danos 
    motorX.setAcceleration(ACCELERATION/10);
    motorY.setAcceleration(ACCELERATION/10);
    motorX.setMaxSpeed(MAX_SPEED/10);
    motorY.setMaxSpeed(MAX_SPEED/10);

    end_x = digitalRead(END_X_PIN);
    end_y = digitalRead(END_Y_PIN);
    motorX.moveTo(-2000);
    motorY.moveTo(-2000);
    while(end_x == HIGH || end_y == HIGH){
        if(end_x == HIGH){
            motorX.run();
        }
        if(end_y == HIGH){
            motorY.run();
        }
        end_x = digitalRead(END_X_PIN);
        end_y = digitalRead(END_Y_PIN);
    }
    current_x_position = 0;
    current_y_position = 0;

    //volta com a aceleração e velocidade normais
    motorX.setAcceleration(ACCELERATION);
    motorY.setAcceleration(ACCELERATION);
    motorX.setMaxSpeed(MAX_SPEED);
    motorY.setMaxSpeed(MAX_SPEED);
}

void StepMotor::goToBin(int x_position, int y_position)
{
    if (x_position != current_x_position)
    {
        int steps_x = (x_position - current_x_position) * STEPS_TO_BIN_H;
        motorX.moveTo(steps_x);
        
    }
    if (y_position != current_y_position)
    {
        int steps_y = (y_position - current_y_position) * STEPS_TO_BIN_V;
        motorY.moveTo(steps_y);
    }
    while(motorX.distanceToGo() != 0 || motorY.distanceToGo() != 0){
        motorX.run();
        motorY.run();
    }
    current_x_position = x_position;
    current_y_position = y_position;
}

void StepMotor::upBin(){
    motorX.setAcceleration(ACCELERATION/10);
    motorY.setAcceleration(ACCELERATION/10);
    motorX.setMaxSpeed(MAX_SPEED/10);
    motorY.setMaxSpeed(MAX_SPEED/10);

    motorY.moveTo(STEPS_TO_UP_BIN);
    motorY.runToPosition();

    motorX.setAcceleration(ACCELERATION);
    motorY.setAcceleration(ACCELERATION);
    motorX.setMaxSpeed(MAX_SPEED);
    motorY.setMaxSpeed(MAX_SPEED);
}

void StepMotor::downBin(){

    motorX.setAcceleration(ACCELERATION/10);
    motorY.setAcceleration(ACCELERATION/10);
    motorX.setMaxSpeed(MAX_SPEED/10);
    motorY.setMaxSpeed(MAX_SPEED/10);
    
    motorY.moveTo(-STEPS_TO_UP_BIN);
    motorY.runToPosition();
    
    motorX.setAcceleration(ACCELERATION);
    motorY.setAcceleration(ACCELERATION);
    motorX.setMaxSpeed(MAX_SPEED);
    motorY.setMaxSpeed(MAX_SPEED);
}


void StepMotor::goToRetrieval()
{
    if (current_x_position != RETRIEVAL_POS_X)
    {
        int steps_x = (RETRIEVAL_POS_X - current_x_position) * STEPS_TO_BIN_H;
        motorX.moveTo(steps_x);
        motorX.runToPosition();
        current_x_position = RETRIEVAL_POS_X;
    }
    if (current_y_position != RETRIEVAL_POS_Y) 
    {
        int steps_y = (RETRIEVAL_POS_Y - current_y_position) * STEPS_TO_BIN_V;
        motorY.moveTo(steps_y);
        motorY.runToPosition();
        current_y_position = RETRIEVAL_POS_Y;
    }
    
}
