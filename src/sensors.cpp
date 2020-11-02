#include"sensors.hpp"

//define sig indexes
#define ALLIANCE_SIG 1
#define ENEMEY_SIG 2

//signatures generated using the vision utility	
pros::vision_signature_s_t BLUE_SIG = pros::Vision::signature_from_utility(1, -3495, -2369, -2932, 9043, 15777, 12410, 2.000, 0);
pros::vision_signature_s_t RED_SIG = pros::Vision::signature_from_utility(2, 5439, 7267, 6352, -1, 811, 404, 2.500, 0);

//Sensor init
pros::Vision vSensor(8, pros::E_VISION_ZERO_CENTER);
pros::ADILineSensor topLimit('A');

//bool that holds the state of the limiter
extern bool canLimit;

//define the alliance color to sort the correct ball color.
#define BLUE

//tuning variables
static int32_t delayEject = 250;
static int32_t bottomSpeed = 127;
static const int32_t lowSpeed = 80;

int32_t topVelocity = 375;
static int32_t minVelocity = 350;

int32_t normalLineValue = 2900;
int32_t lowLineValue = 2800;

//enable/disable sorting task
bool SORT_SYS_ENABLE = true;

unsigned int limitPresses = 0;

//polls line sensor to check for when a ball passes through.
void pollSensors()
{
	while(true)
	{
		while(topLimit.get_value() > lowLineValue)
		{
			pros::delay(10);
		}
		pros::delay(100);
		while(topLimit.get_value() < normalLineValue)
		{
			pros::delay(10);
		}
		limitPresses++;
		pros::lcd::print(5, "%d", limitPresses);
		pros::delay(10);
	}
}

bool disableTop = false;
bool disableBottom = false;
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
	#ifdef RED
	vSensor.set_signature(ENEMEY_SIG, &RED_SIG);
	#else
	vSensor.set_signature(ENEMEY_SIG, &BLUE_SIG);
	#endif

	//set loader brake modes to lock so the alliance ball can stop at the top of the loader
	topSystem.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	bottomSystem.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	
	while(true)
	{
		if(!runningAuton)
		{
			if(!controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2))
			{
				bottomSpeed = lowSpeed;
				topSystem.move(0);
				canLimit = true;
			}
			else
			{
				bottomSpeed = 127;
				canLimit = false;
			}
			if(topLimit.get_value() > lowLineValue)
			{
				bottomSystem.move(bottomSpeed);
			}
			if(topLimit.get_value() < lowLineValue && canLimit)
			{
				bottomSystem.move_velocity(0);
			}
		}
		
        //if the sorting system is disabled then don't attemp to sort.
        if(!SORT_SYS_ENABLE)
            continue;
	
		//get the largest object(0), based on the signature passed in.
		pros::vision_object_s_t First_rtn = vSensor.get_by_sig(0, ALLIANCE_SIG);
		pros::vision_object_s_t Second_rtn = vSensor.get_by_sig(0, ENEMEY_SIG);

		if(runningAuton)
		{
			//std::cout<<"auton"<<std::endl;
			static bool runSwitch = false;

			//if the top limiter sensor is hit and the program is allowed to limit, stop loading more.
			if(topLimit.get_value() < normalLineValue && canLimit && !runSwitch)
			{
				topSystem.move_velocity(0);
				
				bottomSystem.move_velocity(0);
				if(!disableTop)
					topSystem.move_velocity(topVelocity);
				runSwitch = true;
				std::cout<<"LIMIT"<<std::endl;
				continue;
			}

			if(topLimit.get_value() < normalLineValue && canLimit && runSwitch)
			{
				std::cout<<"waitinggg"<<std::endl;
				bottomSystem.move_velocity(0);
				continue;
			}

			if(!(topLimit.get_value() < normalLineValue) && canLimit && runSwitch)
			{
				runSwitch = false;
			}
		}
		else
		{
			if(topLimit.get_value() < normalLineValue && canLimit && !controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2))
			{
				bottomSystem.move_velocity(0);
				//topSystem.move_velocity(0);
				//std::cout<<"stoppping"<<std::endl;
				continue;
			}			
		}
		
        /*255 returns if no objects of stated signature is found.*/

		//if both sigs are found then sort based on color and positioning
		if(Second_rtn.signature != 255 && First_rtn.signature != 255 && First_rtn.width > 15 && Second_rtn.width > 15)
		{
			std::cout<<"both";
			if(First_rtn.y_middle_coord > Second_rtn.y_middle_coord)
			{
				vSensor.set_led(COLOR_GREEN);
				if(!disableTop && (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) || runningAuton))
					topSystem.move_velocity(-topVelocity);
				if(!disableBottom && topSystem.get_actual_velocity() > minVelocity && topSystem.get_direction() == 1 && canLimit == false)
				{
					std::cout<<"moving"<<std::endl;
					bottomSystem.move(bottomSpeed);
				}
			}
			else
			{
				vSensor.set_led(COLOR_GREEN);
				if(!disableTop && (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) || runningAuton))
				{
					topSystem.move_velocity(topVelocity);
				}
				else if(!disableTop && !controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2))
				{
					topSystem.move(0);
				}
				
				if(!disableBottom && topSystem.get_actual_velocity() > minVelocity && topSystem.get_direction() == 1 && canLimit == false)
					bottomSystem.move(bottomSpeed);
			}
		}
		
		//if the alliance color ball was found the just load up
		else
		if(First_rtn.signature != 255 && First_rtn.width > 40)
		{
			std::cout<<"alliance";
			#ifdef RED
			vSensor.set_led(COLOR_BLUE);
			#else
			vSensor.set_led(COLOR_RED);
			#endif
					
			if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) || runningAuton)
			{
				if(runningAuton)
				{
					topSystem.move_velocity(topVelocity);
				}
				if(topSystem.get_actual_velocity() > minVelocity && topSystem.get_direction() == 1)
				{
					if(!disableBottom)
						bottomSystem.move(bottomSpeed);
					if(!disableTop)
					{
						topSystem.move_velocity(topVelocity);
					}
				}
				else
				{
					if(!disableTop && controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2))
						topSystem.move_velocity(topVelocity);
					//bottomSystem.move_velocity(0);
				}
			}
		}
		//if the alliance ball is not detected then search for the enemy ball for discarding.
		else
		if(Second_rtn.signature != 255 && Second_rtn.width > 40)
		{
			std::cout<<"enemy";
			#ifdef RED
			vSensor.set_led(COLOR_RED);
			#else
			vSensor.set_led(COLOR_BLUE);
			#endif
			if(!disableTop)
				topSystem.move_velocity(-topVelocity);
			if(!disableBottom)
				bottomSystem.move(bottomSpeed);
			pros::delay(delayEject);
		}
		//if nothing was found then just load like normal
		else
		{
			std::cout<<"nothing "<<canLimit<<std::endl;
			vSensor.set_led(COLOR_LIGHT_CORAL);

			if(!disableTop && (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) || runningAuton))
				topSystem.move_velocity(topVelocity);
			if(!disableBottom && !canLimit)
			{
				std::cout<<"m"<<std::endl;
				bottomSystem.move(bottomSpeed);
			}
		}
		if(!controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2) && !runningAuton)
			topSystem.move(0);
		//make the thread sleep to prevent other threads from being starved of resources.
		pros::Task::delay(10);
	}
}