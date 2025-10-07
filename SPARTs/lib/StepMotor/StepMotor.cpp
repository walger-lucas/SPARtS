#include "StepMotor.h"


StepMotor::StepMotor(/* args */)
{
    current_x_position = 0;
    current_y_position = 0;
    convenyor_position = 0;
    end_x = 0;
    end_y = 0;
}

StepMotor::~StepMotor()
{
}

void StepMotor::begin()
{
    motorX = AccelStepper(AccelStepper::DRIVER, STEP_PIN_X, DIR_PIN_X);
    motorY = AccelStepper(AccelStepper::DRIVER, STEP_PIN_Y, DIR_PIN_Y);
    motorConvenyor = AccelStepper(AccelStepper::DRIVER, STEP_PIN_R, DIR_PIN_R);  
    motorX.setMaxSpeed(MAX_SPEED);
    motorX.setAcceleration(ACCELERATION);
    motorY.setMaxSpeed(MAX_SPEED);
    motorY.setAcceleration(ACCELERATION);
    motorConvenyor.setMaxSpeed(MAX_SPEED);
    motorConvenyor.setAcceleration(ACCELERATION);

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
    goToBin(RETRIEVAL_POS_X, RETRIEVAL_POS_Y);
}

void StepMotor::convenyorNext(){
    if (convenyor_position <= 3){
        motorConvenyor.moveTo(CONVENYOR_STEPS);
        motorConvenyor.runToPosition();
        convenyor_position++;
    }
}

void StepMotor::convenyorStartPosition(){

}
