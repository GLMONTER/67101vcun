#include"main.h"

pros::Imu imu(21);

extern pros::ADIDigitalIn topLimit;

extern bool SORT_SYS_ENABLE;
extern bool canLimit;
extern unsigned int limitPresses;

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
    while(limitPresses < pressCount)
    {
        std::cout<<limitPresses<<std::endl;
        canLimit = false;
        if(!printed)
        {
            std::cout<<"waiting on limits"<<std::endl;
            printed = true;
        }

        continue;
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
    float Kd = -0.5;
    float Kp = -9.0;

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
        setDrive(-value, value);
        

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
        {0.003, 0, 0.0001}, // Turn controller gains
        {0.0025, 0.001, 0.0001})  // Angle controller gains (helps drive straight)

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

static void redAuton()
{
    waitUntilPressCount(2, true);
    setDrive(20, 20);
    pros::delay(400);
    setDrive(0,0);
    return;
    //start lifts and sorting
    SORT_SYS_ENABLE = true;
    setLoaders(loaderSetting::Forward);
    //swing into tower
    swingTurn(75, 30, 700, 1000, false);
    waitUntilPressCount(1, true);
    
    //Perform loading/sorting procedure

    //swing out of tower to begin strafing
    swingTurn(-90, 35, 700, 750, true);
    setLoaders(loaderSetting::Disabled);

    //align for strafing.
    gyroTurn(90);

    //strafe right for next tower
    strafeAbstract(xModel, -200, 930, 500);

    //align at tower
    gyroTurn(90);
    setDrive(50, 50);
    
    pros::delay(750);
    setDrive(0,0);

    setDrive(-50, -50);
        
    pros::delay(750);
    setDrive(0,0);

    gyroTurn(90);
    pros::delay(500);

    //strafe to next tower
    strafeAbstract(xModel, -200, 930, 500);

    //swing into tower
    swingTurn(80, 30, 750, 500, true);

    setLoaders(loaderSetting::Forward);
    
}
void runAuton()
{
    runningAuton = true;
    chassis->setMaxVelocity(130);
    
    //init gyro
    imu.reset();
    pros::delay(2000);

    redAuton();
    
   
 

}
