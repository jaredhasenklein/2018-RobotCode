#include <iostream>
#include <memory>
#include <string>
#include "math.h"

// And So It Begins...
#include "RJ_RobotMap.h"


class Robot: public frc::TimedRobot {

	// Robot Hardware Setup
	RJ_RobotMap IO;


	// Built-In Drive code for teleop
	DifferentialDrive Adrive { IO.DriveBase.MotorsLeft, IO.DriveBase.MotorsRight };

	// Drive Input Filter
	float OutputX = 0.0, OutputY = 0.0;

	// create pdp variable
	PowerDistributionPanel *pdp = new PowerDistributionPanel();

	// State Variables
	bool driveButtonYPrev = false;
	bool intakeDeployed = false;
	bool XYDeployed = false;

	//Autonomous Variables
	int modeState;
	int isWaiting = 0;
	Timer AutonTimer;
	std::string gameData, autoDelay, autoSelected;
	int modeState, DriveState, TurnState, ScaleState, NearSwitch, AutoSpot, LeftMode;
	bool AutonOverride, AutoDelayActive;



	void RobotInit() {
		//disable drive watchdogs
		Adrive.SetSafetyEnabled(false);
	}

	void RobotPeriodic() {
		// Update Smart Dash
		SmartDashboardUpdate();
		IO.NavXDebugger();
		gameData = frc::DriverStation::GetInstance().GetGameSpecificMessage();
	}

	void DisabledPeriodic() {
		// NOP
	}

	void TeleopInit() {
		// drive command averaging filter
		OutputX = 0, OutputY = 0;
	}

