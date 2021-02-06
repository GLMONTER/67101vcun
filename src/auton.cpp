#include"main.h"


pros::Imu imu(21);

extern pros::Distance distance_sensor;
extern pros::ADIDigitalIn topLimit;

extern bool SORT_SYS_ENABLE;
extern bool canLimit;
extern unsigned int limitPresses;
extern bool disableTop;
extern bool disableBottom;
extern int32_t topVelocity;

enum loaderSetting
{
    Forward = 0,
    Backward = 1,
    Disabled = 2
};
static void init()
{
   
    //release hood by spinning Trio.
	topSystem.move(127);
	rearSystem.move(127);
	setLoaders(0);
	pros::Task::delay(1500);
	topSystem.move(0);
   
	//reverse loaders for deplyoment
}

bool runningAuton = false;

//wait until a certain number of balls have gone through
static void waitUntilPressCount(const unsigned int pressCount, const bool waitUntilHold, const unsigned int delayUntilHold)
{
    static bool printed = false;

    std::cout<<"started wait"<<std::endl;
    std::cout<<pressCount<<std::endl;
    std::cout<<limitPresses<<std::endl;
    canLimit = false;
    
    while(limitPresses < pressCount)
    {
        pros::lcd::print(3, "%d", limitPresses);
        std::cout<< " Limit : " <<limitPresses<<std::endl;
        std::cout<< " PRESS : " <<pressCount<<std::endl;
        pros::delay(100);
        //std::cout<<limitPresses<<std::endl;
        canLimit = false;
        if(!printed)
        {
            std::cout<<"waiting on limits"<<std::endl;
            printed = true;
        }
    }
    if(waitUntilHold)
    {
        pros::delay(250);
        std::cout<<"starting wait until hold"<<std::endl;
        while(distance_sensor.get() > 50)
        {
            continue;
        }
        std::cout<<"finished"<<std::endl;
    }
    pros::delay(delayUntilHold);
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
    float Kp = -4.5;

    while (abs(error) > 1 || leftBack.get_actual_velocity() > 0.1)
    {
        pros::lcd::print(0, "val: %f\n", imu.get_yaw());
        error =  target - imu.get_yaw();
      //  printf("%f \n", error);
        integral = integral + error;
        if (abs(error) < 2)
        {
            integral = 0.0;
        }
        derivative = error - perror;
        perror = error;
        value = (integral*Ki) + (derivative*Kd) + (error*Kp);
        setDrive(-value, value);
        
        pros::delay(5);
    }
    setDrive(0,0);
}
std::shared_ptr<OdomChassisController> chassis =
  ChassisControllerBuilder()
    .withMotors(20, 6, 18, 10) // left motor is 1, right motor is 2 (reversed)
    // green gearset, 4 inch wheel diameter, 11.5 inch wheelbase
    .withDimensions(AbstractMotor::gearset::green, {{4_in, 9_in}, imev5GreenTPR})
    .withGains(
        {0.0018, 0.0005, 0.0001}, // Distance controller gains
        {0.0025, 0.0005, 0.0001}, // Turn controller gains
        {0.001, 0.0005, 0.0001})  // Angle controller gains (helps drive straight)
    // left encoder in ADI ports A & B, right encoder in ADI ports C & D (reversed)
 .withSensors(
        ADIEncoder{'C', 'D', false}, // left encoder in ADI ports A & B
        ADIEncoder{'E', 'F', false}, // right encoder in ADI ports C & D (reversed)
        ADIEncoder{'A', 'B', false}  // middle encoder in ADI ports E & F
    )    // specify the tracking wheels diameter (2.75 in), track (7 in), and TPR (360)
        .withOdometry({{2.783_in, 15.25_in, 7.5_in, 2.783_in}, quadEncoderTPR}, StateMode::FRAME_TRANSFORMATION)
    .buildOdometry();

