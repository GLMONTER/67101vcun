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

extern void moveToPoint(const float x, const float y, const float angle);

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
	moveToPoint(-12.5, 52, -0.29);
    setLoaders(loaderSetting::Backward);
    moveToPoint(-14.77, 69.44, 0.838);
    moveToPoint(-11.87, 73.41, 0.856);
    moveToPoint(-14.77, 69.44, 0.838);
    moveToPoint(-11.87, 73.41, 0.856);
    moveToPoint(-14.77, 69.44, 0.838);
    moveToPoint(-11.87, 73.41, 0.856);
    moveToPoint(-14.77, 69.44, 0.838);
}
void homeRow()
{
    canLimit = true;
    setLoaders(loaderSetting::Forward);
    moveToPoint(0, 6, 0);

    moveToPoint(-15, 33, -0.745);
    moveToPoint(-33, 36, -1.57);

    waitUntilPressCount(1, false, 0);
    pros::delay(250);

    moveToPoint(15.54, 3.56, -0.737);
    
    canLimit = false;
   
    
    moveToPoint(40.5, -32.13, -3.068);
    //THIS ONE
    moveToPoint(39, -53.5, -2.979);
    if(limitPresses == 2)
        waitUntilPressCount(3, false, 0);
    else
        waitUntilPressCount(2, false, 0);

    pros::delay(750);

    //back out
    moveToPoint(35, -40, -2.979);
}
void skills()
{
    setLoaders(loaderSetting::Forward);
    canLimit = true;
    //get ball
    moveToPoint(-7.12, 16.5, -0.304);
    //go to front of tower
    moveToPoint(-27.97, 48.75, -2.308);
    //score
    moveToPoint(-34.5, 43, -2.25);
    canLimit = false;
    pros::delay(500);
    canLimit = true;
    moveToPoint(-27.97, 48.75, -2.308);

    //get next ball
    moveToPoint(-50, 69.63, -0.939);
    //go to text tower
    
    moveToPoint(-73.76, 87, -1.533);
    canLimit = false;
    pros::delay(1500);
    //back out
    moveToPoint(-67.29, 78.19, -1.466);
    canLimit = true;
    moveToPoint(-61.68, 89.17, -0.7269);
    moveToPoint(-29.59, 108.04, -0.701);
    moveToPoint(-36.12, 114.89, -0.702);
}
void left()
{
    canLimit = true;
    setLoaders(loaderSetting::Forward);
    moveToPoint(0, 6, 0);

    moveToPoint(-15, 33, -0.745);
    moveToPoint(-33, 36, -1.57);

    waitUntilPressCount(1, false, 0);
    pros::delay(250);

    moveToPoint(15.54, 3.56, -0.737);
}
//actually running the auton
void runAuton()
{
    canLimit = true;
    runningAuton = true;
    init();
    left();

    runningAuton = false;
}
