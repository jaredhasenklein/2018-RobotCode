//2018-RobotCode
#include <iostream>
#include <memory>
#include <string>
#include "AHRS.h"
#include "WPILib.h"
#include "math.h"
#include "Encoder.h"

#include <IterativeRobot.h>
#include <LiveWindow/LiveWindow.h>
#include <SmartDashboard/SendableChooser.h>
#include <SmartDashboard/SmartDashboard.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

// Time forward: go forward for 0.75 seconds.
#define RED_2_CASE2_FWD (71.0)
#define RED_2_CASE3_TIME (1.0)
#define RED_2_CASE3_LSPEED (-0.2)
#define RED_2_CASE3_RSPEED (-0.2)
//linear calibrations
// tolerance in inches
#define LINEAR_TOLERANCE (0.2)
// This is the gain for using the encoders to set the distance
//		while going straight.
#define KP_LINEAR (0.27)
// This is the gain for using the gyroscope to go straight
#define KP_ROTATION (0.017)
#define LINEAR_SETTLING_TIME (0.1)
#define LINEAR_MAX_DRIVE_SPEED (0.75)

//turning calibrations
#define ROTATIONAL_TOLERANCE (1.0)
// This is the gain for turning using the gyroscope
#define ERROR_GAIN (-0.05)
#define ROTATIONAL_SETTLING_TIME (0.5)

//encoder max drive time
#define MAX_DRIVE_TIME (3.0)

class Robot: public frc::IterativeRobot {
	DifferentialDrive Adrive;
	frc::LiveWindow* lw = LiveWindow::GetInstance();
	frc::SendableChooser<std::string> chooseAutonSelector, chooseAutoDelay, chooseDriveEncoder,
			chooseKicker, chooseShooter, chooseLowDriveSens, chooseLowTurnSens,
			chooseHighDriveSens, chooseHighTurnSens;
	const std::string AutoDelayOff = "No Delay";
	const std::string AutoDelay1 = "3 second delay";
	const std::string AutoDelay2 = "5 second delay";
	const std::string AutoOff = "No Auto Mode";
	const std::string AutoLeft = "Left Position";
	const std::string AutoCenter = "Center Position";
	const std::string AutoRight = "Right Position";
	const std::string RH_Encoder = "RH_Encoder";
	const std::string LH_Encoder = "LH_Encoder";
	const std::string DriveDefault = "Standard";
	const std::string Drive1 = "Sens_x^2";
	const std::string Drive2 = "Sens_x^3";
	const std::string Drive3 = "Sens_x^5";
	const std::string TurnDefault = "Standard";
	const std::string Turn1 = "Sens_x^2";
	const std::string Turn2 = "Sens_x^3";
	const std::string Turn3 = "Sens_x^5";
	std::string autoSelected, autoDelay, encoderSelected, LowDriveChooser, LowTurnChooser,
			HighDriveChooser, HighTurnChooser, gameData;
	Joystick Drivestick;
	Joystick OperatorStick;
	VictorSP DriveLeft0;
	VictorSP DriveLeft1;
	VictorSP DriveLeft2;
	VictorSP DriveRight0;
	VictorSP DriveRight1;
	VictorSP DriveRight2;
	VictorSP Dpad1;
	VictorSP Dpad2;
	VictorSP RightStick1;
	VictorSP RightStick2;
	Timer AutonTimer;
	Timer EncoderCheckTimer;
	Encoder EncoderLeft;
	Encoder EncoderRight;

	double OutputX, OutputY;
	double OutputX1, OutputY1;
	DigitalInput DiIn8, DiIn9;

	AHRS *ahrs;
//tells us what state we are in in each auto mode
	int modeState, DriveState, TurnState, ScaleState, NearSwitch, FarSwitch, AutoSpot;
	bool AutonOverride, AutoDelayActive;
	int isWaiting = 0;			/////***** Divide this into 2 variables.

	// create pdp variable
	PowerDistributionPanel *pdp = new PowerDistributionPanel();

//Solenoid's declared
	Solenoid *driveSolenoid = new Solenoid(0);
	Solenoid *XYbutton = new Solenoid(4);
	Solenoid *Bbutton = new Solenoid(3);
	Solenoid *Abutton = new Solenoid(2);
	Solenoid *IntakeButton = new Solenoid(1);

