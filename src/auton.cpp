#include"main.h"

pros::Imu imu(18);

extern pros::ADILineSensor topLimit;

extern bool SORT_SYS_ENABLE;

extern bool canLimit;
extern unsigned int limitPresses;
extern bool disableTop;
extern bool disableBottom;

extern int32_t topVelocity;
const extern int32_t normalLineValue;
const extern int32_t lowLineValue;

enum loaderSetting
{
    Forward = 0,
    Backward = 1,
    Disabled = 2
};
static void init()
{
    //release hood by spinning Trio and reverse loaders.
	topSystem.move(127);
	bottomSystem.move(127);
	setLoaders(1);
	pros::Task::delay(700);
	topSystem.move(0);
}

bool runningAuton = false;

//wait until a certain number of balls have gone through
static void waitUntilPressCount(const unsigned int pressCount, const bool waitUntilHold)
{
    static bool printed = false;

    std::cout<<"started wait"<<std::endl;
    std::cout<<pressCount<<std::endl;
    std::cout<<limitPresses<<std::endl;
    canLimit = false;
    
    while(limitPresses < pressCount - 1)
    {
        std::cout<< " Limit : " <<limitPresses<<std::endl;
        std::cout<< " PRESS : " <<pressCount<<std::endl;
        pros::delay(100);
       
        canLimit = false;
        if(!printed)
        {
            std::cout<<"waiting on limits"<<std::endl;
            printed = true;
        }
    }
    if(waitUntilHold)
    {
        while(topLimit.get_value() > lowLineValue)
        {
            pros::delay(10);
        }
    }
    canLimit = true;
}

//turn using gyro and PID
static void gyroTurn(const float deg)
{
	leftBack.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
    leftFront.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	rightBack.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	rightFront.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);

    float error = 10.0;
    float integral = 0.0;
    float derivative = 0.0;
    float perror = 0.0;
    float value = 0.0;

    float target = deg;
    float Ki = -0.0015;
    float Kd = -0.6;
    float Kp = -6.5;

    while (abs(error) > 1 || leftBack.get_actual_velocity() > 0.1)
    {
        pros::lcd::print(0, "val: %f\n", imu.get_yaw());
        error =  target - imu.get_yaw();
        printf("%f \n", error);
        integral = integral + error;
        if (abs(error) < 2)
        {
            integral = 0.0;
        }
        derivative = error - perror;
        perror = error;
        value = (integral*Ki) + (derivative*Kd) + (error*Kp);
        if(value > 0)
            setDrive(-value, value);
        else
            setDrive(value, -value);
        
        pros::delay(5);
    }
    setDrive(0,0);
}


static auto chassis = ChassisControllerBuilder()
    .withMotors
    (
        20,  // Top left
        11, // Top right (reversed)
        1, // Bottom right (reversed)
        9   // Bottom left
    )
    
    .withGains(
        {0.0025, 0.0005, 0.0001}, // Distance controller gains
        {0.0025, 0.0005, 0.0001}, // Turn controller gains
        {0.0025, 0.0005, 0.0001})  // Angle controller gains (helps drive straight)
        

    .withDimensions(AbstractMotor::gearset::green, {{4_in, 11.5_in}, imev5GreenTPR})
    .withOdometry()
    .buildOdometry();

auto xModel = std::dynamic_pointer_cast<XDriveModel>(chassis->getModel());

//function to see when the robot shoots a ball into the tower.
static void waitUntilShoot(const uint32_t timeAfterShoot)
{
    //wait until the limit switch is pressed
    while(!topLimit.get_value())
    {
        pros::delay(10);
    }
    //delay doing anything until the ball is completely out of the robot.
    pros::delay(timeAfterShoot);
}

static void strafeAbstract(std::shared_ptr<okapi::XDriveModel>& model, double velocityPower, const uint32_t timeToStrafe, const uint32_t timeToSettle)
{
    //use the chassis model to strafe for a certain amount of time, then stop.
    model->strafe(velocityPower);
    pros::delay(timeToStrafe);
    model->stop();
    pros::delay(timeToSettle);
}

static void swingTurn(const int32_t forwardPower, const int32_t turnPower, const uint32_t timeToRun, const uint32_t driveSettle, const bool settle)
{
    //set the drive based on input
    setDrive(forwardPower + turnPower, forwardPower - turnPower);
    //run drive for specified time
    pros::delay(timeToRun);
    //turn off drive
    setDrive(0,0);
    //settle
    if(settle)
        pros::delay(driveSettle);
}

static void twoRight()
{
    //set initial chassis velocity
    chassis->setMaxVelocity(130);
    
    //start lifts and sorting
    SORT_SYS_ENABLE = true;
    canLimit = false;
    setLoaders(loaderSetting::Forward);

    //swing into tower and load
    swingTurn(80, 20, 1650, 600, true);
    pros::Task::delay(700);
     
    //move out 
    chassis->moveDistance(-1.5_ft);
}
static void twoLeft()
{
    chassis->setMaxVelocity(130);

    //start lifts and sorting
    SORT_SYS_ENABLE = true;
    canLimit = false;
    setLoaders(loaderSetting::Forward);
  
    chassis->driveToPoint({2.45_ft, 0.25_ft});
  
    chassis->turnAngle(-108_deg);
    chassis->setMaxVelocity(65);
    chassis->moveDistance(1.2_ft);

    //swing into tower
    pros::delay(2000);
    chassis->setMaxVelocity(140);
    chassis->moveDistance(-1_ft);
}

static void newHomeRow()
{
    //set initial velocity
    chassis->setMaxVelocity(130);
    
    //start lifts and sorting
    SORT_SYS_ENABLE = true;
    canLimit = false;
    setLoaders(loaderSetting::Forward);

    //swing into tower and load
    swingTurn(80, 20, 1650, 600, true);
    pros::Task::delay(700);
     
    //back out and eject alliance ball
    chassis->moveDistance(-3.25_ft);
    setLoaders(loaderSetting::Forward);
    pros::delay(750);
    
    //turn towards last tower
    chassis->setMaxVelocity(130);
    chassis->turnAngle(250_deg);
    chassis->setMaxVelocity(125);
    chassis->moveDistance(2.75_ft);
    gyroTurn(-179);
    
    //go towards tower, load, and leave.
    chassis->setMaxVelocity(80);
    chassis->moveDistance(1.6_ft);
    pros::delay(1500);
    chassis->moveDistance(-2_ft);
}

//actually running the auton
void runAuton()
{
    
    init();
    setLoaders(0);
    runningAuton = true;
    waitUntilPressCount(2, true);
    strafeAbstract(xModel, -150, 1000, 0);
    runningAuton = false;
    
}
