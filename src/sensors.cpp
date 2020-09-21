#include"sensors.hpp"

#define ALLIANCE_SIG 1
#define ENEMEY_SIG 2

//signatures generated using the vision utility	
pros::vision_signature_s_t BLUE_SIG = pros::Vision::signature_from_utility(1,-2757,1,-1378,1053,8719,4886,0.8,0);
pros::vision_signature_s_t RED_SIG = pros::Vision::signature_from_utility(2,4947,9669,7308,-2185,-455,-1320,0.8,0);

//Sensor init
pros::Vision vSensor(8, pros::E_VISION_ZERO_CENTER);
pros::ADIDigitalIn topLimit('E');

//bool that holds the state of the limiter
extern bool canLimit;
//define the alliance color to sort the correct ball color.
#define BLUE

bool SORT_SYS_ENABLE = true;
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
	

		//if the top limiter sensor is hit and the program is allowed to limit, stop loading more.
		if(topLimit.get_value() && canLimit)
		{
			topSystem.move_velocity(0);
            bottomSystem.move_velocity(0);
			continue;
		}

        /*255 returns if no objects of stated signature is found.*/

		//if both sigs are found then sort based on color and positioning
		if(Second_rtn.signature != 255 && First_rtn.signature != 255 && First_rtn.width > 15 && Second_rtn.width > 15)
		{
			if(First_rtn.y_middle_coord > Second_rtn.y_middle_coord)
			{
				vSensor.set_led(COLOR_GREEN);
				topSystem.move(-127);
				bottomSystem.move(127);
			}
			else
			{
				vSensor.set_led(COLOR_GREEN);
				topSystem.move(127);
				bottomSystem.move(127);
			}
		}
		//if the alliance color ball was found the just load up
		else
		if(First_rtn.signature != 255 && First_rtn.width > 10)
		{
			#ifdef BLUE
			vSensor.set_led(COLOR_BLUE);
			#else
			vSensor.set_led(COLOR_RED);
			#endif
			topSystem.move(127);
			bottomSystem.move(127);
		}
		//if the alliance ball is not detected then search for the enemy ball for discarding.
		else
		if(Second_rtn.signature != 255 && Second_rtn.width > 10)
		{
			#ifdef BLUE
			vSensor.set_led(COLOR_RED);
			#else
			vSensor.set_led(COLOR_BLUE);
			#endif
			topSystem.move(-127);
			bottomSystem.move(127);
			pros::Task::delay(200);
		}
		//if nothing was found then just load like normal
		else
		{
			vSensor.set_led(COLOR_LIGHT_CORAL);

			topSystem.move(127);
			bottomSystem.move(127);
		}
		//make the thread sleep to prevent other threads from being starved of resources.
		pros::Task::delay(10);
	}
}