	bool useRightEncoder = false;
	bool driveRightTriggerPrev = false;
	bool driveButtonYPrev = false;
	bool operatorRightTriggerPrev = false;
	bool intakeDeployed = false;
	bool XYDeployed = false;
	bool shooterOn = false;

public:
	Robot() :
			Adrive(DriveLeft0, DriveRight0), Drivestick(0), OperatorStick(1), DriveLeft0(
					0), DriveLeft1(1), DriveLeft2(2), DriveRight0(3), DriveRight1(
					4), DriveRight2(5), Dpad1(8), Dpad2(9), RightStick1(6), RightStick2(
					7), EncoderLeft(0, 1), EncoderRight(2, 3), OutputX(0), OutputY(
					0), OutputX1(0), OutputY1(0), DiIn8(8), DiIn9(9), ahrs(NULL),
					modeState(0), DriveState(0), TurnState(0), ScaleState(0), NearSwitch(0),
					FarSwitch(0), AutoSpot(0), AutonOverride(0), AutoDelayActive(0) {

	}

private:
	void RobotInit() {
		chooseAutonSelector.AddDefault(AutoOff, AutoOff);
		chooseAutonSelector.AddObject(AutoLeft, AutoLeft);
		chooseAutonSelector.AddObject(AutoCenter, AutoCenter);
		chooseAutonSelector.AddObject(AutoRight, AutoRight);
		frc::SmartDashboard::PutData("Auto Selector", &chooseAutonSelector);

		chooseAutoDelay.AddDefault(AutoDelayOff, AutoDelayOff);
		chooseAutoDelay.AddObject(AutoDelay1, AutoDelay1);
		chooseAutoDelay.AddObject(AutoDelay2, AutoDelay2);
		frc::SmartDashboard::PutData("Auto Delay",&chooseAutoDelay);

		chooseDriveEncoder.AddDefault(LH_Encoder, LH_Encoder);
		chooseDriveEncoder.AddObject(RH_Encoder, RH_Encoder);
		frc::SmartDashboard::PutData("Encoder", &chooseDriveEncoder);

		chooseLowTurnSens.AddDefault(TurnDefault, TurnDefault);
		chooseLowTurnSens.AddObject(Turn1, Turn1);
		chooseLowTurnSens.AddObject(Turn2, Turn2);
		chooseLowTurnSens.AddObject(Turn3, Turn3);
		frc::SmartDashboard::PutData("LowTurnSens", &chooseLowTurnSens);

		chooseLowDriveSens.AddDefault(DriveDefault, DriveDefault);
		chooseLowDriveSens.AddObject(Drive1, Drive1);
		chooseLowDriveSens.AddObject(Drive2, Drive2);
		chooseLowDriveSens.AddObject(Drive3, Drive3);
		frc::SmartDashboard::PutData("LowDriveSens", &chooseLowDriveSens);

		chooseHighTurnSens.AddDefault(TurnDefault, TurnDefault);
		chooseHighTurnSens.AddObject(Turn1, Turn1);
		chooseHighTurnSens.AddObject(Turn2, Turn2);
		chooseHighTurnSens.AddObject(Turn3, Turn3);
		frc::SmartDashboard::PutData("HighTurnSens", &chooseHighTurnSens);

		chooseHighDriveSens.AddDefault(DriveDefault, DriveDefault);
		chooseHighDriveSens.AddObject(Drive1, Drive1);
		chooseHighDriveSens.AddObject(Drive2, Drive2);
		chooseHighDriveSens.AddObject(Drive3, Drive3);
		frc::SmartDashboard::PutData("HighDriveSens", &chooseHighDriveSens);

		//turn off shifter solenoids
		driveSolenoid->Set(false);

		//disable drive watchdogs
		Adrive.SetSafetyEnabled(false);

		//changes these original negative values to positive values
		EncoderLeft.SetReverseDirection(true);
		EncoderRight.SetReverseDirection(false);

		//calibrations for encoders
		EncoderLeft.SetDistancePerPulse(98.0 / 3125.0 * 4.0);
		EncoderRight.SetDistancePerPulse(98.0 / 3125.0 * 4.0);

		//drive command averaging filter
		OutputX = 0, OutputY = 0;

		//variable that chooses which encoder robot is reading for autonomous mode
		useRightEncoder = true;
	}

