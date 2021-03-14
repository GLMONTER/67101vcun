#include"sensors.hpp"

//Sensor init
pros::Optical vSensor(3);
pros::Distance distance_sensor(12);

//bool that holds the state of the limiter
extern bool canLimit;

//define the alliance color to sort the correct ball color.
#define BLUE
bool redPassed = false;
bool bluePassed = false;
//tuning variables
static int32_t delayEject = 200;
static int32_t mainSpeed = 127;

static int32_t minVelocity = 450;

//make sure the top system can get to speed before shooting occurs
bool getToSpeed = false;
//enable/disable sorting task
bool SORT_SYS_ENABLE = true;

unsigned int limitPresses = 0;

//polls limit switch to check for when a ball passes through.
void pollSensors()
{
	while(true)
	{
		//wait for ball to come into view
		while(distance_sensor.get() > 100)
		{
			pros::delay(10);
			continue;
		}
		//give time to shoot if needed
		pros::delay(100);
		//wait for ball to leave view
		while(distance_sensor.get() < 100)
		{
			pros::delay(10);
			continue;
		}
		//add to the amount of balls that passed through
		limitPresses++;
		//print new value
		pros::lcd::print(4, "%d", limitPresses);
	}
}
//flags to disable certain drums.
bool disableTop = false;
bool disableBottom = false;
//extern flag to check if auton is running. 
extern bool runningAuton;

//simply checking if ball in view
static bool seeBall()
{
	if(distance_sensor.get() < 50)
	{
		return true;
	}
	return false;
}

//a multithreaded task that starts in init() that controls the drums based on optical sensor input
void sort()
{
	//turn on optical LED
	vSensor.set_led_pwm(100);

	//have to initially
	rearSystem.move(mainSpeed);
	topSystem.move(mainSpeed);

	//set loader brake modes to lock so the alliance ball can stop at the top of the loader
	topSystem.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	rearSystem.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	
	while(true)
	{
		//printing for debugging
		pros::lcd::print(1, "%f", vSensor.get_rgb().red);
		pros::lcd::print(2, "%f", vSensor.get_rgb().blue);
		
        //if the sorting system is disabled then don't attemp to sort.
        if(!SORT_SYS_ENABLE)
            continue;

		if(!controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) && !runningAuton)
		{
			canLimit = true;
			mainSpeed = 85;
		}
		else if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) && !runningAuton)
		{
			canLimit = false;
			mainSpeed = 127;
		}
		if(runningAuton && canLimit)
		{
			mainSpeed = 85;
		}
		if(runningAuton && !canLimit)
		{
			mainSpeed = 127;
		}
		//hold the ball in position if not wanting to shoot
		if(seeBall() && canLimit)
		{
			rearSystem.move_velocity(0);
			topSystem.move_velocity(0);
			getToSpeed = true;
			pros::delay(10);
			continue;
		}

        //check if a red ball is found and respond accordingly based on alliance color
		else
		if((vSensor.get_rgb().red / vSensor.get_rgb().blue) > 1.75)
		{	
			redPassed = true;
			if(!canLimit && runningAuton)
			{
				topSystem.move(-mainSpeed);
			}
			#ifdef RED
			rearSystem.move(-mainSpeed);
			#else
			rearSystem.move(mainSpeed);
			#endif
			
			#ifdef RED
			if(getToSpeed)
			{
				pros::delay(250);
				topSystem.move(-mainSpeed);
				getToSpeed = false;
			}
			else
			{
				topSystem.move(-mainSpeed);
			}
			#else
			topSystem.move(-mainSpeed);
			pros::delay(delayEject);
			#endif
		}
        //check if a blue ball is found and respond accordingly based on alliance color
		else
		if((vSensor.get_rgb().blue / vSensor.get_rgb().red) > 1.75)
		{
			bluePassed = true;
			#ifdef RED
			if(!disableBottom)
				rearSystem.move(mainSpeed);
			#else
			if(!disableBottom)
				rearSystem.move(-mainSpeed);
			#endif
			#ifdef BLUE
			if(getToSpeed)
			{
				pros::delay(250);
				topSystem.move(-mainSpeed);
				getToSpeed = false;
			}
			else
			{
				topSystem.move(-mainSpeed);
			}
			#else
			topSystem.move(-mainSpeed);
			pros::delay(delayEject);
			#endif
			
		}
		//if nothing was found then just load like normal
		else
		{
			topSystem.move(-mainSpeed);
			rearSystem.move(-mainSpeed);
		}
		//make the thread sleep to prevent other threads from being starved of resources.
		pros::Task::delay(10);
	}
}