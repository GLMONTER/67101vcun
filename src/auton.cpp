#include"main.h"
pros::Imu imu(18);


void gyroTurn(float deg)
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


auto chassis = ChassisControllerBuilder()
    .withMotors(
        20,  // Top left
        11, // Top right (reversed)
        1, // Bottom right (reversed)
        9   // Bottom left
        
    )
        .withDimensions(AbstractMotor::gearset::green, {{4_in, 11.5_in}, imev5GreenTPR})
    .build();

  auto xModel = std::dynamic_pointer_cast<XDriveModel>(chassis->getModel());

  
void swingTurn(int32_t forwardPower, int32_t turnPower)
{
    setDrive(forwardPower + turnPower, forwardPower - turnPower);
}
    
void runAuton()
{
    imu.reset();
   pros::delay(2000);
    
    swingTurn(70, 30);
    
    pros::delay(500);
    setDrive(0,0);
    pros::delay(500);
    swingTurn(-90, 35);
    pros::delay(400);
    setDrive(0,0);
    pros::delay(750);
    gyroTurn(90);
    xModel->strafe(-200);
    pros::Task::delay(930);
    xModel->stop();
    pros::Task::delay(500);

    gyroTurn(90);

}
