#include "main.h"
#include"display/lvgl.h"
extern void tracking();
extern void trackPosition();
void initialize()
{
	//init gyro and screen
	pros::lcd::initialize();
	
	pros::Task pollTask(pollSensors, "poll");
	
	//start the async sort task to begin sorting during driver control.
	pros::Task sortTask(sort);

	//start custom controller tracking.
	pros::Task controllerTrack(tracking);

	//pros::Task TRACKPOS(trackPosition);
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

struct Position
{
    float a;
    float x;
    float y;
    int32_t leftLst;
    int32_t rightLst;
    int32_t backLst;
};

extern Position position;

//toggles for sorting system
bool sortToggle = 1;
static bool sortPressed = 0;

bool canLimit = false;

static bool topToggle = false;
static bool topPressed;


extern bool runningAuton;
extern void moveToPoint(const float x, const float y, const float angle);

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
		/*
		while(true)
		{
			trackPosition();
		}
		*/
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
			std::string log = "angle :" + std::to_string(position.a * 180/3.14); //"Y:" + std::to_string(position.y) + " X:" + std::to_string(position.x) + "R:" + std::to_string(position.a);
			
			controller.print(0, 0, "%s" , log.c_str());

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
			setLoaders(loaderSetting::Forward);
		}
		else
		if(controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1))
		{
			setLoaders(loaderSetting::Backward);
		}
		else
		if(!controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1) && !controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1))
		{
			setLoaders(loaderSetting::Disabled);
		}
		pros::delay(5);
	}
}
