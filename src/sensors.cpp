#include"sensors.hpp"

//define sig indexes
#define ALLIANCE_SIG 1
#define ENEMEY_SIG 2

//signatures generated using the vision utility	
pros::vision_signature_s_t BLUE_SIG = pros::Vision::signature_from_utility(1, -3719, -3175, -3446, 10543, 14845, 12694, 2.700, 0);
pros::vision_signature_s_t RED_SIG = pros::Vision::signature_from_utility(2, 7969, 10049, 9010, -447, 815, 184, 2.500, 0);

//Sensor init
pros::Vision vSensor(1, pros::E_VISION_ZERO_CENTER);
pros::ADIDigitalIn topLimit(8);
pros::Distance distance_sensor(12);

//bool that holds the state of the limiter
extern bool canLimit;

//define the alliance color to sort the correct ball color.
#define BLUE

//tuning variables
static int32_t delayEject = 500;
static int32_t mainSpeed = 127;

int32_t topVelocity = 600;
static int32_t minVelocity = 450;

//enable/disable sorting task
bool SORT_SYS_ENABLE = true;

unsigned int limitPresses = 0;

//polls limit switch to check for when a ball passes through.
void pollSensors()
{
	while(true)
	{
		while(distance_sensor.get() > 15)
		{
			continue;
		}
		while(distance_sensor.get() < 25)
		{
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
void sort(void* sigPass)
{
	rearSystem.move(mainSpeed);
	topSystem.move(mainSpeed);
    pros::vision_signature_s_t sig =  *reinterpret_cast<pros::vision_signature_s_t*>(sigPass);
	//resetting vision sensor LED color.
	vSensor.clear_led();

	//set the red and blue signatures to be referenced later.
	vSensor.set_signature(ALLIANCE_SIG, &sig);
	#ifdef RED
	vSensor.set_signature(ENEMEY_SIG, &RED_SIG);
	#else
	vSensor.set_signature(ENEMEY_SIG, &BLUE_SIG);
	#endif

	//set loader brake modes to lock so the alliance ball can stop at the top of the loader
	topSystem.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	rearSystem.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	
	while(true)
	{
        //if the sorting system is disabled then don't attemp to sort.
        if(!SORT_SYS_ENABLE)
            continue;
	
		//get the largest object(0), based on the signature passed in.
		pros::vision_object_s_t First_rtn = vSensor.get_by_sig(0, ALLIANCE_SIG);
		pros::vision_object_s_t Second_rtn = vSensor.get_by_sig(0, ENEMEY_SIG);
		if(!controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) && !runningAuton)
		{
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
				static int i = 0;
				i++;
				if(i == 1000)
				{
					i = 0;
				}
				rearSystem.move_velocity(0);
				topSystem.move_velocity(0);
					
				
				continue;
		}

        /*255 returns if no objects of stated signature is found.*/

		//if both sigs are found then sort based on color and positioning
		if(Second_rtn.signature != 255 && First_rtn.signature != 255 && First_rtn.width > 15 && Second_rtn.width > 15)
		{
			if(!canLimit && runningAuton)
			{
				topSystem.move(mainSpeed);
			}
			std::cout<<"both"<<std::endl;
			if(!controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) && !runningAuton)
			{
				//topSystem.move(0);
			}
			if(First_rtn.y_middle_coord > Second_rtn.y_middle_coord)
			{
				vSensor.set_led(COLOR_GREEN);
				if(!disableTop && controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2))
					topSystem.move(-mainSpeed);
				if(!disableBottom)
					rearSystem.move(mainSpeed);
			}
			else
			{
				vSensor.set_led(COLOR_GREEN);
				if(!disableTop && controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2))
					topSystem.move(mainSpeed);
				if(!disableBottom)
				{
					if(topSystem.get_actual_velocity() > minVelocity)
					{
						rearSystem.move(mainSpeed);
					}
				}
			}
		}
		//if the alliance color ball was found the just load up
		else
		if(First_rtn.signature != 255 && First_rtn.width > 40)
		{
			std::cout<<"alliance"<<std::endl;
			if(!controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) && !runningAuton)
			{
			//	topSystem.move(0);
			}
			if(!canLimit && runningAuton)
			{
				topSystem.move(mainSpeed);
			}
			#ifdef BLUE
			vSensor.set_led(COLOR_BLUE);
			#else
			vSensor.set_led(COLOR_RED);
			#endif
			rearSystem.move(-mainSpeed);
			
			topSystem.move(-mainSpeed);
			
		}
		//if the alliance ball is not detected then search for the enemy ball for discarding.
		else
		if(Second_rtn.signature != 255 && Second_rtn.width > 40)
		{
			std::cout<<"enemy"<<std::endl;
			#ifdef BLUE
			vSensor.set_led(COLOR_RED);
			#else
			vSensor.set_led(COLOR_BLUE);
			#endif
			
			if(!disableBottom)
				rearSystem.move(mainSpeed);
			pros::delay(delayEject);
		}
		//if nothing was found then just load like normal
		else
		{
			/*
			if(!canLimit && runningAuton)
			{
				topSystem.move(mainSpeed);
			}
			//std::cout<<"nothing"<<std::endl;
			if(!controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) && !runningAuton)
			{
				//topSystem.move(0);
			}
			*/
			vSensor.set_led(COLOR_LIGHT_CORAL);
			topSystem.move(-mainSpeed);
			rearSystem.move(-mainSpeed);
			/*
			if(!disableTop && controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2))
				topSystem.move(mainSpeed);
			if(!disableBottom && (!seeBall && !canLimit))
				rearSystem.move(mainSpeed);
			if(!disableBottom && topSystem.get_actual_velocity() > minVelocity && controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2))
				rearSystem.move(mainSpeed);
			if(!canLimit && runningAuton && topSystem.get_actual_velocity() > minVelocity)
				rearSystem.move(mainSpeed);
*/
		}
		//make the thread sleep to prevent other threads from being starved of resources.
		pros::Task::delay(10);
	}
}