#include"main.h"

pros::Imu imu(18);

extern pros::ADIDigitalIn topLimit;

extern bool SORT_SYS_ENABLE;
extern bool canLimit;
extern unsigned int limitPresses;
extern bool disableTop;
extern bool disableBottom;
extern int32_t topVelocity;
#define BEN
enum loaderSetting
{
    Forward = 0,
    Backward = 1,
    Disabled = 2
};
bool runningAuton = false;
//wait until a certain number of balls have gone through
static void waitUntilPressCount(const unsigned int pressCount, const bool waitUntilHold)
{
    static bool printed = false;

    std::cout<<"started wait"<<std::endl;
    std::cout<<pressCount<<std::endl;
    std::cout<<limitPresses<<std::endl;
    canLimit = false;
    while(limitPresses < pressCount)
    {
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
        std::cout<<"starting wait until hold"<<std::endl;
        while(topLimit.get_value())
        {
            continue;
        }
        while(!topLimit.get_value())
        {
            continue;
        }
        std::cout<<"finished"<<std::endl;
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
    /*
    .withGains(
        {0.0025, 0.0005, 0.0001}, // Distance controller gains
        {0.0025, 0.0005, 0.0001}, // Turn controller gains
        {0.0025, 0.0005, 0.0001})  // Angle controller gains (helps drive straight)
        */

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
    model->strafe(velocityPower);
    pros::delay(timeToStrafe);
    model->stop();
    pros::delay(timeToSettle);
}
static void swingTurn(const int32_t forwardPower, const int32_t turnPower, const uint32_t timeToRun, const uint32_t driveSettle, const bool settle)
{
    setDrive(forwardPower + turnPower, forwardPower - turnPower);
    pros::Task::delay(timeToRun);
    setDrive(0,0);
    if(settle)
        pros::Task::delay(driveSettle);
}
static void newhomwRow()
{
    chassis->setMaxVelocity(130);
    
    //start lifts and sorting
    SORT_SYS_ENABLE = true;
    canLimit = false;
    setLoaders(loaderSetting::Forward);

    swingTurn(80, 20, 1650, 600, true);
    pros::Task::delay(700);
     
    chassis->moveDistance(-3.25_ft);
    setLoaders(loaderSetting::Forward);
    pros::delay(750);
   
    //Perform loading/sorting procedure
    
    chassis->setMaxVelocity(130);
    chassis->turnAngle(250_deg);
    chassis->setMaxVelocity(125);
    chassis->moveDistance(2.75_ft);
    gyroTurn(-179);
    /*
    while(true)
    {
        pros::lcd::print(0, "%f", imu.get_yaw());
        pros::delay(10);
    }
    */
    pros::lcd::print(0, "%f", imu.get_yaw());
    //chassis->turnAngle(-50_deg);
    chassis->setMaxVelocity(80);
    chassis->moveDistance(1.6_ft);
    pros::delay(2000);
    chassis->moveDistance(-2_ft);
}
static void homeRow()
{
    canLimit = true;
        //start lifts and sorting
    SORT_SYS_ENABLE = true;
    
    setLoaders(loaderSetting::Forward);
    //swing into tower
    swingTurn(75, 33, 600, 1000, false);
    waitUntilPressCount(1, true);
    
 
   
    //swing out of tower to begin strafing
    swingTurn(-90, 35, 600, 750, true);
    //setLoaders(loaderSetting::Backward);
   
    //align for strafing.
    gyroTurn(90);
    //topVelocity = 500;
   

    //strafe right for next tower
    strafeAbstract(xModel, -200, 1050, 400);
    topVelocity = 600;
    //align at tower
    gyroTurn(90);
    //setLoaders(loaderSetting::Forward);
   
    chassis->setMaxVelocity(130);
    chassis->moveDistance(0.9_ft);
  
    canLimit = false;
    waitUntilPressCount(3, false);
    //setLoaders(loaderSetting::Disabled);
   
    canLimit = true;
    setLoaders(loaderSetting::Disabled);
    

    chassis->moveDistance(-0.75_ft);
    topVelocity = 410;
        
  
    gyroTurn(90);
    pros::delay(500);

    //strafe to next tower
    strafeAbstract(xModel, -200, 1400, 500);

    //swing into tower
     setLoaders(loaderSetting::Forward);
    swingTurn(83, 28, 750, 500, true);

   gyroTurn(135);
    canLimit = false;
}
static void rightTwo()
{
    #ifdef CAL
    chassis->setMaxVelocity(130);
    SORT_SYS_ENABLE = false;
    topSystem.move(127);
    pros::delay(500);
    topSystem.move(0);
    //start lifts and sorting
    SORT_SYS_ENABLE = true;
    canLimit = false;
    setLoaders(loaderSetting::Forward);
  
    chassis->driveToPoint({2.275_ft, 0.25_ft});
    //pros::delay(5000);
    gyroTurn(70);
      chassis->setMaxVelocity(65);
    chassis->moveDistance(0.95_ft);

    //swing into tower
    
    waitUntilPressCount(1, true);
    setLoaders(loaderSetting::Backward);
    //Perform loading/sorting procedure
    disableTop = true;
    topSystem.move(0);

    chassis->setMaxVelocity(140);
    chassis->moveDistance(-0.5_ft);
    setLoaders(loaderSetting::Forward);
    chassis->driveToPoint({2.5_ft, -1.75_ft});
    chassis->turnAngle(45_deg);
    chassis->moveDistance(0.5_ft);
    waitUntilPressCount(3, true);
    #endif

    #ifdef BEN
chassis->setMaxVelocity(130);
    SORT_SYS_ENABLE = false;
    topSystem.move(127);
    pros::delay(500);
    topSystem.move(0);
    //start lifts and sorting
    SORT_SYS_ENABLE = true;
    canLimit = false;
    setLoaders(loaderSetting::Forward);
  
    chassis->driveToPoint({2.1_ft, 0.25_ft});
    //pros::delay(5000);
    gyroTurn(70);
      chassis->setMaxVelocity(65);
    chassis->moveDistance(1.75_ft);

    //swing into tower
    
    waitUntilPressCount(1, true);
    setLoaders(loaderSetting::Backward);
    //Perform loading/sorting procedure
    disableTop = true;
    topSystem.move(0);

    chassis->setMaxVelocity(140);
    chassis->moveDistance(-0.5_ft);
    setLoaders(loaderSetting::Forward);
    chassis->driveToPoint({2.5_ft, -1.75_ft});
    chassis->turnAngle(45_deg);
    chassis->moveDistance(0.5_ft);
    waitUntilPressCount(3, true);
    #endif
}

void runAuton()
{
    runningAuton = true;
    
   canLimit = true;
    

    newhomwRow();
    
   
 
runningAuton = false;
}
