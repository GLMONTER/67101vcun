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

    moveToPoint(-15, 31, -0.745, true, 90);
    moveToPoint(-33, 35, -1.57, false, 90);

    waitUntilPressCount(1, false, 0);
    pros::delay(250);

    moveToPoint(15.54, 3.56, -0.737, true, 90);
    
    canLimit = false;
   
    
    moveToPoint(51, -36.13, -3, true, 90);
    canLimit = true;
    //THIS ONE
    moveToPoint(50.5, -42, -3, false, 80, 3000);
    canLimit = false;

    pros::delay(1000);
    canLimit = true;


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
    moveToPoint(-27.5, 44, -2.308, true, 60);
    //score MIDDLE
    moveToPoint(-35.75, 41, -2.25, false, 70, 3000);
    canLimit = false;
    pros::delay(600);
    canLimit = true;
    
    moveToPoint(-29.97, 48.75, -2.308, true, 60);
    //get rid of blue
    canLimit = false;
    pros::delay(800);
    canLimit = true;
    //get next ball
    moveToPoint(-58, 69, -0.97, false, 50, 4000);
    //go to text tower CORNER
    setLoaders(loaderSetting::Backward);
    moveToPoint(-80, 77, -1.47, false, 60, 3000);
    canLimit = false;
    pros::delay(600);
    //back out
    moveToPoint(-67.29, 78.19, -1.466, true, 90);
    setLoaders(loaderSetting::Forward);

    pros::delay(1000);
    canLimit = true;
    //go towards next ball
    moveToPoint(-61, 84, -0.7, false, 60, 3000);
    //get next ball
    moveToPoint(-67, 94, -0.7, false, 60, 3000);
    //go towards middle tower
    moveToPoint(-36, 108, -0.7, true, 60, 3000);
    setLoaders(loaderSetting::Backward);
    //go towards next tower. MIDDLE
    moveToPoint(-39, 113.75, -0.65, false, 60, 3000);
    

    canLimit = false;;
    pros::delay(800);
    canLimit = true;
    //backout
    moveToPoint(-22, 112, -0.65, true, 60, 3000);
    setLoaders(loaderSetting::Forward);
    
    //right towards ball
    moveToPoint(-18, 141, -0.60, false, 60, 3000);
    //get ball
    moveToPoint(-21, 144, -0.60, false, 60, 3000);
    //backout
     moveToPoint(-18, 138, -0.60, false, 60, 3000);
    //get near tower
    moveToPoint(-7, 157.28, 0.181, false, 60, 3000);
    moveToPoint(-8, 160, 0.18, false, 60, 3000);
    canLimit = false;
    pros::delay(1200);
    //go towards mid
    canLimit = true;
    //get towards ball
    moveToPoint(-14, 106, -2.4, false, 60);
    //get ball
    moveToPoint(-20, 100, -2.4, false, 60);
    pros::delay(1000);
    setLoaders(loaderSetting::Backward);
    //get inside middle
    moveToPoint(-11, 92, 2.35, false, 60, 4000);
    moveToPoint(-11, 92, 2.35, false, 60, 4000);

    
    canLimit = false;


    pros::delay(10000);


}
void left()
{
    canLimit = true;
    setLoaders(loaderSetting::Forward);
    moveToPoint(0, 6, 0, true);

    moveToPoint(-15, 33, -0.745, true, 110, 3000);
    moveToPoint(-33, 36, -1.57, false, 90, 3000);

    waitUntilPressCount(2, false, 0);
    pros::delay(700);
    setLoaders(loaderSetting::Backward);
    moveToPoint(-25, 30, -1.57, false, 100);

    //after backout
    setLoaders(loaderSetting::Disabled);
    moveToPoint(26.5, 35, 0.9, false, 70);
    moveToPoint(31, 29, 1.3, false, 40, 3000);
    moveToPoint(28, 29, 1.3, false, 40, 3000);

}
void right()
{
    canLimit = true;
    setLoaders(loaderSetting::Forward);
    moveToPoint(0, 6, 0, true);
    moveToPoint(15.5, 30, 1.54, true, 90);
    moveToPoint(31, 34, 1.54, false, 70, 2500);
     waitUntilPressCount(2, false, 0);
     canLimit = false;
    pros::delay(600);
        setLoaders(loaderSetting::Backward);

    moveToPoint(18, 37, 1.542, true, 100, 3000);
}
//actually running the auton
void runAuton()
{
    canLimit = true;
    runningAuton = true;
    init();
    //moveToPoint(0.0, 8.0, 0.0, false);
    //homeRow();
    //skills();
    left();
    runningAuton = false;
}
