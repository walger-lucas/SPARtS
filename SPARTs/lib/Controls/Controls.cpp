#include "Controls.h"
namespace controls {

    int XYControl::getSpeed(const Speed speed)
    {
        switch (speed)
        {
        case Speed::SLOW: return MAX_SPEED/4;
        case Speed::MEDIUM: return MAX_SPEED/2;
        case Speed::FAST: return MAX_SPEED;
        default:    return MAX_SPEED/2;
        }
    }

    int XYControl::getAcc(const Speed speed)
    {
        switch (speed)
        {
        case Speed::SLOW: return ACCELERATION/4;
        case Speed::MEDIUM: return ACCELERATION/2;
        case Speed::FAST: return ACCELERATION;
        default:    return ACCELERATION/2;
        }
    }

    bool XYControl::moveTo(const Pos2i pos,const Speed speed,bool reset)
    {

        int x {min(pos.x,MAX_X)};
        int y {min(pos.y,MAX_Y)};
        printf("Moving to x:%d y:%d\n",x,y);

        motorX.setMaxSpeed(getSpeed(speed)*2);
        motorY.setMaxSpeed(getSpeed(speed)*2);
        motorX.setAcceleration(getAcc(speed)*8);
        motorY.setAcceleration(getAcc(speed)*8);

        motorX.moveTo(x);
        

        bool endX {digitalRead(END_X_PIN)};
        

        unsigned long start {millis()};
        bool runX = true, runY = true;
        
        do {
            runX = motorX.run();

            if(!digitalRead(END_X_PIN)&&reset)
            {
                delay(1);
                if(!digitalRead(END_X_PIN))
                {
                    
                    motorX.setCurrentPosition(0);
                    motorX.moveTo(max(0,x));
                }
            }

        } while((runX)&& (millis()-start)<TIMEOUT+3000);

        motorY.moveTo(y);
        bool endY {digitalRead(END_Y_PIN)};
        do {         
           
           runY = motorY.run();

           if(!digitalRead(END_Y_PIN)&&reset)
            {
                delay(1);
                if(!digitalRead(END_Y_PIN))
                {
                    motorY.setCurrentPosition(0);
                    motorY.moveTo(max(0,y));
                }
            }

        } while((runY)&& (millis()-start)<TIMEOUT+3000);
        bool result = (millis()-start)<TIMEOUT;
        motorX.stop();
        motorY.stop();
        return result;
    }

    bool XYControl::calibrate()
    {
        delay(1000);

        bool rt = moveTo({-20000,-20000},Speed::MEDIUM,true);
        delay(100);
        motorX.move(-50);
        motorY.move(-50);
        motorX.runToPosition();
        motorY.runToPosition();
        motorX.setCurrentPosition(0);
        motorY.setCurrentPosition(0);
        return rt;
    }

    XYControl::XYControl()
    {
        motorX = AccelStepper(AccelStepper::DRIVER, STEP_PIN_X, DIR_PIN_X);
        motorY = AccelStepper(AccelStepper::DRIVER, STEP_PIN_Y, DIR_PIN_Y);

        motorX.setMaxSpeed(MAX_SPEED);
        motorX.setAcceleration(ACCELERATION);
        motorY.setMaxSpeed(MAX_SPEED);
        motorY.setAcceleration(ACCELERATION);

        motorX.setCurrentPosition(0);
        motorY.setCurrentPosition(0);

        pinMode(END_X_PIN, INPUT_PULLUP);
        pinMode(END_Y_PIN, INPUT_PULLUP);
    }

    ConveyorControl::ConveyorControl()
    {
        motorConveyor = AccelStepper(AccelStepper::DRIVER, STEP_PIN_R, DIR_PIN_R); 

        motorConveyor.setMaxSpeed(MAX_SPEED);
        motorConveyor.setAcceleration(ACCELERATION);
        motorConveyor.setCurrentPosition(0);

        pinMode(END_CONVEYOR_PIN, INPUT_PULLUP);
    }