	void TeleopPeriodic() {
		double Deadband = 0.11;
		double DPadSpeed = 1.0;
		bool RightStickLimit1 = IO.TestJunk.DiIn8.Get();
		bool RightStickLimit2 = IO.TestJunk.DiIn9.Get();

		//high gear & low gear controls
		if (IO.DS.DriveStick.GetRawButton(5))
			IO.DriveBase.SolenoidShifter.Set(true);// High gear press LH bumper

		if (IO.DS.DriveStick.GetRawButton(6))
			IO.DriveBase.SolenoidShifter.Set(false);// Low gear press RH bumper

		//  Rumble code
		//  Read all motor current from PDP and display on drivers station
		//double driveCurrent = pdp->GetTotalCurrent();	// Get total current
		double driveCurrent = 0;

		// rumble if current to high
		double LHThr = 0.0;		// Define value for rumble
		if (driveCurrent > 125.0)// Rumble if greater than 125 amps motor current
			LHThr = 0.5;
		Joystick::RumbleType Vibrate;				// define Vibrate variable
		Vibrate = Joystick::kLeftRumble;		// set Vibrate to Left
		IO.DS.DriveStick.SetRumble(Vibrate, LHThr); // Set Left Rumble to RH Trigger
		Vibrate = Joystick::kRightRumble;		// set vibrate to Right
		IO.DS.DriveStick.SetRumble(Vibrate, LHThr);// Set Right Rumble to RH Trigger

		//drive controls
		double SpeedLinear = IO.DS.DriveStick.GetRawAxis(1) * 1; // get Yaxis value (forward)
		double SpeedRotate = IO.DS.DriveStick.GetRawAxis(4) * -1; // get Xaxis value (turn)

		// Set dead band for X and Y axis
		if (fabs(SpeedLinear) < Deadband)
			SpeedLinear = 0.0;
		if (fabs(SpeedRotate) < Deadband)
			SpeedRotate = 0.0;

		//slow down direction changes from 1 cycle to 5
		OutputY = (0.8 * OutputY) + (0.2 * SpeedLinear);
		OutputX = (0.8 * OutputX) + (0.2 * SpeedRotate);

		// Drive Code
		Adrive.ArcadeDrive(OutputY, OutputX, true);


		/*
		 * MANIP CODE
		 */

		//A Button to extend (Solenoid On)
		IO.TestJunk.Abutton.Set(IO.DS.OperatorStick.GetRawButton(1));

		//B Button to extend (Solenoid On)
		IO.TestJunk.Bbutton.Set(IO.DS.OperatorStick.GetRawButton(2));

		//if Left Bumper button pressed, extend (Solenoid On)
		if (IO.DS.OperatorStick.GetRawButton(5)) {
			intakeDeployed = true;
			IO.TestJunk.IntakeButton.Set(intakeDeployed);
		}

		//else Right Bumper pressed, retract (Solenoid Off)
		else if (IO.DS.OperatorStick.GetRawButton(6)) {
			intakeDeployed = false;
			IO.TestJunk.IntakeButton.Set(intakeDeployed);
		}
		//if 'X' button pressed, extend (Solenoid On)
		if (IO.DS.OperatorStick.GetRawButton(3)) {
			XYDeployed = true;
			IO.TestJunk.XYbutton.Set(XYDeployed);
		}

		//else 'Y' button pressed, retract (Solenoid Off)
		else if (IO.DS.OperatorStick.GetRawButton(4)) {
			XYDeployed = false;
			IO.TestJunk.XYbutton.Set(XYDeployed);
		}

		//dpad POV stuff
		if (IO.DS.OperatorStick.GetPOV(0) == 0) {
			IO.TestJunk.Dpad1.Set(DPadSpeed);
			IO.TestJunk.Dpad2.Set(DPadSpeed);
		} else if (IO.DS.OperatorStick.GetPOV(0) == 180) {
			IO.TestJunk.Dpad1.Set(-DPadSpeed);
			IO.TestJunk.Dpad2.Set(-DPadSpeed);
		} else {
			IO.TestJunk.Dpad1.Set(0.0);
			IO.TestJunk.Dpad2.Set(0.0);
		}

		double RightSpeed = IO.DS.OperatorStick.GetRawAxis(4) * -1; // get Xaxis value for Right Joystick

		if (fabs(RightSpeed) < Deadband) {
			RightSpeed = 0.0;
		} else if (RightSpeed > Deadband and !RightStickLimit1)
			RightSpeed = 0.0;
		else if (RightSpeed < Deadband and !RightStickLimit2)
			RightSpeed = 0.0;

		IO.TestJunk.RightStick1.Set(RightSpeed);
		IO.TestJunk.RightStick2.Set(RightSpeed);

	}


	void AutonomousInit() {
		//AutoProgram.Initalize();

		modeState = 1;
		isWaiting = 0;							/////***** Rename this.

		AutonTimer.Reset();
		AutonTimer.Start();
		// Encoder based auton
		IO.DriveBase.EncoderLeft.Reset();
		IO.DriveBase.EncoderRight.Reset();
		// Turn off drive motors
		IO.DriveBase.MotorsLeft.Set(0);

		IO.DriveBase.MotorsRight.Set(0);

		//zeros the navX
		if (ahrs) {
			ahrs->ZeroYaw();
		}
		//forces robot into low gear
		IO.DriveBase.SolenoidShifter.Set(false);

		//makes sure claw clamps shut
		IO.DriveBase.ClawClamp.Set(DoubleSolenoid::Value::kForward)






	}

#define caseLeft 1
#define caseRight 2

