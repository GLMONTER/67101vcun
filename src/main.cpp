#include "main.h"
#include"display/lvgl.h"

void initialize()
{
	//init gyro and screen
	pros::lcd::initialize();
    imu.reset();
    pros::delay(2000);
	
	pros::Task pollTask(pollSensors, "poll");
	
	//start the async sort task to begin sorting during driver control.
	pros::Task sortTask(sort);
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

//toggles for sorting system
bool sortToggle = 1;
static bool sortPressed = 0;

bool canLimit = false;

static bool topToggle = false;
static bool topPressed;
//extern void trackPosition();
extern bool runningAuton;
//extern void moveToPoint(const float x, const float y, const float angle);
void opcontrol() 
{
	runningAuton = false;
	/*
	LV_IMG_DECLARE(vaquita);

	
	lv_obj_t * im = lv_img_create(lv_scr_act(), NULL);
	
	lv_img_set_src(im, &vaquita);
	lv_obj_set_pos(im,  0, -75);
	lv_obj_set_drag(im, false);
*/
	while (true) 
	{
		static int i = 0;
		if(i == 100)
		{
			std::string faultStatus;
			if(leftBack.get_faults() == pros::motor_fault_e_t::E_MOTOR_FAULT_NO_FAULTS && 
			leftFront.get_faults() == pros::motor_fault_e_t::E_MOTOR_FAULT_NO_FAULTS && 
			rightBack.get_faults() == pros::motor_fault_e_t::E_MOTOR_FAULT_NO_FAULTS && 
			rightFront.get_faults() == pros::motor_fault_e_t::E_MOTOR_FAULT_NO_FAULTS)
			{
				faultStatus = "OK";
			}
			else
			{
				faultStatus == "FAULT!";
			}
			
			std::string topTemp = "TOP:" + std::to_string((int)topSystem.get_temperature()) 
			+ " BOT:" + std::to_string((int)rearSystem.get_temperature()) + " : " + faultStatus.c_str();
	

			controller.print(0, 0, "%s" , topTemp.c_str());

			i = 0;
		}
		i++;
		int32_t ch1 = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
		int32_t ch2 = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
		int32_t ch3 = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
		int32_t ch4 = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_X);

		leftFront.move(ch3 + ch1 + ch4);
		rightFront.move(ch3 - ch1 - ch4);
		leftBack.move(ch3 + ch1 - ch4);
		rightBack.move(ch3 - ch1 + ch4);

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