    bool ConveyorControl::next()
    {
        if (cur_pos < MAX_BIN){
            motorConveyor.move(CONVEYOR_STEPS);
            motorConveyor.runToPosition();
            cur_pos++;
        }
        if(cur_pos >= MAX_BIN)
        {
            motorConveyor.move(CONVEYOR_STEPS/3);
            delay(200);
            if(start())
            {
                cur_pos = 0;
                return true;
            }
            return false;
        }
        return true;
        
    }

    bool ConveyorControl::start()
    {
        motorConveyor.setMaxSpeed(MAX_SPEED/2);
        motorConveyor.setAcceleration(ACCELERATION/2);
        motorConveyor.move(-200000);
        int end = digitalRead(END_CONVEYOR_PIN);
        unsigned long time_start = millis();
        while(end && millis()-time_start<TIMEOUT){
                motorConveyor.run();
                if(!digitalRead(END_CONVEYOR_PIN))
                {
                    delay(1);
                    end = digitalRead(END_CONVEYOR_PIN);
                }
        }
        motorConveyor.stop();
        motorConveyor.setCurrentPosition(0);
        motorConveyor.setMaxSpeed(MAX_SPEED);
        motorConveyor.setAcceleration(ACCELERATION);
        if(millis()-time_start>=TIMEOUT)
            return false;
        motorConveyor.move(BEGIN_STEP_OFFSET);
        motorConveyor.runToPosition();
        return true;
    }
    int ConveyorControl::getPos()
    {
        return cur_pos;
    }

    int PlatformControl::getSpeed(Direction dir,Speed speed)
    {
        return (dir == Direction::EXTEND)? 180 : 0;
        int signal = (dir == Direction::EXTEND)? 1 : -1;
        switch (speed)
        {
        case Speed::SLOW: return (90 + signal*MAX_SPEED*3.5/4);
        case Speed::MEDIUM: return (90 + signal*MAX_SPEED*3.5/4);
        case Speed::FAST: return (90 + signal*MAX_SPEED);
        default:    return (90 + signal*MAX_SPEED);
        }
    }

    PlatformControl::PlatformControl()
    {

    }

    void PlatformControl::setup()
    {
        ESP32PWM::allocateTimer(0);
        ESP32PWM::allocateTimer(1);
        ESP32PWM::allocateTimer(2);
        ESP32PWM::allocateTimer(3);

        pos = Direction::EXTEND;
        pinMode(23,OUTPUT);
        servo.attach(SERVO_PIN,500,2400);
        servo.setPeriodHertz(50); 
        servo.write(90);
        pinMode(END_PIN,INPUT_PULLUP);
    }
    bool PlatformControl::onEnd()
    {
        return digitalRead(END_PIN);
    }
    bool PlatformControl::calibrate()
    {
        pos = Direction::EXTEND;
        servo.write(getSpeed(Direction::EXTEND,Speed::FAST));
        delay(400);
        servo.write(90);
        return move(Direction::RETRACT,Speed::MEDIUM);
    }

    bool PlatformControl::move(Direction dir, Speed speed)
    {
        if(pos==dir)
           return true;
        delay(500);
        unsigned long start = millis();
        servo.write(getSpeed(dir,speed));
        if(onEnd())
        {
            delay(700);   
            //while(onEnd() && millis()-start < 1500){};
        }
        //servo.write(getSpeed(dir,Speed::FAST));
        while(!onEnd() && millis()-start< TIMEOUT){};
        servo.write(90);
        
        if(millis()-start >= TIMEOUT)
        {
            pos = Direction::EXTEND;
            return false;
        }
        pos = dir;
        return true;
    }


    void MovementControl::fetch()
    {
        rfid_t rfid;
        readAndFetch(rfid);
    }