	void AutonomousPeriodic() {



		SmartDashboard::PutString("gameData", gameData);
				if(gameData[0] == 'L')
					NearSwitch = caseLeft;
				 else
					NearSwitch = caseRight;
				// Set far switch game state
				if(gameData[1] == 'L')
					ScaleState = caseLeft;
				 else
					ScaleState = caseRight;

				if (autoDelay == IO.DS.sAutoDelay3 and AutonTimer.Get()< 3){
					AutoDelayActive = true;
				}
				else if (autoDelay == IO.DS.sAutoDelay5 and AutonTimer.Get()< 5){
					AutoDelayActive = true;
				}
				else if (AutoDelayActive){
					AutoDelayActive=false;
					autoDelay=IO.DS.sAutoDelayOff;
					AutonTimer.Reset();
				}

				if(autoSelected==AutoLeftSpot and NearSwitch==caseLeft and !AutoDelayActive){
					AutoLeftSwitchLeft();
				}
				else if(autoSelected==AutoLeftSpot and NearSwitch==caseRight and !AutoDelayActive){
					AutoLeftSwitchRight();
				}
				else if (autoSelected==AutoCenterSpot and !AutoDelayActive)
					AutoCenter();
				else if (autoSelected==AutoRightSpot and NearSwitch==caseLeft  and !AutoDelayActive)
					AutoRightSwitchLeft();
				else if (autoSelected==AutoRightSpot and NearSwitch==caseRight  and !AutoDelayActive)
					AutoRightSwitchRight();



	}





#define AB1_INIT 1
#define AB1_FWD 2
#define	AB1_TURN90 3
#define AB1_FWD2 4
#define AB1_SCORE 5
#define AB1_BACK 6
#define AB1_END 7
	void autoBlue1(void) {
		//blue side code
		//Starts from the center and drives to put the cube in the switch
		switch (modeState) {
		case AB1_INIT:
			// This uses state 1 for initialization.
			// This keeps the initialization and the code all in one place.
			ahrs->ZeroYaw();
			modeState = AB1_FWD;
			break;
		case AB1_FWD:
			// Drives forward off the wall to perform the turn
			// TODO: adjust value
			if (forward(71.0)) {
				AutonTimer.Reset();
				modeState = AB1_TURN90;
			}
			break;
		case AB1_TURN90:
			// Turns 90 in the direction of the switch goal
			if (autonTurn(90)) {
				AutonTimer.Reset();
				modeState = AB1_FWD2;
			}
			break;
		case AB1_FWD2:
			// Drives forward to the switch goal
			// TODO: adjust value
			// TODO: should also be adjusting elevator height and claw location during this move
			if (forward(71.0)) {
				modeState = AB1_SCORE;
			}
			break;
		case AB1_SCORE:
			// Opens the claw to drop the pre-loaded cube
			// TODO: confirm directionality
			// TODO: consider separate function for clamp opening to coordinate wheel motion
			if (1) {
				IO.DriveBase.ClawClamp.Set(DoubleSolenoid::Value::kForward);
				modeState = AB1_BACK;
			}
			break;
		case AB1_BACK:
			if (timedDrive(5.0, 0.3, 0.3)) {
				AutonTimer.Reset();
				modeState = AB1_END;
			}
			break;

		default:
			stopMotors();
		}
		return;
	}






	void motorSpeed(double leftMotor, double rightMotor) {
		IO.DriveBase.MotorsLeft.Set(leftMotor * -1);
		IO.DriveBase.MotorsRight.Set(rightMotor);

		}

	int stopMotors() {
			//sets motor speeds to zero
			motorSpeed(0, 0);
			return 1;
		}


