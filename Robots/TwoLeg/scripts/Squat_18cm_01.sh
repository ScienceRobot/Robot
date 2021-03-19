#Squat_18cm_01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#presumes legs are 18cm apart
#lower legs are around -16 degrees
#(upper legs are around 9 degrees)
#torso at -3,-2

#hold lower legs X with feet motors (-16), alt: use CurrentAngle  (if rt=-9 it will send body back, and torso will have to lean forward)
#0,MotorAccel(MOTOR_LEFT_FOOT,1,4000,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle,-2,2,WAIT_STAGE)
#0,MotorAccel(MOTOR_RIGHT_FOOT,1,4000,ACCEL_RIGHT_LOWER_LEG,X,CurrentAngle,-2,2,WAIT_STAGE)
0,MotorAccel(MOTOR_LEFT_FOOT,1,4000,ACCEL_LEFT_FOOT,X,0,-3,3,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle[LLLX],-2,2,WAIT_STAGE|HOLD_ANGLE)
0,MotorAccel(MOTOR_RIGHT_FOOT,1,4000,ACCEL_RIGHT_FOOT,X,0,-3,3,ACCEL_RIGHT_LOWER_LEG,X,CurrentAngle[RLLX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#0,HoldAngleWithMotor(ACCEL_LEFT_LOWER_LEG,X,-16,-0.5,0.5,MOTOR_LEFT_FOOT,1,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,X,-16,-0.5,0.5,MOTOR_RIGHT_FOOT,1,4000,WAIT_STAGE)
#hold feet X with feet motors (0) (higher priority)
#0,MotorAccel(MOTOR_LEFT_FOOT,1,4000,ACCEL_LEFT_FOOT,X,0,-3,3,WAIT_STAGE|HOLD_ANGLE)
#0,MotorAccel(MOTOR_RIGHT_FOOT,1,4000,ACCEL_RIGHT_FOOT,X,0,-3,3,WAIT_STAGE|HOLD_ANGLE)


#(this is the actual squat, the rest is balance): move upper legs down to with knee motors (from 17 to 21)
#Sync both accels together to within 1.0 degree of separation
#0,MotorAccel(MOTOR_LEFT_KNEE,5,4000,ACCEL_LEFT_UPPER_LEG,X,32,-2,2,ACCEL_RIGHT_UPPER_LEG,X,32,-2,2,WAIT_STAGE|SYNC_MAX_SPREAD=1.0)
#0,MotorAccel(MOTOR_RIGHT_KNEE,5,4000,ACCEL_RIGHT_UPPER_LEG,X,32,-2,2,ACCEL_LEFT_UPPER_LEG,X,32,-2,2,WAIT_STAGE|SYNC_MAX_SPREAD=1.0)
0,MotorAccel(MOTOR_LEFT_KNEE,5,4000,ACCEL_LEFT_UPPER_LEG,X,32,-2,2,WAIT_STAGE)
0,MotorAccel(MOTOR_RIGHT_KNEE,5,4000,ACCEL_RIGHT_UPPER_LEG,X,32,-2,2,WAIT_STAGE)

#0,HoldAngleWithMotor(ACCEL_LEFT_UPPER_LEG,X,ACCEL_RIGHT_UPPER_LEG,X,1.0,32,-2,2,MOTOR_LEFT_KNEE,5,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_RIGHT_UPPER_LEG,X,ACCEL_LEFT_UPPER_LEG,X,1.0,32,-2,2,MOTOR_RIGHT_KNEE,5,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_LEFT_UPPER_LEG,X,32,-2,2,MOTOR_LEFT_KNEE,5,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_RIGHT_UPPER_LEG,X,32,-2,2,MOTOR_RIGHT_KNEE,5,4000,WAIT_STAGE)



#hold a relative angle between upper and lower legs X with knee motors (-27 degrees), 
#0,HoldRelativeAngleWithMotor(ACCEL_LEFT_LOWER_LEG,X,ACCEL_LEFT_UPPER_LEG,X,-27,-2,2,MOTOR_LEFT_KNEE,5,4000,WAIT_STAGE)
#0,HoldRelativeAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,X,ACCEL_RIGHT_UPPER_LEG,X,-27,-2,2,MOTOR_RIGHT_KNEE,5,4000,WAIT_STAGE)
#hold upper legs X with knee motors (19), 
#0,HoldAngleWithMotor(ACCEL_LEFT_UPPER_LEG,X,19,-2,2,MOTOR_LEFT_KNEE,5,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_RIGHT_UPPER_LEG,X,19,-2,2,MOTOR_RIGHT_KNEE,5,4000,WAIT_STAGE)
#hold torso X with knee motors (-6),  (higher priority)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,-6,-2,2,MOTOR_LEFT_KNEE,5,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,-6,-2,2,MOTOR_RIGHT_KNEE,5,4000,WAIT_STAGE)



#hold torso X with leg motors (-5 - goes from -4 to -6), 
0,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,4000,ACCEL_TORSO,X,CurrentAngle,-3,3,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#0,MotorAccel(MOTOR_RIGHT_LEG,1,4000,ACCEL_TORSO,X,CurrentAngle,-3,3,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-3,3,MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,-5,-3,3,MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,-3,-2,2,MOTOR_LEFT_LEG,1,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,-3,-2,2,MOTOR_RIGHT_LEG,1,4000,WAIT_STAGE)


#hold torso Z with side motors (0)  - note torso z can be zero- but it requires the upper legs to have equal X and Z
#0,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-3,3,MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,5,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,Z,-2,-3,3,MOTOR_LEFT_SIDE,5,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,Z,-2,-3,3,MOTOR_RIGHT_SIDE,5,4000,WAIT_STAGE)


#hold lower leg Z with ankle motors (3.5 and -3.5), - note that this presumes that legs are 18cm apart (legs slightly spread apart)
#0,HoldAngleWithMotor(ACCEL_LEFT_LOWER_LEG,Z,3.5,-1,1,MOTOR_LEFT_ANKLE,5,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,Z,-3.5,-1,1,MOTOR_RIGHT_ANKLE,5,4000,WAIT_STAGE)
#hold feet Z with ankle motors (0) (higher priority)
#0,HoldAngleWithMotor(ACCEL_LEFT_FOOT,Z,0,-3,3,MOTOR_LEFT_ANKLE,5,4000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_RIGHT_FOOT,Z,0,-3,3,MOTOR_RIGHT_ANKLE,5,4000,WAIT_STAGE)

#hold robot balanced - lasts for 4 seconds until timeout
1,MotorAccel(MOTOR_LEFT_FOOT,1,4000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,Angle[LLLX],-2,2,WAIT_STAGE|HOLD_ANGLE)
1,MotorAccel(MOTOR_RIGHT_FOOT,1,4000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,Angle[RLLX],-2,2,WAIT_STAGE|HOLD_ANGLE)
1,MotorAccel(MOTOR_LEFT_KNEE,5,4000,ACCEL_LEFT_UPPER_LEG,X,CurrentAngle,-2,2,WAIT_STAGE|HOLD_ANGLE)
1,MotorAccel(MOTOR_RIGHT_KNEE,5,4000,ACCEL_RIGHT_UPPER_LEG,X,CurrentAngle,-2,2,WAIT_STAGE|HOLD_ANGLE)
1,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,4000,ACCEL_TORSO,X,CurrentAngle,-3,3,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)

4,StopLogging

