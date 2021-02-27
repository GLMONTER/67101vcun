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
    float Ki = -0.006;
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
        {0.0015, 0.0005, 0.0001}, // Distance controller gains
        {0.002, 0.00025, 0.0001}, // Turn controller gains
        {0.001, 0.0005, 0.0001})  // Angle controller gains (helps drive straight)
    // left encoder in ADI ports A & B, right encoder in ADI ports C & D (reversed)
 .withSensors(
        ADIEncoder{'C', 'D', false}, // left encoder in ADI ports A & B
        ADIEncoder{'E', 'F', false} // right encoder in ADI ports C & D (reversed)
    )    // specify the tracking wheels diameter (2.75 in), track (7 in), and TPR (360)
    
.withOdometry({{2.75_in, 15_in}, quadEncoderTPR}, StateMode::FRAME_TRANSFORMATION)    
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

std::shared_ptr<AsyncMotionProfileController> profileController =
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
    chassis->driveToPoint({0.5_ft, 1.75_ft}, true);
    chassis->turnToAngle(145_deg);
    chassis->setState({0_ft, 0_ft, 0_deg});
    chassis->driveToPoint({5.35_ft, -0.5_ft}, false);
    waitUntilPressCount(5, false, 0);

}
void left()
{
    swingTurn(100, 20, 1000, 0, false);
    swingTurn(100, 30, 400, 0, false);

    waitUntilPressCount(2, false, 0);
    chassis->setMaxVelocity(150);
    chassis->moveDistance(-1_ft);
}
void right()
{
    swingTurn(100, -20, 1000, 0, false);
    swingTurn(100, -30, 400, 0, false);

    waitUntilPressCount(2, false, 0);
    chassis->setMaxVelocity(150);
    chassis->moveDistance(-1_ft);

}
void Skills()
{
      chassis->setMaxVelocity(90);
    canLimit = true;
    chassis->driveToPoint({3_ft, 0_ft}, false);

    gyroTurn(-60);
    chassis->setMaxVelocity(125);

    chassis->driveToPoint({3.05_ft, -1.6_ft}, false);

    SORT_SYS_ENABLE = false;
    rearSystem.move(-127);
    pros::delay(250);
    SORT_SYS_ENABLE = true;
    canLimit = false;
    pros::delay(500);
    chassis->driveToPoint({2.25_ft, 0.9_ft}, true);
    canLimit = true;
    chassis->turnToAngle(-37_deg);
    chassis->driveToPoint({3.5_ft, -0.3_ft}, false);
   // gyroTurn(-37);

    chassis->driveToPoint({3.9_ft, -0.45_ft}, false);
    chassis->setState({0_in, 0_in, 0_deg});
    chassis->driveToPoint({-1.5_ft, 2.75_ft}, true);
    gyroTurn(-15);
    chassis->driveToPoint({-0.25_ft, 2.75_ft}, false);
    gyroTurn(-20);
    rearSystem.move(-127);
    pros::delay(250);
    SORT_SYS_ENABLE = true;
    canLimit = false;
  
    pros::delay(10000);

}

//actually running the auton
void runAuton()
{
    runningAuton = true;
    init();
    setLoaders(1);
    right();

    runningAuton = false;
}