	void AutonomousInit() override {
		modeState = 1;
		isWaiting = 0;	/////***** Rename this.
		AutoDelayActive = false;

		AutonTimer.Reset();
		AutonTimer.Start();
		EncoderCheckTimer.Reset();
		EncoderCheckTimer.Start();

		// Encoder based auton
		resetEncoder();

		// Turn off drive motors
		DriveLeft0.Set(0);
		DriveLeft1.Set(0);
		DriveLeft2.Set(0);
		DriveRight0.Set(0);
		DriveRight1.Set(0);
		DriveRight2.Set(0);

		//zeros the navX
		if (ahrs) {
			ahrs->ZeroYaw();
		}

		//forces robot into low gear
		driveSolenoid->Set(false);

		//Read switch and scale game data
		gameData = frc::DriverStation::GetInstance().GetGameSpecificMessage();


	}

	void TeleopInit() {
		OutputX = 0, OutputY = 0;

	}

	void DisabledInit() {

	}

	void RobotPeriodic() {
		//links multiple motors together
		DriveLeft1.Set(DriveLeft0.Get());
		DriveLeft2.Set(DriveLeft0.Get());
		DriveRight1.Set(DriveRight0.Get());
		DriveRight2.Set(DriveRight0.Get());

		// Encoder Selection for autotools
		encoderSelected = chooseDriveEncoder.GetSelected();
		useRightEncoder = (encoderSelected == RH_Encoder);

		// Select Auto Program
		autoSelected = chooseAutonSelector.GetSelected();
		autoDelay = chooseAutoDelay.GetSelected();

		LowTurnChooser = chooseLowTurnSens.GetSelected();
		LowDriveChooser = chooseLowDriveSens.GetSelected();
		HighTurnChooser = chooseHighTurnSens.GetSelected();
		HighDriveChooser = chooseHighDriveSens.GetSelected();
	}

	void DisabledPeriodic() {

	}

#define NoAuto 0
#define SpotLeft 1
#define SpotCenter 2
#define SpotRight 3
#define caseLeft 1
#define caseRight 2
	void AutonomousPeriodic() {

		// Set near switch game state
		if(gameData[0] == 'L')
			NearSwitch = caseLeft;
		 else
			NearSwitch = caseRight;
		// Set far switch game state
		if(gameData[2] == 'L')
			ScaleState = caseLeft;
		 else
			ScaleState = caseRight;
		// Set Scale game state
		if(gameData[3] == 'L')
			FarSwitch = caseLeft;
		 else
			FarSwitch = caseRight;

		if(autoSelected==AutoLeft)
			AutoSpot=SpotLeft;
		else if (autoSelected==AutoCenter)
			AutoSpot=SpotCenter;
		else if (autoSelected==AutoRight)
			AutoSpot=SpotRight;
		else
			AutoSpot=NoAuto;

		if (autoDelay == AutoDelay1 and AutonTimer.Get()< 3){
			AutoDelayActive = true;
		}
		switch(AutoSpot);
		case SpotLeft:

			break;
		case SpotCenter:

			break;
		case SpotRight:

			break;
		default:


		autoForward();
	}

#define caseDriveDefault 1
#define caseDrive1 2
#define caseDrive2 3
#define caseDrive3 4

