/*#include<main.h>
//distance from tracking center to center of left tracking wheel
float sSubL = 0.0f;

//distance from tracking center to center of right tracking wheel
float sSubR = 0.0f;

//distance from tracking center to rear back tracking wheel
float sSubS = 0.0f;


//TYPES FOR VECTOR ARE UNKNOWN AT THE MOMENT
//previous global position vector
auto dSubZero = 0.0f;

//previous global orientation
auto ZeroSubZero = 0.0f;

//global orientation at last reset
auto ZeroSubR = 0.0f;

double previousLeftEncoder = 0;
double previousRightEncoder = 0;

double deltaRight = 0.0;
double deltaLeft = 0.0;
double deltaStrafe = 0.0;

double leftWheelDiameter = 4.25;
double rightWheelDiameter = 4.25;
double middleWheelDiameter = 4.25;

pros::ADIEncoder leftEncoder(1, 2, false);
pros::ADIEncoder rightEncoder(1, 2, false);

void tracking()
{
    //get the new encoder distance
    double currentRight = rightEncoder.get_value();
    double currentLeft = leftEncoder.get_value();

    //change in encoder values since last tick
    deltaRight = std::abs(currentRight - previousRightEncoder);
    deltaLeft = std::abs(currentLeft - previousLeftEncoder);

    pros::Task::delay(10);
}
*/