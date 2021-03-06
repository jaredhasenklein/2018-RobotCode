#include "RJ_RobotMap.h"

RJ_RobotMap::RJ_RobotMap() {

	//
	// Drive Base
	//

	// Set Motor directions
	DriveBase.MotorsLeft.SetInverted(false);
	DriveBase.MotorsRight.SetInverted(false);

	DriveBase.ClawIntake1.SetInverted(false);
	DriveBase.ClawIntake2.SetInverted(true);

	//Set Elevator Motor directions & Scale
	DriveBase.Elevator1.SetInverted(true);
	DriveBase.Elevator2.SetInverted(false);
	DriveBase.EncoderElevator.SetDistancePerPulse((47.5) / -10197); //move one inch and measure pulses

	DriveBase.Winch1.SetInverted(true);
	DriveBase.Winch2.SetInverted(false);

	// Set Encoder Direction & Scale
	DriveBase.EncoderLeft.SetReverseDirection(true);
	DriveBase.EncoderRight.SetReverseDirection(false);

	// Practice Bot
	DriveBase.EncoderLeft.SetDistancePerPulse(167.0 / 16745.0 * 4.0);
	DriveBase.EncoderRight.SetDistancePerPulse(-167.0 / 16745.0 * 4.0);

	// Comp Bot
	//DriveBase.EncoderLeft.SetDistancePerPulse(48.0 / -1523.0 * 4.0);
	//DriveBase.EncoderRight.SetDistancePerPulse(48.0 / -1523.0 * 4.0);

	// Set Default Gear
	DriveBase.SolenoidShifter.Set(false);

	// Invert Wrist motor
	DriveBase.Wrist1.SetInverted(false);

	//
	// Smart Dashboard
	//

	// Auto Program Chooser
	DS.chooseAutoProgram.AddDefault(DS.AutoSwitch, DS.AutoSwitch);
	DS.chooseAutoProgram.AddObject(DS.AutoScale, DS.AutoScale);
	DS.chooseAutoProgram.AddObject(DS.AutoLine, DS.AutoLine);
	DS.chooseAutoProgram.AddObject(DS.AutoNone, DS.AutoNone);
	DS.chooseAutoProgram.AddObject(DS.AutoNearSide, DS.AutoNearSide);
	DS.chooseAutoProgram.AddObject(DS.AutoNscNswFsc, DS.AutoNscNswFsc);
	frc::SmartDashboard::PutData("Auto OBJ", &DS.chooseAutoProgram);

	// Auto Position Chooser
	DS.chooseAutoPosStart.AddDefault(DS.sAutoCenter, DS.sAutoCenter);
	DS.chooseAutoPosStart.AddObject(DS.sAutoLeft, DS.sAutoLeft);
	DS.chooseAutoPosStart.AddObject(DS.sAutoRight, DS.sAutoRight);
	frc::SmartDashboard::PutData("Auto Pos", &DS.chooseAutoPosStart);

	// Auto Finisher Chooser
	DS.chooseAutoFinisher.AddDefault(DS.sAuto2Cube, DS.sAuto2Cube);
	DS.chooseAutoFinisher.AddObject(DS.sAutoNo, DS.sAutoNo);
	DS.chooseAutoFinisher.AddObject(DS.sAutoWallHug, DS.sAutoWallHug);
	frc::SmartDashboard::PutData("Auto Finisher", &DS.chooseAutoFinisher);

	// Auto Game Data Overide
	DS.chooseAutoGameData.AddDefault(DS.sGameDataOff, DS.sGameDataOff);
	DS.chooseAutoGameData.AddObject(DS.sGameDataLefts, DS.sGameDataLefts);
	DS.chooseAutoGameData.AddObject(DS.sGameDataRights, DS.sGameDataRights);
	frc::SmartDashboard::PutData("Auto Game Data", & DS.chooseAutoGameData);

	// Auto Program Delay
	DS.chooseAutoDelay.AddDefault(DS.sAutoDelayOff, DS.sAutoDelayOff);
	DS.chooseAutoDelay.AddObject(DS.sAutoDelay3, DS.sAutoDelay3);
	DS.chooseAutoDelay.AddObject(DS.sAutoDelay5, DS.sAutoDelay5);
	frc::SmartDashboard::PutData("Auto Delay", &DS.chooseAutoDelay);

	// Encoder Selection
	DS.chooseAutoEncoder.AddDefault(DS.EncoderAuto, DS.EncoderAuto);
	DS.chooseAutoEncoder.AddObject(DS.EncoderNone, DS.EncoderNone);
	DS.chooseAutoEncoder.AddObject(DS.EncoderLeft, DS.EncoderLeft);
	DS.chooseAutoEncoder.AddObject(DS.EncoderRight, DS.EncoderRight);
	DS.chooseAutoEncoder.AddObject(DS.EncoderBoth, DS.EncoderBoth);
	frc::SmartDashboard::PutData("Drive Encoder", &DS.chooseAutoEncoder);

	// Is the pot enabled
	DS.choosePotDisabled.AddDefault(DS.EnabledPOT, DS.EnabledPOT);
	DS.choosePotDisabled.AddObject(DS.DisabledPOT, DS.DisabledPOT);
	frc::SmartDashboard::PutData("POT Enabled?", &DS.choosePotDisabled);

	//
	// Vision System
	//
	/*
	 Vision.cam0.SetFPS(30);
	 Vision.cam0.SetResolution(160, 120);
	 Vision.cam0.SetExposureAuto();
	 Vision.cam0.SetWhiteBalanceAuto();

	 Vision.cam1.SetFPS(30);
	 Vision.cam1.SetResolution(160, 120);
	 Vision.cam1.SetExposureAuto();
	 Vision.cam1.SetWhiteBalanceAuto();
	 */

}

