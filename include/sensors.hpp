#include"main.h"

extern bool SORT_SYS_ENABLE;
extern pros::vision_signature_s_t BLUE_SIG;
extern pros::vision_signature_s_t RED_SIG;
extern pros::Vision vSensor;
extern pros::Imu imu;
extern pros::ADIDigitalIn topLimit;
void sort(void* sigPass);