		int forward(double targetDistance) {
			//put all encoder stuff in same place
			double encoderDistance;
			if (useRightEncoder)
				encoderDistance = IO.DriveBase.EncoderRight.GetDistance();
			else
				encoderDistance = IO.DriveBase.EncoderLeft.GetDistance();

			double encoderError = encoderDistance - targetDistance;
			double driveCommandLinear = encoderError * KP_LINEAR;

			//limits max drive speed
			if (driveCommandLinear > LINEAR_MAX_DRIVE_SPEED) {
				driveCommandLinear = LINEAR_MAX_DRIVE_SPEED;
			} else if (driveCommandLinear < -1 * LINEAR_MAX_DRIVE_SPEED) { /////***** "-1" is a "magic number." At least put a clear comment in here.
				driveCommandLinear = -1 * LINEAR_MAX_DRIVE_SPEED; /////***** same as above.
			}

			double gyroAngle = ahrs->GetAngle();
			double driveCommandRotation = gyroAngle * KP_ROTATION;
			//calculates and sets motor speeds
			motorSpeed(driveCommandLinear + driveCommandRotation,
					driveCommandLinear - driveCommandRotation);

			//routine helps prevent the robot from overshooting the distance
			if (isWaiting == 0) { /////***** Rename "isWaiting."  This isWaiting overlaps with the autonTurn() isWaiting.  There is nothing like 2 globals that are used for different things, but have the same name.
				if (abs(encoderError) < LINEAR_TOLERANCE) {
					isWaiting = 1;
					AutonTimer.Reset();
				}
			}
			//timed wait
			else {
				float currentTime = AutonTimer.Get();
				if (abs(encoderError) > LINEAR_TOLERANCE) {
					isWaiting = 0;					/////***** Rename
				} else if (currentTime > LINEAR_SETTLING_TIME) {
					isWaiting = 0;					/////***** Rename
					return 1;
				}
			}
			return 0;
		}

		int autonTurn(float targetYaw) {

			float currentYaw = ahrs->GetAngle();
			float yawError = currentYaw - targetYaw;

			motorSpeed(-1 * yawError * ERROR_GAIN, yawError * ERROR_GAIN);

			if (isWaiting == 0) {/////***** Rename "isWaiting."  This isWaiting overlaps with the forward() isWaiting.  There is nothing like 2 globals that are used for different things, but have the same name.
				if (abs(yawError) < ROTATIONAL_TOLERANCE) {
					isWaiting = 1;
					AutonTimer.Reset();
				}
			}
			//timed wait
			else {
				float currentTime = AutonTimer.Get();
				if (abs(yawError) > ROTATIONAL_TOLERANCE) {
					isWaiting = 0;
				} else if (currentTime > ROTATIONAL_SETTLING_TIME) {
					isWaiting = 0;
					return 1;
				}
			}
			return 0;
		}

		int timedDrive(double driveTime, double leftMotorSpeed,
				double rightMotorSpeed) {
			float currentTime = AutonTimer.Get();
			if (currentTime < driveTime) {
				motorSpeed(leftMotorSpeed, rightMotorSpeed);
			} else {
				stopMotors();
				return 1;
			}
			return 0;
		}






		void SmartDashboardUpdate() {

				// Auto State
				SmartDashboard::PutNumber("Auto Switch (#)", AutoVal);
				SmartDashboard::PutString("Auto Program", autoSelected);
				SmartDashboard::PutNumber("Auto State (#)", modeState);
				SmartDashboard::PutNumber("Auto Timer (s)", AutonTimer.Get());

				// Drive Encoders
				SmartDashboard::PutNumber("Drive Encoder Left (RAW)",
						EncoderLeft.GetRaw());
				SmartDashboard::PutNumber("Drive Encoder Left (Inches)",
						EncoderLeft.GetDistance());

				SmartDashboard::PutNumber("Drive Encoder Right (RAW)",
						EncoderRight.GetRaw());
				SmartDashboard::PutNumber("Drive Encoder Right (Inch)",
						EncoderRight.GetDistance());

				encoderSelected = chooseDriveEncoder.GetSelected();
				useRightEncoder = (encoderSelected == RH_Encoder);

				autoBackupDistance = SmartDashboard::GetNumber(
						"IN: Auto Backup Distance (Inch)", autoBackupDistance);
				SmartDashboard::PutNumber("Auto Backup Distance", autoBackupDistance);

				// Gyro
				if (ahrs) {
					double gyroAngle = ahrs->GetAngle();
					SmartDashboard::PutNumber("Gyro Angle", gyroAngle);
				} else {
					SmartDashboard::PutNumber("Gyro Angle", 999);
				}




}

}
;

START_ROBOT_CLASS(Robot);
