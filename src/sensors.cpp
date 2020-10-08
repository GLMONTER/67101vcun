#include"sensors.hpp"

//define sig indexes
#define ALLIANCE_SIG 1
#define ENEMEY_SIG 2

//signatures generated using the vision utility	
pros::vision_signature_s_t BLUE_SIG = pros::Vision::signature_from_utility(1, -3719, -3175, -3446, 10543, 14845, 12694, 2.500, 0);
pros::vision_signature_s_t RED_SIG = pros::Vision::signature_from_utility(2, 7969, 10049, 9010, -447, 815, 184, 2.500, 0);

//Sensor init
pros::Vision vSensor(8, pros::E_VISION_ZERO_CENTER);
pros::ADIDigitalIn topLimit('A');

//bool that holds the state of the limiter
extern bool canLimit;
//define the alliance color to sort the correct ball color.
#define RED

//tuning variables for the different robots.
static int32_t delayEject = 250;
static int32_t bottomSpeed = 127;
static int32_t topVelocity = 440;
static int32_t minVelocity = 400;

//enable/disable sorting task
bool SORT_SYS_ENABLE = true;

unsigned int limitPresses = 0;

//polls limit switch to check for when a ball passes through.
void pollSensors()
{
	while(true)
	{
		if(topLimit.get_new_press())
		{
			limitPresses++;
			std::cout<<limitPresses<<std::endl;
			std::cout<<"new press!"<<std::endl;
		}
		pros::delay(10);
	}
}
extern bool runningAuton;
//this function will sort the balls based on the color signature passed in. 
//The task will start at the beginning of the program with the correct ball color to start.
void sort(void* sigPass)
{
    pros::vision_signature_s_t sig =  *reinterpret_cast<pros::vision_signature_s_t*>(sigPass);
	//resetting vision sensor LED color.
	vSensor.clear_led();

	//set the red and blue signatures to be referenced later.
	vSensor.set_signature(ALLIANCE_SIG, &sig);
	#ifdef BLUE
	vSensor.set_signature(ENEMEY_SIG, &RED_SIG);
	#else
	vSensor.set_signature(ENEMEY_SIG, &BLUE_SIG);
	#endif

	//set loader brake modes to lock so the alliance ball can stop at the top of the loader
	topSystem.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	bottomSystem.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	
	while(true)
	{
        //if the sorting system is disabled then don't attemp to sort.
        if(!SORT_SYS_ENABLE)
            continue;
	
		//get the largest object(0), based on the signature passed in.
		pros::vision_object_s_t First_rtn = vSensor.get_by_sig(0, ALLIANCE_SIG);
		pros::vision_object_s_t Second_rtn = vSensor.get_by_sig(0, ENEMEY_SIG);

		if(runningAuton)
		{
			static bool runSwitch = false;

			//if the top limiter sensor is hit and the program is allowed to limit, stop loading more.
			if(topLimit.get_value() && canLimit && !runSwitch)
			{
				topSystem.move_velocity(0);
				bottomSystem.move(-127);
				pros::delay(75);
				bottomSystem.move_velocity(0);
				topSystem.move(127);
				runSwitch = true;
				std::cout<<"LIMIT"<<std::endl;
				continue;
			}

			if(topLimit.get_value() && canLimit && runSwitch)
			{
				bottomSystem.move_velocity(0);
				continue;
			}

			if(!topLimit.get_value() && canLimit && runSwitch)
			{
				runSwitch = false;
			}
		}
		else
		{
			if(topLimit.get_value() && canLimit)
			{
				bottomSystem.move_velocity(0);
				topSystem.move_velocity(0);

				continue;
			}
		}
		
		

        /*255 returns if no objects of stated signature is found.*/

		//if both sigs are found then sort based on color and positioning
		if(Second_rtn.signature != 255 && First_rtn.signature != 255 && First_rtn.width > 15 && Second_rtn.width > 15)
		{
			if(First_rtn.y_middle_coord > Second_rtn.y_middle_coord)
			{
				vSensor.set_led(COLOR_GREEN);
				topSystem.move_velocity(-topVelocity);
				bottomSystem.move(bottomSpeed);
			}
			else
			{
				vSensor.set_led(COLOR_GREEN);
				topSystem.move_velocity(topVelocity);
				bottomSystem.move(bottomSpeed);
			}
		}
		//if the alliance color ball was found the just load up
		else
		if(First_rtn.signature != 255 && First_rtn.width > 40)
		{
			#ifdef BLUE
			vSensor.set_led(COLOR_BLUE);
			#else
			vSensor.set_led(COLOR_RED);
			#endif
			if(topSystem.get_actual_velocity() > minVelocity)
			{
				topSystem.move_velocity(topVelocity);
				bottomSystem.move(bottomSpeed);
			}
			else
			{
				topSystem.move_velocity(topVelocity);
				bottomSystem.move(0);
			}
			
		}
		//if the alliance ball is not detected then search for the enemy ball for discarding.
		else
		if(Second_rtn.signature != 255 && Second_rtn.width > 40)
		{
			#ifdef BLUE
			vSensor.set_led(COLOR_RED);
			#else
			vSensor.set_led(COLOR_BLUE);
			#endif
			topSystem.move_velocity(-topVelocity);
			bottomSystem.move(bottomSpeed);
			pros::delay(delayEject);
		}
		//if nothing was found then just load like normal
		else
		{
			vSensor.set_led(COLOR_LIGHT_CORAL);

			topSystem.move_velocity(topVelocity);
			bottomSystem.move(bottomSpeed);
		}
		//make the thread sleep to prevent other threads from being starved of resources.
		pros::Task::delay(10);
	}
}