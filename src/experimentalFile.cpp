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
    if(controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B))
    {
        copyString += std::string("moveToPoint(" + std::to_string(position.x) + ", " + std::to_string(position.y) + ", "+ std::to_string(position.a) + ");" + "\n");
    }
    if(controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2))
    {
        std::cout<<"COPY : \n \n"<<copyString<<"\n \n END";
    }
}

