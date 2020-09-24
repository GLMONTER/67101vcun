#include"main.h"


void sortFailsafe();
void setDrive(int32_t leftPower, int32_t rightPower);
void setLoaders(int32_t leftPower, int32_t rightPower);

extern pros::Controller controller;

extern pros::Motor leftLoader;
extern pros::Motor rightLoader;

extern pros::Motor rightFront;
extern pros::Motor leftFront;
extern pros::Motor rightBack;
extern pros::Motor leftBack;

extern pros::Motor topSystem;
extern pros::Motor bottomSystem;