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
	pros::Task::delay(1500);
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

//actually running the auton
void runAuton()
{
    canLimit = true;
    runningAuton = true;
    init();
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



    runningAuton = false;
}
