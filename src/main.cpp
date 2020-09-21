#include "main.h"
#include"display/lvgl.h"

void initialize()
{
/*
	//release hood by spinning Trio.
	Trio.move(127);
	pros::Task::delay(250);
	Trio.move(0);
*/
	#define BLUE
	
	#ifdef BLUE
	//start the async sort task to begin sorting during driver control.
	//pros::Task sortTask(sort, reinterpret_cast<void*>(&BLUE_SIG),"test");
	#else
	pros::Task sortTask(sort, reinterpret_cast<void*>(&RED_SIG),"test");
	#endif
}

void disabled()
{
	
}


void competition_initialize()
{

}

extern void runAuton();
void autonomous()
{
	runAuton();
}


//toggles for Trio
static bool flyToggle = 0;
static bool flyPressed = 0;

//toggles for sorting system
bool sortToggle = 1;
static bool sortPressed = 0;

bool canLimit = true;

static bool topToggle = true;
static bool topPressed;
//pros::ADIEncoder left('B', 'C');
void opcontrol() 
{
	while (true) 
	{
		int32_t ch1 = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
		int32_t ch2 = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
		int32_t ch3 = -controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
		int32_t ch4 = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_X);

		leftFront.move(ch3 + ch1 + ch4);
		rightFront.move(ch3 - ch1 - ch4);
		leftBack.move(ch3 + ch1 - ch4);
		rightBack.move(ch3 - ch1 + ch4);
		

		//a load toggle to allow shooting.
		if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_UP))
		{
			if(!topPressed)
			{
				topToggle = 1 - topToggle;

				topPressed = 1;
			}
		}
		else
			topPressed = 0;

		if(topToggle)
		{
			canLimit = true;
		}
		else
		{
			canLimit = false;
		}

		//a failsafe for the sorting system
		if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_X))
		{
			if(!sortPressed)
			{
				sortToggle = 1 - sortToggle;

				sortPressed = 1;
			}
		}
		else
			sortPressed = 0;

		if(sortToggle)
		{
			SORT_SYS_ENABLE = true;
		}
		else
		{
			SORT_SYS_ENABLE = false;
			sortFailsafe();
		}

		//Trio CONTROLER and im big gey
		if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2))
		{
			if(!flyPressed)
			{
				flyToggle = 1 - flyToggle;

				flyPressed = 1;
			}
		}
		else
			flyPressed = 0;

		//if the toggle is enabled then start the Trio, if disabled then stop it
		if(flyToggle)
		{
			//Trio.move(-127);
		}
		else
		{
			//Trio.move(0);
		}

		//LOADING SYSTEM.
		if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1))
		{
			rightLoader.move(127);
			leftLoader.move(127);
		}
		else
		if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1))
		{
			rightLoader.move(-127);
			leftLoader.move(-127);
		}
		else
		if(!controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1) && !controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1))
		{
			rightLoader.move(0);
			leftLoader.move(0);
		}

		pros::delay(10);
	}
}
