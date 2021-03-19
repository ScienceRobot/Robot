#LeanRight02b.sh
0,StartLogging 
#StartTimeInMS,HoldAngleWithMotor(EthAccelsPCBName,AccelNum,AccelAngleXYorZ,Angle1,Angle2,EthMotorsPCBName,MotorName/Num,Dir*Strength,Duration/TimeOut,flags)
#  flags=WAIT_STAGE - only execute instructions with the same start time (stage), and hold all later instructions.
#	      WAIT_ALL   - do not execute any other instructions (with the same start time on later script lines) until this instruction is done.

#legs spread 22cm from left foot to right foot center

#upper left leg x - give wide DegreesMinus because otherwise will try to compensate (since accel just based on accelerometer currently)
#note too that DegreesPlus is to the right viewing robot 2 degrees or less
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,2,2,6,-10,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,5000,WAIT_STAGE)
#upper right leg x
#use right leg accel as reference -14 or less
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,2,2,-10,-10,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,5000,WAIT_STAGE)
#use left leg accel as reference
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,2,2,2,-10,0,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,5000,WAIT_STAGE)

0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,-1,500)
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,-1,500)


#SIDE MOTORS (LOWER LEGS)
#hold lower legs Z with side motors
#left lower leg z (left/right)
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,2,-6,-2,2,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,5000,WAIT_STAGE)
#right lower leg z 
#lower right leg Z of -9.0 is accurate for leaning left
#w/o braking -8,-1,1
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,2,-6,-2,2,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,5000,WAIT_STAGE)
500,StopLogging

