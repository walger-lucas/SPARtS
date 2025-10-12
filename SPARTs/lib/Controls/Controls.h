#ifndef STEPMOTOR_H
#define STEPMOTOR_H
#include <AccelStepper.h>
#include <ESP32Servo.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>

#define STEPS_TO_BIN_H 100
#define STEPS_TO_BIN_V -150

#define RETRIEVAL_POS_X 0
#define RETRIEVAL_POS_Y 5

namespace controls {

enum class Speed: uint8_t
{
    SLOW, MEDIUM, FAST
};

struct Pos2i {
    int x{0}, y{0};

    Pos2i(int x,int y) : x(x), y(y){};
    static double distance(const Pos2i pos1, const Pos2i pos2)
    {
        int delta_x = (pos1.x-pos2.x), delta_y = (pos1.y-pos2.y);
        return sqrt(delta_x*delta_x+delta_y*delta_y);
    }
    
};

inline Pos2i operator+(const Pos2i pos1,const Pos2i pos2)
{
    return {pos1.x+pos2.x,pos1.y+pos2.y};
};

class XYControl {

    static constexpr int ACCELERATION{200},MAX_SPEED{1000}, TIMEOUT {10000};
    static constexpr int STEP_PIN_X {25}, DIR_PIN_X {33}, END_X_PIN{34};
    static constexpr int STEP_PIN_Y {27}, DIR_PIN_Y {26}, END_Y_PIN{35};
    static constexpr int MAX_Y {500}, MAX_X {500};
    static int getSpeed(const Speed speed);
    static int getAcc(const Speed speed);

    AccelStepper motorX;
    AccelStepper motorY;

    public:
    XYControl();
    bool calibrate();
    bool moveTo(const Pos2i pos, const Speed speed = Speed::MEDIUM);
    Pos2i getPos() {return Pos2i{motorX.currentPosition(),motorY.currentPosition()};};
};

class ConveyorControl{
    static constexpr int ACCELERATION{200},MAX_SPEED{1000},TIMEOUT{3000};
    static constexpr int STEP_PIN_R {13}, DIR_PIN_R {14}, END_CONVEYOR_PIN{32};
    static constexpr int MAX_BIN {5};
    static constexpr int CONVEYOR_STEPS {90};
    static constexpr int BEGIN_STEP_OFFSET {5};

    int cur_pos {0};
    AccelStepper motorConveyor;
    public:
    ConveyorControl();
    bool start();
    bool next();
    int getPos();
};

class PlatformControl
{
    public:
        enum class Direction{EXTEND,RETRACT};
    private:
    static constexpr int TIMEOUT = 1000;
    
    static constexpr int SERVO_PIN {23}, END_PIN{36}; //SENSOR_VP EP1;
    static constexpr int MAX_SPEED = 90;
    static bool getSpeed(Direction dir, Speed speed);
    Direction pos {Direction::RETRACT};
    bool onEnd();
    Servo servo;
    public:

    PlatformControl();
    bool calibrate();
    bool move(Direction dir,Speed speed =Speed::MEDIUM);
    bool isExtended() {return pos == Direction::EXTEND;};
};

using rfid_t = std::array<uint8_t,12> ;
class MovementControl {
    private:
    static constexpr int STEPS_TO_UP_BIN {50};
    MFRC522DriverPinSimple pin{15};
    SPIClass &spiClass = SPI;
    const SPISettings spiSettings = SPISettings(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0); // May have to be set if hardware is not fully compatible to Arduino specifications.
    MFRC522DriverSPI driver{pin, spiClass, spiSettings}; // Create SPI driver.
    MFRC522 mfrc522{driver};
 // Create MFRC522 instance.
    public:
    XYControl xy_table;
    PlatformControl platform;
    //rfid thingy TODO
    void fetch();
    void store();
    bool read(rfid_t& rfid);
    bool readAndFetch(rfid_t& rfid);
    bool readWithoutExtending(rfid_t& rfid);
    void init();
};



}



/*
class StepMotor
{
private:
    int current_x_position;
    int current_y_position;
    int CONVEYOR_position;
    int end_x;
    int end_y;
    int end_CONVEYOR;
    AccelStepper motorCONVEYOR;
public:
    StepMotor();
    ~StepMotor();
    void begin();
    void goToBin(int x_position, int y_position);
    void goToRetrieval();
    void upBin();
    void downBin();
    void goToStartPosition();
    bool CONVEYORNext();
    bool CONVEYORStartPosition();

};
*/

#endif