	void TeleopPeriodic() {
		double Control_Deadband = 0.10;
		double Drive_Deadband = 0.10;
		double Gain = 1;
		double DPadSpeed = 1.0;
		bool RightStickLimit1 = DiIn8.Get();
		bool RightStickLimit2 = DiIn9.Get();
		std::string DriveDebug = "";
		std::string TurnDebug = "";

		//high gear & low gear controls
		if (Drivestick.GetRawButton(6))
			driveSolenoid->Set(true);			// High gear press LH bumper
		if (Drivestick.GetRawButton(5))
			driveSolenoid->Set(false);			// Low gear press RH bumper

		//  Rumble code
		//  Read all motor current from PDP and display on drivers station
		double driveCurrent = pdp->GetTotalCurrent();	// Get total current

		// rumble if current to high
		double LHThr = 0.0;		// Define value for rumble
		if (driveCurrent > 125.0)// Rumble if greater than 125 amps motor current
			LHThr = 0.5;
		Joystick::RumbleType Vibrate;				// define Vibrate variable
		Vibrate = Joystick::kLeftRumble;		// set Vibrate to Left
		Drivestick.SetRumble(Vibrate, LHThr);  	// Set Left Rumble to RH Trigger
		Vibrate = Joystick::kRightRumble;		// set vibrate to Right
		Drivestick.SetRumble(Vibrate, LHThr);// Set Right Rumble to RH Trigger

		//drive controls
		double SpeedLinear = Drivestick.GetRawAxis(1) * 1; // get Yaxis(Left stick) value (forward)
		double SpeedRotate = Drivestick.GetRawAxis(4) * 1; // get Xaxis (right stick) value (turn)

		SmartDashboard::PutNumber("YJoystick", SpeedLinear);
		SmartDashboard::PutNumber("XJoystick", SpeedRotate);

		// Set dead band for X and Y axis
		if (fabs(SpeedLinear) < Control_Deadband)
			SpeedLinear = 0.0;
		if (fabs(SpeedRotate) < Control_Deadband)
			SpeedRotate = 0.0;

		if (!driveSolenoid->Get()) {

			if (LowDriveChooser == DriveDefault)
				DriveState = caseDriveDefault;
			else if (LowDriveChooser == Drive1)
				DriveState = caseDrive1;
			else if (LowDriveChooser == Drive2)
				DriveState = caseDrive2;
			else if (LowDriveChooser == Drive3)
				DriveState = caseDrive3;
			else
				DriveState = 0;

			if (LowTurnChooser == TurnDefault)
				TurnState = caseDriveDefault;
			else if (LowTurnChooser == Turn1)
				TurnState = caseDrive1;
			else if (LowTurnChooser == Turn2)
				TurnState = caseDrive2;
			else if (LowTurnChooser == Turn3)
				TurnState = caseDrive3;
			else
				TurnState = 0;

		} else {

			if (HighDriveChooser == DriveDefault)
				DriveState = caseDriveDefault;
			else if (HighDriveChooser == Drive1)
				DriveState = caseDrive1;
			else if (HighDriveChooser == Drive2)
				DriveState = caseDrive2;
			else if (HighDriveChooser == Drive3)
				DriveState = caseDrive3;
			else
				DriveState = 0;

			if (HighTurnChooser == TurnDefault)
				TurnState = caseDriveDefault;
			else if (HighTurnChooser == Turn1)
				TurnState = caseDrive1;
			else if (HighTurnChooser == Turn2)
				TurnState = caseDrive2;
			else if (HighTurnChooser == Turn3)
				TurnState = caseDrive3;
			else
				TurnState = 0;
		}

		switch (DriveState) {
		case caseDriveDefault:
			// Set control to out of box functionality
			OutputY = SpeedLinear;
			DriveDebug = TurnDefault;
			break;
		case caseDrive1:
			// Set  control response curve  to square input
			DriveDebug = Turn1;
			if (SpeedLinear > Control_Deadband)
				OutputY = Drive_Deadband + (Gain * (SpeedLinear * SpeedLinear));
			else if (SpeedLinear < -Control_Deadband)
				OutputY = -Drive_Deadband
						+ (-Gain * (SpeedLinear * SpeedLinear));
			else
				OutputY = 0;

			break;
		case caseDrive2:
			// Set  control response curve  to cubed input
			DriveDebug = Turn2;
			if (SpeedLinear > Control_Deadband)
				OutputY = Drive_Deadband + (Gain * pow(SpeedLinear, 3));
			else if (SpeedLinear < -Control_Deadband)
				OutputY = -Drive_Deadband + (Gain * pow(SpeedLinear, 3));
			else
				OutputY = 0;

			break;
		case caseDrive3:
			// Set control response curve to input^5
			DriveDebug = Turn3;
			if (SpeedLinear > Control_Deadband)
				OutputY = Drive_Deadband + (Gain * pow(SpeedLinear, 5));
			else if (SpeedLinear < -Control_Deadband)
				OutputY = -Drive_Deadband + (Gain * pow(SpeedLinear, 5));
			else
				OutputY = 0;

			break;
		default:
			DriveDebug = "Not Set";
			OutputY = 0;

		}

		switch (TurnState) {
		case caseDriveDefault:
			// Set control to out of box functionality
			OutputX = SpeedRotate;
			TurnDebug = TurnDefault;
			break;
		case caseDrive1:
			// Set  control response curve  to square input
			TurnDebug = Turn1;
			if (SpeedRotate > Control_Deadband)
				OutputX = Drive_Deadband + (Gain * (SpeedRotate * SpeedRotate));
			else if (SpeedRotate < -Control_Deadband)
				OutputX = -Drive_Deadband
						+ (-Gain * (SpeedRotate * SpeedRotate));
			else
				OutputX = 0;
			break;
		case caseDrive2:
			// Set  control response curve  to cubed input
			TurnDebug = Turn2;
			if (SpeedRotate > Control_Deadband)
				OutputX = Drive_Deadband + (Gain * pow(SpeedRotate, 3));
			else if (SpeedRotate < -Control_Deadband)
				OutputX = -Drive_Deadband + (Gain * pow(SpeedRotate, 3));
			else
				OutputX = 0;
			break;
		case caseDrive3:
			// Set control response curve to input^5
			TurnDebug = Turn3;
			if (SpeedRotate > Control_Deadband)
				OutputX = Drive_Deadband + (Gain * pow(SpeedRotate, 5));
			else if (SpeedRotate < -Control_Deadband)
				OutputX = -Drive_Deadband + (Gain * pow(SpeedRotate, 5));
			else
				OutputX = 0;
			break;
		default:
			OutputX = 0;
			TurnDebug = "Not Set";
		}

		SmartDashboard::PutString("Drive response curve", DriveDebug);
		SmartDashboard::PutString("Turn response curve", TurnDebug);
		SmartDashboard::PutNumber("SpeedLinear", SpeedLinear);
		SmartDashboard::PutNumber("SpeedRotate", SpeedRotate);

		//slow down direction changes from 1 cycle to 5
		OutputY1 = (0.8 * OutputY1) + (0.2 * OutputY);
		OutputX1 = (0.8 * OutputX1) + (0.2 * OutputX);

		//drive

		SmartDashboard::PutNumber("OutputY", OutputY);
		SmartDashboard::PutNumber("OutputX", OutputX);

		if (Drivestick.GetRawButton(4)) {
			//boiler auto back up when y button pushed
			if (!driveButtonYPrev) {
				EncoderRight.Reset();
				EncoderLeft.Reset();
				//	ahrs->ZeroYaw();
				driveButtonYPrev = true;
			}
			//forward(autoBackupDistance);
		} else {
			//manual control
			driveButtonYPrev = false;
			Adrive.ArcadeDrive(OutputY1, OutputX1, false);
		}

		/*
		 * MANIP CODE
		 */

		//A Button to extend (Solenoid On)
		Abutton->Set(OperatorStick.GetRawButton(1));

		//B Button to extend (Solenoid On)
		Bbutton->Set(OperatorStick.GetRawButton(2));

		//if Left Bumper button pressed, extend (Solenoid On)
		if (OperatorStick.GetRawButton(5)) {
			intakeDeployed = true;
			IntakeButton->Set(intakeDeployed);
		}

		//else Right Bumper pressed, retract (Solenoid Off)
		else if (OperatorStick.GetRawButton(6)) {
			intakeDeployed = false;
			IntakeButton->Set(intakeDeployed);
		}
		//if 'X' button pressed, extend (Solenoid On)
		if (OperatorStick.GetRawButton(3)) {
			XYDeployed = true;
			XYbutton->Set(XYDeployed);
		}

		//else 'Y' button pressed, retract (Solenoid Off)
		else if (OperatorStick.GetRawButton(4)) {
			XYDeployed = false;
			XYbutton->Set(XYDeployed);
		}

		//dpad POV stuff
		if (OperatorStick.GetPOV(0) == 0) {
			Dpad1.Set(DPadSpeed);
			Dpad2.Set(DPadSpeed);
		} else if (OperatorStick.GetPOV(0) == 180) {
			Dpad1.Set(-DPadSpeed);
			Dpad2.Set(-DPadSpeed);
		} else {
			Dpad1.Set(0.0);
			Dpad2.Set(0.0);
		}

		double RightSpeed = OperatorStick.GetRawAxis(4) * -1; // get Xaxis value for Right Joystick

		if (fabs(RightSpeed) < Control_Deadband) {
			RightSpeed = 0.0;
		} else if (RightSpeed > Control_Deadband and !RightStickLimit1)
			RightSpeed = 0.0;
		else if (RightSpeed < Control_Deadband and !RightStickLimit2)
			RightSpeed = 0.0;

//		if (OperatorStick.GetRawAxis(2) > 0.5) {
//			RightSpeed = 1.0;
//		} else if (OperatorStick.GetRawAxis(3) > 0.5) {
//			RightSpeed = -1.0;
//		} else if (OperatorStick.GetRawAxis(4) < Control_Deadband)
//			RightSpeed = 0.0;
//
//		RightStick1.Set(RightSpeed);
//		RightStick2.Set(RightSpeed);

	}

