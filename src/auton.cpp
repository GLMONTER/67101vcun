#include"main.h"

//the color to sort out.
#define RED

//for checking presence of ball
extern pros::Distance distance_sensor;

//used for advanced sorting autons.
extern bool redPassed;
extern bool bluePassed;

extern bool SORT_SYS_ENABLE;
extern bool canLimit;
extern unsigned int limitPresses;

extern bool disableTop;
extern bool disableBottom;

extern int32_t topVelocity;

extern void moveToPoint(const float x, const float y, const float angle, bool goThroughFlag, const uint32_t maxVelocity = 127, uint32_t timeout = 0);

void init()
{
    //release hood by spinning Trio.
	topSystem.move(127);
	rearSystem.move(127);
    //intake loaders
	setLoaders(loaderSetting::Backward);
	pros::Task::delay(750);
	topSystem.move(0);
}

bool runningAuton = false;

//wait until a certain number of balls have gone through
static void waitUntilPressCount(const unsigned int pressCount, const bool waitUntilHold, const unsigned int delayUntilHold)
{
    static bool printed = false;
    canLimit = false;
    
    while(limitPresses < pressCount)
    {
        pros::delay(100);
        canLimit = false;
        if(!printed)
            printed = true;
    }
    if(waitUntilHold)
    {
        pros::delay(250);
        while(distance_sensor.get() > 50)
        {
            continue;
        }
    }
    pros::delay(delayUntilHold);
    canLimit = true;
}
void demo()
{
    setLoaders(loaderSetting::Forward);
	moveToPoint(-12.5, 52, -0.29, true);
    setLoaders(loaderSetting::Backward);
    moveToPoint(-14.77, 69.44, 0.838, true);
    moveToPoint(-11.87, 73.41, 0.856, true);
    moveToPoint(-14.77, 69.44, 0.838, true);
    moveToPoint(-11.87, 73.41, 0.856, true);
    moveToPoint(-14.77, 69.44, 0.838, true);
    moveToPoint(-11.87, 73.41, 0.856, true);
    moveToPoint(-14.77, 69.44, 0.838, true);
}
void homeRow()
{
    canLimit = true;
    setLoaders(loaderSetting::Forward);
    moveToPoint(0, 6, 0, true);

    moveToPoint(-15, 31, -0.745, true, 150);
    moveToPoint(-34, 36, -1.57, false, 100);

    waitUntilPressCount(1, false, 0);
    pros::delay(250);

    moveToPoint(15.54, 3.56, -0.737, true, 130);
    
    canLimit = false;
   
    
    moveToPoint(37.5, -38.13, -3.068, true, 110);
    //THIS ONE
    moveToPoint(38, -52.5, -2.979, false, 80);
    if(limitPresses == 2)
        waitUntilPressCount(3, false, 0);
    else
        waitUntilPressCount(2, false, 0);

    pros::delay(750);

    //back out
    moveToPoint(35, -40, -2.979, true);
}
void skills()
{
    setLoaders(loaderSetting::Forward);
    canLimit = true;
    //get ball
    moveToPoint(-7.12, 16.5, -0.304, true, 50);
    //go to front of tower
    moveToPoint(-28.5, 47, -2.308, true, 60);
    //score MIDDLE
    moveToPoint(-36.75, 44, -2.25, false, 70, 3000);
    canLimit = false;
    pros::delay(1000);
    canLimit = true;
    
    moveToPoint(-27.97, 48.75, -2.308, true, 80);

    //get next ball
    moveToPoint(-51, 71, -0.89, false, 50, 4000);
    //go to text tower CORNER
    
    moveToPoint(-71, 86, -1.47, false, 80, 4000);
    canLimit = false;
    pros::delay(1250);
    //back out
    moveToPoint(-67.29, 78.19, -1.466, true, 90);
    pros::delay(1000);
    canLimit = true;
    //go towards next ball
    moveToPoint(-47, 90, -0.7, false, 70, 4000);
    //get next ball
    moveToPoint(-55, 100, -0.7, false, 80, 4000);

    moveToPoint(-22, 112, -0.7, true, 80, 3000);
    //go towards next tower. MIDDLE
    moveToPoint(-22.5, 115.75, -0.65, false, 80, 3000);
    canLimit = false;;
    pros::delay(1200);
    canLimit = true;
    //backout
    moveToPoint(-22, 112, -0.65, true, 80, 3000);

    
    //right towards ball
    moveToPoint(9, 132, -0.60, false, 80, 3000);
    //get ball
    moveToPoint(0, 137, -0.60, false, 80, 3000);
    //get near tower
    moveToPoint(26.17, 140.28, 0.181, false, 80);
    moveToPoint(25.17, 146, 0.15, false, 70);
    canLimit = false;
    pros::delay(1200);
    moveToPoint(5.17, 138.28, 0.181, false, 80);



}
void left()
{
    canLimit = true;
    setLoaders(loaderSetting::Forward);
    moveToPoint(0, 6, 0, true);

    moveToPoint(-15, 33, -0.745, true, 110);
    moveToPoint(-33, 36, -1.57, false, 90);

    waitUntilPressCount(1, false, 0);
    pros::delay(250);

    moveToPoint(-25, 30, -1.57, false, 100);
}
void right()
{
    canLimit = true;
    setLoaders(loaderSetting::Forward);
    moveToPoint(0, 6, 0, true);
    moveToPoint(15.5, 30, 1.542, true, 100);
    moveToPoint(32, 35.5, 1.5, false, 70);
     waitUntilPressCount(1, false, 0);
     setLoaders(loaderSetting::Disabled);
     canLimit = false;
    pros::delay(2000);
    moveToPoint(17.5, 36.42, 1.542, true, 110);
}
//actually running the auton
void runAuton()
{
    canLimit = true;
    runningAuton = true;
    init();
    //moveToPoint(0.0, 8.0, 0.0, true);
    skills();


    runningAuton = false;
}