void RJ_RobotMap::NavXDebugger() {
	//if ( !DriveBase.ahrs ) return;

	bool reset_yaw_button_pressed = false;
	if (reset_yaw_button_pressed) {
		DriveBase.ahrs.ZeroYaw();
	}

	//printf("IMU_Yaw: %5.2f \n",  DriveBase.ahrs.GetYaw());

	SmartDashboard::PutBoolean("IMU_Connected", DriveBase.ahrs.IsConnected());
	SmartDashboard::PutNumber("IMU_Yaw", DriveBase.ahrs.GetYaw());
	SmartDashboard::PutNumber("IMU_Pitch", DriveBase.ahrs.GetPitch());
	SmartDashboard::PutNumber("IMU_Roll", DriveBase.ahrs.GetRoll());
	SmartDashboard::PutNumber("IMU_CompassHeading", DriveBase.ahrs.GetCompassHeading());
	SmartDashboard::PutNumber("IMU_Update_Count", DriveBase.ahrs.GetUpdateCount());
	SmartDashboard::PutNumber("IMU_Byte_Count", DriveBase.ahrs.GetByteCount());

	/* These functions are compatible w/the WPI Gyro Class */
	SmartDashboard::PutNumber("IMU_TotalYaw", DriveBase.ahrs.GetAngle());
	SmartDashboard::PutNumber("IMU_YawRateDPS", DriveBase.ahrs.GetRate());

	SmartDashboard::PutNumber("IMU_Accel_X", DriveBase.ahrs.GetWorldLinearAccelX());
	SmartDashboard::PutNumber("IMU_Accel_Y", DriveBase.ahrs.GetWorldLinearAccelY());
	SmartDashboard::PutBoolean("IMU_IsMoving", DriveBase.ahrs.IsMoving());
	SmartDashboard::PutNumber("IMU_Temp_C", DriveBase.ahrs.GetTempC());
	SmartDashboard::PutBoolean("IMU_IsCalibrating", DriveBase.ahrs.IsCalibrating());

	SmartDashboard::PutNumber("Velocity_X", DriveBase.ahrs.GetVelocityX());
	SmartDashboard::PutNumber("Velocity_Y", DriveBase.ahrs.GetVelocityY());
	SmartDashboard::PutNumber("Displacement_X", DriveBase.ahrs.GetDisplacementX());
	SmartDashboard::PutNumber("Displacement_Y", DriveBase.ahrs.GetDisplacementY());

	/* Display Raw Gyro/Accelerometer/Magnetometer Values                       */
	/* NOTE:  These values are not normally necessary, but are made available   */
	/* for advanced users.  Before using this data, please consider whether     */
	/* the processed data (see above) will suit your needs.                     */

	SmartDashboard::PutNumber("RawGyro_X", DriveBase.ahrs.GetRawGyroX());
	SmartDashboard::PutNumber("RawGyro_Y", DriveBase.ahrs.GetRawGyroY());
	SmartDashboard::PutNumber("RawGyro_Z", DriveBase.ahrs.GetRawGyroZ());
	SmartDashboard::PutNumber("RawAccel_X", DriveBase.ahrs.GetRawAccelX());
	SmartDashboard::PutNumber("RawAccel_Y", DriveBase.ahrs.GetRawAccelY());
	SmartDashboard::PutNumber("RawAccel_Z", DriveBase.ahrs.GetRawAccelZ());
	SmartDashboard::PutNumber("RawMag_X", DriveBase.ahrs.GetRawMagX());
	SmartDashboard::PutNumber("RawMag_Y", DriveBase.ahrs.GetRawMagY());
	SmartDashboard::PutNumber("RawMag_Z", DriveBase.ahrs.GetRawMagZ());
	SmartDashboard::PutNumber("IMU_Temp_C", DriveBase.ahrs.GetTempC());
	/* Omnimount Yaw Axis Information                                           */
	/* For more info, see http://navx-mxp.kauailabs.com/installation/omnimount  */
	AHRS::BoardYawAxis yaw_axis = DriveBase.ahrs.GetBoardYawAxis();
	SmartDashboard::PutString("YawAxisDirection", yaw_axis.up ? "Up" : "Down");
	SmartDashboard::PutNumber("YawAxis", yaw_axis.board_axis);

	/* Sensor Board Information                                                 */
	SmartDashboard::PutString("FirmwareVersion", DriveBase.ahrs.GetFirmwareVersion());

	/* Quaternion Data                                                          */
	/* Quaternions are fascinating, and are the most compact representation of  */
	/* orientation data.  All of the Yaw, Pitch and Roll Values can be derived  */
	/* from the Quaternions.  If interested in motion processing, knowledge of  */
	/* Quaternions is highly recommended.                                       */
	SmartDashboard::PutNumber("QuaternionW", DriveBase.ahrs.GetQuaternionW());
	SmartDashboard::PutNumber("QuaternionX", DriveBase.ahrs.GetQuaternionX());
	SmartDashboard::PutNumber("QuaternionY", DriveBase.ahrs.GetQuaternionY());
	SmartDashboard::PutNumber("QuaternionZ", DriveBase.ahrs.GetQuaternionZ());

}