	int forward(double targetDistance) {
		double encoderDistance = readEncoder();
		double encoderError = encoderDistance - targetDistance;
		double driveCommandLinear = encoderError * KP_LINEAR;
		//limits max drive speed
		if (driveCommandLinear > LINEAR_MAX_DRIVE_SPEED) {
			driveCommandLinear = LINEAR_MAX_DRIVE_SPEED;
		} else if (driveCommandLinear < -1 * LINEAR_MAX_DRIVE_SPEED) { /////***** "-1" is a "magic number." At least put a clear comment in here.
			driveCommandLinear = -1 * LINEAR_MAX_DRIVE_SPEED;
		}
		//gyro values that make the robot drive straight
		double gyroAngle = ahrs->GetAngle();
		double driveCommandRotation = gyroAngle * KP_ROTATION;
		//encdoer check
		if (EncoderCheckTimer.Get() > MAX_DRIVE_TIME) {
			motorSpeed(0.0, 0.0);
			DriverStation::ReportError(
					"(forward) Encoder Max Drive Time Exceeded");
		} else {
			//calculates and sets motor speeds
			motorSpeed(driveCommandLinear + driveCommandRotation,
					driveCommandLinear - driveCommandRotation);
		}
		//routine helps prevent the robot from overshooting the distance
		if (isWaiting == 0) {
			if (abs(encoderError) < LINEAR_TOLERANCE) {
				isWaiting = 1;
				AutonTimer.Reset();
			}
		}
		//timed wait
		else {
			float currentTime = AutonTimer.Get();
			if (abs(encoderError) > LINEAR_TOLERANCE) {
				isWaiting = 0;
			} else if (currentTime > LINEAR_SETTLING_TIME) {
				isWaiting = 0;
				return 1;
			}
		}
		return 0;
	}

