#include"main.h"

pros::Imu imu(21);
#define RED

//for checking presence of ball
extern pros::Distance distance_sensor;

extern bool redPassed;
extern bool bluePassed;

extern bool SORT_SYS_ENABLE;
extern bool canLimit;
extern unsigned int limitPresses;
extern bool disableTop;
extern bool disableBottom;
extern int32_t topVelocity;

//pre-defind loading settings for easier understanding
enum loaderSetting
{
    Forward = 0,
    Backward = 1,
    Disabled = 2
};

void init()
{
   
    //release hood by spinning Trio.
	topSystem.move(127);
	rearSystem.move(127);
    //intake loaders
	setLoaders(0);
	pros::Task::delay(1500);
	topSystem.move(0);
}

bool runningAuton = false;
static void strafeAb(int istrafeSpeed)
{
    leftFront.move(istrafeSpeed);
    leftBack.move(-istrafeSpeed);

    rightFront.move(-istrafeSpeed);
    rightBack.move(istrafeSpeed);
}
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
    float Ki = -0.03;
    float Kd = -0.6;
    float Kp = -4.5;

    while (std::abs(error) > 1 || leftBack.get_actual_velocity() > 0.1)
    {
        pros::lcd::print(0, "val: %f\n", imu.get_yaw());
        error =  target - imu.get_yaw();
      //  printf("%f \n", error);
        integral = integral + error;
        if (abs(error) < 1)
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
auto chassis =
  ChassisControllerBuilder()
    .withMotors(20, 6, 18, 10) // left motor is 1, right motor is 2 (reversed)
    // green gearset, 4 inch wheel diameter, 11.5 inch wheelbase
    .withDimensions(AbstractMotor::gearset::green, {{4_in, 9_in}, imev5GreenTPR})
    
    // left encoder in ADI ports A & B, right encoder in ADI ports C & D (reversed)
 .withSensors(
        ADIEncoder{'C', 'D', false}, // left encoder in ADI ports A & B
        ADIEncoder{'E', 'F', false} // right encoder in ADI ports C & D (reversed)
    )    // specify the tracking wheels diameter (2.75 in), track (7 in), and TPR (360)
    
.withOdometry({{2.75_in, 15_in}, quadEncoderTPR}, StateMode::FRAME_TRANSFORMATION)    
.buildOdometry();

auto xModel = std::dynamic_pointer_cast<XDriveModel>(chassis->getModel());

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

auto profileController =
  AsyncMotionProfileControllerBuilder()
    .withLimits({
      1.0, // Maximum linear velocity of the Chassis in m/s
      2.0, // Maximum linear acceleration of the Chassis in m/s/s
      10.0 // Maximum linear jerk of the Chassis in m/s/s/s
    })
    .withOutput(chassis)
    .buildMotionProfileController();
void home()
{    
   
    swingTurn(100, -20, 1000, 0, false);
    swingTurn(100, -30, 400, 0, false);

    waitUntilPressCount(2, false, 0);
    chassis->setMaxVelocity(150);
    chassis->driveToPoint({2_ft, 0.5_ft}, true);
    
    
    chassis->driveToPoint({4_ft, -3_ft}, true);
    
    waitUntilPressCount(5, false, 0);

}
void right()
{
    swingTurn(100, 20, 1000, 0, false);
    swingTurn(100, 30, 400, 0, false);

    waitUntilPressCount(2, false, 0);
    
    pros::delay(200);
    setDrive(-60, -60);
    pros::delay(500);
    SORT_SYS_ENABLE  =false;
    topSystem.move(127);
    setDrive(60, 60);
    pros::delay(600);
    setDrive(0, 0);

     pros::delay(1000);
    setLoaders(loaderSetting::Backward);

    setDrive(-60, -60);
    pros::delay(500);
    setDrive(0,0);

    chassis->setMaxVelocity(150);
    
    chassis->moveDistance(-1_ft);
}

void rightElim()
{
    swingTurn(100, 18, 1000, 0, false);
    swingTurn(100, 42, 400, 0, false);

    waitUntilPressCount(1, true, 0);
    chassis->setMaxVelocity(150);
    setDrive(-127, -127);
    pros::delay(500);
    setDrive(0, 0);
    gyroTurn(-45);

    strafeAb(50);
    pros::delay(1300);
    setDrive(0,0);

    strafeAb(-25);
    pros::delay(400);
    setDrive(0,0);

    setLoaders(loaderSetting::Backward);
    /*
    setDrive(100, 100);
    pros::delay(900);
    setDrive(50, 50);
    pros::delay(300);
    setDrive(0, 0);
    */
   chassis->setMaxVelocity(125);
   chassis->moveDistance(1.5_ft);
    gyroTurn(-36);

    canLimit = false;




}
void left()
{
    swingTurn(100, -20, 1000, 0, false);
    swingTurn(100, -32, 400, 0, false);

    waitUntilPressCount(2, false, 0);
    
    pros::delay(200);
    setDrive(-60, -60);
    pros::delay(500);
    SORT_SYS_ENABLE  =false;
    topSystem.move(127);
    setDrive(60, 60);
    pros::delay(500);
    setDrive(0, 0);

     pros::delay(1000);
    setLoaders(loaderSetting::Backward);

    setDrive(-60, -60);
    pros::delay(500);
    setDrive(0,0);

    chassis->setMaxVelocity(150);
    
    chassis->moveDistance(-1_ft);

}
void Skills()
{
    canLimit = true;
    
    profileController->generatePath({
    {0_ft, 0_ft, 0_deg},
  {4.2_ft, 0_ft, 0_deg}  // Profile starting position, this will normally be (0, 0, 0)
 }, // The next point in the profile, 3 feet forward
  "A" // Profile name
);
profileController->setTarget("A");

profileController->waitUntilSettled();
setLoaders(loaderSetting::Backward);
gyroTurn(62);
pros::delay(1000);
chassis->setMaxVelocity(75);
setDrive(75, 75);
pros::delay(2000);
canLimit = false;
pros::delay(500);
setDrive(-50, -50);
pros::delay(750);
setDrive(0,0);
gyroTurn(62);


canLimit = true;
setLoaders(loaderSetting::Forward);

strafeAb(-100);
pros::delay(950);
setDrive(0, 0);

chassis->setMaxVelocity(90);

profileController->generatePath({
    {0_ft, 0_ft, 0_deg},
  {10_ft, 1_ft, 0_deg}  // Profile starting position, this will normally be (0, 0, 0)
 }, // The next point in the profile, 3 feet forward
  "B" // Profile name
);
profileController->setTarget("B");
profileController->waitUntilSettled();
gyroTurn(28);
setDrive(80, 80);
pros::delay(3500);
canLimit = false;

pros::delay(5000);

}

//actually running the auton
void runAuton()
{
    runningAuton = true;
    init();
    setLoaders(loaderSetting::Forward);
    Skills();

    runningAuton = false;
}