/*
static auto chassis = ChassisControllerBuilder()
    .withMotors(20, 6, 18, 10) // left motor is 1, right motor is 2 (reversed)
    // green gearset, 4 inch wheel diameter, 11.5 inch wheelbase
    .withDimensions(AbstractMotor::gearset::green, {{4_in, 16_in}, imev5GreenTPR})
     .withGains(
        {0.002, 0.0005, 0.0001}, // Distance controller gains
        {0.0025, 0.0005, 0.0001}, // Turn controller gains
        {0.0012, 0.0005, 0.0001})  // Angle controller gains (helps drive straight)
    .withSensors(
        ADIEncoder{'C', 'D', false}, // left encoder in ADI ports A & B
        ADIEncoder{'E', 'F', false},  // right encoder in ADI ports C & D (reversed)
        ADIEncoder{'A', 'B', true}  // middle encoder in ADI ports E & F
    )
    // specify the tracking wheels diameter (2.75 in), track (7 in), and TPR (360)
    // specify the middle encoder distance (1 in) and diameter (2.75 in)
    .withOdometry({{2.75_in, 15.25_in, 6.5_in, 2.75_in}, quadEncoderTPR})
    .buildOdometry();
*/
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
static void threeRightNew()
{
      topSystem.move_velocity(topVelocity);
    //set initial chassis velocity
    chassis->setMaxVelocity(120);
    
    //start lifts and sorting
    SORT_SYS_ENABLE = true;
    
    setLoaders(loaderSetting::Forward);

    //swing into tower and load
   swingTurn(80, 21, 1670, 0, true);
   
   //chassis->moveDistance(2.05_ft);
  // gyroTurn(72);
  // chassis->setMaxVelocity(100);
  // chassis->moveDistance(1.75_ft);
   
    waitUntilPressCount(1, true, 0);

     setLoaders(1);
    //move out 
    chassis->moveDistance(-0.8_ft);
    setLoaders(2);
    gyroTurn(-60);
    chassis->moveDistance(1_ft);
    strafeAbstract(xModel, -100, 1400, 0);
    setLoaders(0);
    chassis->moveDistance(1.35_ft);
    strafeAbstract(xModel, 100, 600, 0);
    
    gyroTurn(-20);
    chassis->setMaxVelocity(80);
    chassis->moveDistance(1_ft);
    gyroTurn(-5);
    waitUntilPressCount(3, true, 0);
    chassis->moveDistance(-1_ft);
    
}
static void threeRight()
{
    topSystem.move_velocity(topVelocity);
    //set initial chassis velocity
    chassis->setMaxVelocity(130);
    
    //start lifts and sorting
    SORT_SYS_ENABLE = true;
    
    setLoaders(loaderSetting::Forward);

    //swing into tower and load
   //swingTurn(80, 21, 1670, 0, true);
   chassis->moveDistance(1.8_ft);
   gyroTurn(66);
   chassis->setMaxVelocity(110);
   chassis->moveDistance(1.8_ft);
    waitUntilPressCount(1, true,0);

     
    //move out 
    chassis->moveDistance(-0.8_ft);
    setLoaders(2);
    gyroTurn(-40);
    chassis->setMaxVelocity(100);
    setLoaders(0);
    chassis->moveDistance(2_ft);
    gyroTurn(-35);
    chassis->moveDistance(0.75_ft);
    waitUntilPressCount(3, false, 0);
    chassis->moveDistance(-1_ft);

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
std::shared_ptr<AsyncMotionProfileController> profileController =
  AsyncMotionProfileControllerBuilder()
    .withLimits({
      1.0, // Maximum linear velocity of the Chassis in m/s
      2.0, // Maximum linear acceleration of the Chassis in m/s/s
      10.0 // Maximum linear jerk of the Chassis in m/s/s/s
    })
    .withOutput(chassis)
    .buildMotionProfileController();
void right()
{    
    chassis->setMaxVelocity(150);
   //chassis->moveDistance(1.6_ft);
    chassis->driveToPoint({1.55_ft, 0_ft});
    chassis->turnToAngle(65_deg);
    SORT_SYS_ENABLE = true;
    chassis->setMaxVelocity(100);
    chassis->moveDistance(0.84_ft);
    waitUntilPressCount(2, false, 750);
    

    chassis->moveDistance(-0.75_ft);
    //chassis->driveToPoint({2_ft, 0_ft}, true);
    
}

void left()
{    
    chassis->setMaxVelocity(150);
    chassis->driveToPoint({1.6_ft, 0.2_ft});
    chassis->turnToAngle(-65_deg);
    SORT_SYS_ENABLE = true;
    chassis->setMaxVelocity(120);
    chassis->moveDistance(0.8_ft);
    waitUntilPressCount(2, false, 750);
    
    chassis->moveDistance(-0.75_ft);
    
}

//actually running the auton
void runAuton()
{
    runningAuton = true;
    init();
    
    setLoaders(1);
    left();
    /*
    gyroTurn(-35);
    pros::delay(1000);
    gyroTurn(55);
*/
    runningAuton = false;
}