	//need to change signs!!!
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

#define AR2_INIT 1
#define AR2_FWD 2
#define AR2_TIMED 3
#define AR2_END 4
	void autoForward(void) {
		//change to timed drive
		//puts gear on front of airship
		switch (modeState) {
		case AR2_INIT:
			// This uses state 1 for initialization.
			// This keeps the initialization and the code all in one place.
			ahrs->ZeroYaw();
			resetEncoder();
			modeState = AR2_FWD;
			break;
		case AR2_FWD:
			if (forward(RED_2_CASE2_FWD)) {
				AutonTimer.Reset();
				modeState = AR2_TIMED;
			}
			break;
		case AR2_TIMED:
			if (timedDrive(RED_2_CASE2_FWD, RED_2_CASE3_LSPEED,
			RED_2_CASE3_RSPEED)) {
				AutonTimer.Reset();
				modeState = AR2_END;
			}
			break;
		default:
			stopMotors();
		}
		return;
	}

	void motorSpeed(double leftMotor, double rightMotor) {
		DriveLeft0.Set(leftMotor * -1);
		DriveLeft1.Set(leftMotor * -1);
		DriveLeft2.Set(leftMotor * -1);
		DriveRight0.Set(rightMotor);
		DriveRight1.Set(rightMotor);
		DriveRight2.Set(rightMotor);
	}

//need to change signs!!!
	int stopMotors() {
		//sets motor speeds to zero
		motorSpeed(0, 0);
		return 1;
	}

	//------------- Start Code for Running Encoders --------------
	double readEncoder() {
		double usableEncoderData;
		double r = EncoderRight.GetDistance();
		double l = EncoderLeft.GetDistance();
		//If a encoder is disabled switch l or r to each other.
		if (l > 0) {
			usableEncoderData = fmax(r, l);
		} else if (l == 0) {
			usableEncoderData = r;
		} else {
			usableEncoderData = fmin(r, l);
		}
		return usableEncoderData;
	}

	void resetEncoder() {
		EncoderLeft.Reset();
		EncoderRight.Reset();
		EncoderCheckTimer.Reset();
	}
	//------------- End Code for Running Encoders --------------------

private:

}
;

START_ROBOT_CLASS(Robot)
