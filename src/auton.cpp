#include"main.h"

pros::Imu imu(18);
extern pros::ADIDigitalIn topLimit;
extern bool SORT_SYS_ENABLE;
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
    .withMotors(
        20,  // Top left
        11, // Top right (reversed)
        1, // Bottom right (reversed)
        9   // Bottom left
        
    )
        .withDimensions(AbstractMotor::gearset::green, {{4_in, 11.5_in}, imev5GreenTPR})
    .build();

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
static void swingTurn(const int32_t forwardPower, const int32_t turnPower, const uint32_t timeToRun, const uint32_t driveSettle)
{
    setDrive(forwardPower + turnPower, forwardPower - turnPower);
    pros::Task::delay(timeToRun);
    setDrive(0,0);
    pros::Task::delay(driveSettle);
}
enum loaderSetting
{
    Forward = 0,
    Backward = 1,
    Disabled = 2
};
static void setLoaders(const loaderSetting setting)
{
    if(setting == loaderSetting::Forward)
    {
        leftLoader.move(127);
        rightLoader.move(127);
    }
    else if(setting == loaderSetting::Backward)
    {
        leftLoader.move(-127);
        rightLoader.move(-127); 
    }
    else if(setting == loaderSetting::Disabled)
    {
        leftLoader.move(0);
        rightLoader.move(0);  
    }
    
}
static void redAuton()
{
    //swing into tower
    swingTurn(70, 30, 500, 500);

     //start lifts and sorting
    SORT_SYS_ENABLE = true;
    setLoaders(loaderSetting::Forward);

    //Perform loading/sorting procedure

    //swing out of tower to begin strafing
    swingTurn(-90, 35, 400, 750);
    setLoaders(loaderSetting::Disabled);

    //align for strafing.
    gyroTurn(90);

    //strafe right for next tower
    strafeAbstract(xModel, -200, 930, 500);

    //align at tower
    gyroTurn(90);

    //move towards tower
    chassis->moveDistance(0.5_ft);

    //wait until scored
    waitUntilShoot(500);

    chassis->moveDistance(-0.5_ft);

    //strafe to next tower
    strafeAbstract(xModel, -200, 930, 500);
    
    //swing into tower
    swingTurn(70, -30, 500, 500);

    setLoaders(loaderSetting::Forward);
}
void runAuton()
{
    //init gyro
    imu.reset();
    pros::delay(2000);

    redAuton();

}