    void MovementControl::store(){
        Pos2i initial = xy_table.getPos();
        xy_table.moveTo(initial+Pos2i{0,STEPS_TO_UP_BIN},Speed::MEDIUM);
        platform.move(PlatformControl::Direction::EXTEND,Speed::MEDIUM);
        xy_table.moveTo(initial,Speed::SLOW);
        platform.move(PlatformControl::Direction::RETRACT,Speed::FAST);
    }
    bool MovementControl::read(rfid_t& rfid)
    {
        bool rfid_read = false;
        Pos2i initial = xy_table.getPos();
        platform.move(PlatformControl::Direction::EXTEND,Speed::FAST);
        xy_table.moveTo(initial+Pos2i{0,STEPS_TO_UP_BIN},Speed::SLOW);
        delay(50);
        if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
        {
            rfid_read = true;
            rfid.fill(0);
            for(int i =0;i<mfrc522.uid.size;i++)
                rfid[i] = mfrc522.uid.uidByte[i];
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
        }
        xy_table.moveTo(initial);
        platform.move(PlatformControl::Direction::RETRACT,Speed::FAST);

        return rfid_read;
    }

    bool MovementControl::readAndFetch(rfid_t& rfid)
    {
        bool rfid_read = false;
        Pos2i initial = xy_table.getPos();
        platform.move(PlatformControl::Direction::EXTEND,Speed::FAST);
        xy_table.moveTo(initial+Pos2i{0,STEPS_TO_UP_BIN},Speed::SLOW);
        delay(50);
        if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
        {
            rfid_read = true;
            rfid.fill(0);
            for(int i =0;i<mfrc522.uid.size;i++)
                rfid[i] = mfrc522.uid.uidByte[i];
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
        }
        platform.move(PlatformControl::Direction::RETRACT,Speed::SLOW);
        xy_table.moveTo(initial);

        return rfid_read;
    }

    bool MovementControl::readWithoutExtending(rfid_t& rfid)
    {
        bool rfid_read = false;
        Pos2i initial = xy_table.getPos();
        xy_table.moveTo(initial+Pos2i{0,STEPS_TO_UP_BIN},Speed::SLOW);
        delay(50);
        if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
        {
            rfid_read = true;
            rfid.fill(0);
            for(int i =0;i<mfrc522.uid.size;i++)
                rfid[i] = mfrc522.uid.uidByte[i];
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
        }
        xy_table.moveTo(initial);
        return rfid_read;  
    }
    
    void MovementControl::init()
    {
        platform.setup();
        mfrc522.PCD_Init();
        platform.calibrate();
        xy_table.calibrate();        
    }

}


/*
StepMotor::StepMotor()
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
    motorConvenyor.setCurrentPosition(0);

    pinMode(END_X_PIN, INPUT_PULLUP);
    pinMode(END_Y_PIN, INPUT_PULLUP);
    pinMode(END_CONVEYOR_PIN,INPUT_PULLUP);
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

bool StepMotor::convenyorNext(){
    if (convenyor_position < 5){
        motorConvenyor.move(CONVEYOR_STEPS);
        motorConvenyor.runToPosition();
        convenyor_position++;
    }
    if(convenyor_position >= 5)
    {
        delay(200);
        if(convenyorStartPosition())
        {
            convenyor_position = 0;
            return true;
        }
        return false;
    }

    return true;
    
    
    
}

bool StepMotor::convenyorStartPosition(){
    motorConvenyor.setCurrentPosition(20000);
    motorConvenyor.move(-20000);
    int end = digitalRead(END_CONVEYOR_PIN);
    unsigned long time_start = millis();
    while(end && millis()-time_start<5000){
            motorConvenyor.run();
            end = digitalRead(END_CONVEYOR_PIN);
    }
    motorConvenyor.stop();
    motorConvenyor.setCurrentPosition(0);
    if(millis()-time_start>5000)
        return false;
    return true;

}
*/

