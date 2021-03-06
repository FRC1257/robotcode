#include "robot.h"
#include "math.h"

/**
 * This is a demo program showing the use of the RobotBase class.
 * The SimpleRobot class is the base of a robot application that will automatically call your
 * Autonomous and OperatorControl methods at the right time as controlled by the switches on
 * the driver station or the field controls.
 */

float targetSpeed[2] = { 0, 0 };
float *speeds = 0;
bool arms = false;

Robot::Robot() :
	rd(1, 2), stick(LEFT_STICK), angle(1), relay(1), ac(3), arm1(3), arm2(4) {
	rd.SetExpiration(0.1);
	lcd = dLcd::GetInstance();
	speed = 0;
}

#define PRINT(msg){lcd->Printf(dLcd::kUser_Line1, 1, msg);}//simple print
void Robot::Autonomous() {
	PRINT("Starting Autonomous");
	relay.Set(relay.kForward);
	bool got = false;
	while (IsAutonomous() && IsEnabled()) {
		if (!got) {
			if (vis.aim()) {
				print("Yay", 2);
				lcd->UpdateLCD();
			} else {
				print("Awww", 2);
				lcd->UpdateLCD();
			}
			got = true;
		} else
			continue;
	}
	relay.Set(relay.kOff);
}

void Robot::OperatorControl() {
	bool control = false;
	bool t = false;
	relay.Set(relay.kOff);
	PRINT("Started Operator Control");
	while (IsOperatorControl() && IsEnabled()) {
		if (stick.GetRawButton(5) && stick.GetRawButton(6))
			arcadeDrive();
		else if (stick.GetZ())
			drive();
		if (stick.GetRawButton(1)) {
			if (arms)
				arms = false;
			else
				arms = true;
		}
		if (arms) {
			control = controlArms(arms);
			if (control) {
				time.Start();
				while(!t)
				{
					if(time.Get() >= 1.0)
					{
						t = true;
						break;
					}
					else
						continue;
				}
				time.Stop(); time.Reset();
			}
		}
		else
		{
			control = false;
			t = false;
		}
	}
}

inline void Robot::drive(double x, double y) {
	rd.SetLeftRightMotorOutputs(x, y);
}

void Robot::drive()//tank drive
{
	double x = stick.GetY();
	double y = stick.GetRawAxis(5);
	accel(x, y);
}

void Robot::arcadeDrive() {
	double x = stick.GetX();
	double y = stick.GetRawAxis(5);
	rd.ArcadeDrive(x, y, false);
}

bool Robot::controlArms(bool arm) {
	if (arm) {
		double d = toMM(ac.GetVoltage());
		if (d <= 20) {
			return true;
		} else
			return false;
	}
	else
		return false;
}

void Robot::accel(double x, double y) {
	if (x == speed && y == speed)
		drive(speed, speed);
	if (x == y) {
		double target = x;
		double n = 0;
		if (target < 0) {
			while (n > target) {
				drive(n, n);
				n -= 0.05;
			}
			speed = n;
		} else {
			while (n < target) {
				drive(n, n);
				n += 0.05;
			}
		}
	} else
		drive(x * sf, y * sf);
}

void Robot::print(char* msg, int i) {
	if (i > 7)
		i = 6;
	if (i < 1)
		i = 1;

	switch (i) {
	case 1:
		lcd->Printf(dLcd::kUser_Line1, 1, msg);
		break;
	case 2:
		lcd->Printf(dLcd::kUser_Line2, 1, msg);
		break;
	case 3:
		lcd->Printf(dLcd::kUser_Line3, 1, msg);
		break;
	case 4:
		lcd->Printf(dLcd::kUser_Line4, 1, msg);
		break;
	case 5:
		lcd->Printf(dLcd::kUser_Line5, 1, msg);
		break;
	case 6:
		lcd->Printf(dLcd::kUser_Line6, 1, msg);
		break;
	}
	lcd->UpdateLCD();
}

inline double Robot::toMM(double volts) {
	return (volts / 2.5) * 6450;//* 253.937; // 
	//0-2.5 volts converted to MM (max distnace of 6,450mm)
}

inline double Robot::toIN(double volts) {
	return (toMM(volts) / 10) / 2.54; //dimensional analysis for the win!!!
}

START_ROBOT_CLASS(Robot)
;
