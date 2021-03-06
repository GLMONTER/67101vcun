
/*
#include"main.h"
//for trig functions
#include<cmath>


#define WHEEL_DIAM 2.783
#define SPIN_TO (WHEEL_DIAM * PI / 360)
#define L_R_TRACKING_DISTANCE 7.5
#define M_TRACKING_DISTANCE 7.5

pros::ADIEncoder leftEncoder(3, 4, false);
pros::ADIEncoder rightEncoder(5, 6, false);
pros::ADIEncoder middleEncoder(1, 2, true);

struct Position
{
    float a;
    float x;
    float y;
    int32_t lastLeft;
    int32_t lastRight;
    int32_t lastMiddle;
};

Position globalPos;
float globalRotation;

void setDriveSpec(const int32_t leftFrontV,const int32_t leftBackV,const int32_t rightFrontV,const int32_t rightBackV)
{
    leftFront.move(leftFrontV);
    leftBack.move(leftBackV);
    rightFront.move(rightFrontV);
    rightBack.move(rightBackV);
}
void trackPosition()
{    
    int32_t leftEncoderValue = leftEncoder.get_value();
    int32_t rightEncoderValue = rightEncoder.get_value();
    int32_t middleEncoderValue = middleEncoder.get_value();

    float leftMovement = (leftEncoderValue - globalPos.lastLeft) * SPIN_TO;
    float rightMovement = (rightEncoderValue - globalPos.lastRight) * SPIN_TO;
    float middleMovement = (middleEncoderValue - globalPos.lastMiddle) * SPIN_TO;


    globalPos.lastLeft = leftEncoderValue;
    globalPos.lastRight = rightEncoderValue;
    globalPos.lastMiddle = middleEncoderValue;

    float hypotenuse;
    float halfOfAngleTraveled;
    float angleTraveledRear;
    float fullAngleTraveled = (leftMovement - rightMovement) / (L_R_TRACKING_DISTANCE * 2);

    if(fullAngleTraveled)
    {
        float r = rightMovement / fullAngleTraveled; //radious of the circle the robot travled around with the right side of robot
        halfOfAngleTraveled = fullAngleTraveled / 2.0;
        float sinHalfOfAngleTraveled = sin(halfOfAngleTraveled);
        hypotenuse = ((r + (L_R_TRACKING_DISTANCE / 2)) * sinHalfOfAngleTraveled) * 2.0;

        //same thing but with back of robot and not sides
        float r2 = middleMovement / fullAngleTraveled;
        angleTraveledRear = ((r2 + M_TRACKING_DISTANCE) * sinHalfOfAngleTraveled) * 2.0;
    }
    else
    {
        hypotenuse = rightMovement;
        halfOfAngleTraveled = 0;
        angleTraveledRear = middleMovement;
    }

    float endAngle = halfOfAngleTraveled + globalPos.a;
    float cosP = cos(endAngle);
    float sinP = sin(endAngle);


    //update global position
    globalPos.y += hypotenuse * cosP;
    globalPos.x += hypotenuse * sinP;

    globalPos.y += angleTraveledRear * -sinP;
    globalPos.x += angleTraveledRear * cosP;

    globalPos.a += fullAngleTraveled;
    pros::lcd::print(0, "x :  %f\n", globalPos.x);
    pros::lcd::print(1, "y :  %f\n", globalPos.y);

    pros::lcd::print(2, "left :  %d\n", leftEncoder.get_value());
    pros::lcd::print(3, "right :  %d\n", rightEncoder.get_value());
    pros::lcd::print(4, "middle :  %d\n", middleEncoder.get_value());
    pros::lcd::print(5, "imu :  %f\n", globalPos.a);

    pros::delay(10);
}
//PID function for the x axis
float getNewX(const float target)
{
    static float error;
    static float integral;
    static float derivative;
    static float previousError;
    static float driveValue;

    const float Ki = 0.005f;
    const float Kd = 0.6f;
    const float Kp = 8.5f;

    error = target - globalPos.x;
    integral = integral + error;
    if(abs(error) < 0.25f)
    {
        integral = 0.0f;
    }

    derivative = error - previousError;
    previousError = error;
    return (integral*Ki) + (derivative*Kd) + (error*Kp);
}

float getNewY(const float target)
{
    static float error;
    static float integral;
    static float derivative;
    static float previousError;
    static float driveValue;

    const float Ki = 0.005f;
    const float Kd = 0.6f;
    const float Kp = 8.5f;

    error = target - globalPos.y;
    integral = integral + error;
    if(abs(error) < 0.25f)
    {
        integral = 0.0f;
    }

    derivative = error - previousError;
    previousError = error;
    return (integral*Ki) + (derivative*Kd) + (error*Kp);
}

float getNewAngle(const float target)
{
    static float error;
    static float integral;
    static float derivative;
    static float previousError;
    static float driveValue;

    const float Ki = -0.005;
    const float Kd = -0.6f;
    const float Kp = -50.5f;
    //subject to change heading for yaw
    
    error = target - globalPos.a;
    integral = integral + error;
    if(abs(error) < 1.0f)
    {
        integral = 0.0f;
    }

    derivative = error - previousError;
    previousError = error;
    return (integral*Ki) + (derivative*Kd) + (error*Kp);
}
//calculating error between requested position and current position using pid and running the drive train
void moveToPoint(const float x, const float y, const float angle)
{
    pros::Task::delay(1000);
    std::cout<<"moving"<<std::endl;
   
   while((std::abs(globalPos.x - x) > 0.25) || (std::abs(globalPos.y - y) > 0.25) || (std::abs(globalPos.a - angle) > 0.25))
    {
        trackPosition();
        
        float tempY = getNewY(y);
        float tempX = getNewX(x);
        float tempAngle = -getNewAngle(angle);
        float angleDifference  = globalPos.a + angle;
        
        if(globalPos.a > 360)
            globalPos.a = remainder(globalPos.a, 360);
        int32_t frontLeftV = tan(45 - angleDifference);
        int32_t backLeftV = tan(-45 - angleDifference);
        int32_t frontRightV = tempY - tempX - tempAngle;
        
        
        int32_t backRightV = tempY + tempX - tempAngle;
        std::cout<<"x : "<<tempX<<" y : "<<tempY<<" A "<<tempAngle<<std::endl;
        std::cout<<"pos : "<<globalPos.x << " " << globalPos.y << " "<<globalPos.a<<std::endl<<std::endl;
        
       
        setDriveSpec(frontLeftV, backLeftV, frontRightV, backRightV);

        pros::delay(10);
    }
}
*/