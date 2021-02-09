#include"sensors.hpp"

//Sensor init
pros::Optical vSensor(3);
pros::Distance distance_sensor(12);

//bool that holds the state of the limiter
extern bool canLimit;

//define the alliance color to sort the correct ball color.
#define RED

//tuning variables
static int32_t delayEject = 500;
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
		while(distance_sensor.get() > 100)
		{
			pros::delay(10);
			continue;
		}
		while(distance_sensor.get() < 60)
		{
			pros::delay(10);
			continue;
		}
		
		pros::lcd::print(4, "%d", limitPresses);
		limitPresses++;
		pros::delay(10);
	}
}

bool disableTop = false;
bool disableBottom = false;
extern bool runningAuton;
static bool seeBall()
{
	if(distance_sensor.get() < 50)
	{
		return true;
	}
	return false;
}
static bool canShoot()
{
	if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) && topSystem.get_actual_velocity() > minVelocity)
	{
		return true;
	}
	return false;
}
//this function will sort the balls based on the color signature passed in. 
//The task will start at the beginning of the program with the correct ball color to start.
void sort()
{
	//turn on optical LED
	vSensor.set_led_pwm(100);

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
			std::cout<<runningAuton<<std::endl;
			canLimit = true;
			mainSpeed = 80;
		}
		else if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) && !runningAuton)
		{
			canLimit = false;
			mainSpeed = 127;
		}
		
		if(seeBall() && canLimit)
		{
			rearSystem.move_velocity(0);
			topSystem.move_velocity(0);
			getToSpeed = true;
			pros::delay(10);
			continue;
		}

        /*255 returns if no objects of stated signature is found.*/

		//if the alliance color ball was found then just load up
		else
		if(vSensor.get_rgb().red > 1000)
		{
			std::cout<<"alliance"<<std::endl;
	
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
		//if the alliance ball is not detected then search for the enemy ball for discarding.
		else
		if(vSensor.get_rgb().blue > 1000)
		{
			std::cout<<"enemy"<<std::endl;
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