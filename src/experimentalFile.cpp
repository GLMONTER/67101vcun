#include"main.h"
#include"control_sys.hpp"
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
std::string copyString;
void tracking()
{
    while(true)
    {
        if(controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B))
        {
            copyString += std::string("moveToPoint(" + std::to_string(position.x) + ", " + std::to_string(position.y) + ", "+ std::to_string(position.a) + ");" + "\n");
        }
        if(controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2))
        {
            std::cout<<"\n COPY : \n \n"<<copyString<<"\n \n END";
        }
        pros::Task::delay(5);
    }
}

