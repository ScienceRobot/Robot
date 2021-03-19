#LeanLeft02.sh
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

#SIDE MOTORS (LOWER LEGS)
#hold lower legs Z with side motors
#left lower leg z (left/right)
#0,-1,1 w/o braking code, but -4.8 is an accurate Z angle for the lower left leg leaning left
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,2,0,-1,1,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,5000,WAIT_STAGE)
#right lower leg z 
#lower right leg Z of -9.0 is accurate for leaning left
#w/o braking -8,-1,1
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,2,-8,-1,1,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,5000,WAIT_STAGE)



#FEET MOTORS (LOWER LEGS)
#hold lower legs X and Z with feet motors
#hole lower legs X with foot motor
#left lower leg x (forward/backward) //was target,Minus,Plus -11,0,8 
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,0,-11,0,4,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,5000,WAIT_STAGE)
#right lower leg x //-11,0,8
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,0,-11,0,4,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,5000,WAIT_STAGE)

#ANKLE MOTORS (LOWER LEGS)
#hold lower legs Z with foot motor (-2,-1,1 is standing straight)
#left lower leg z (left/right)  //was 4.9,-1,1
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,2,0,-1,1,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,5000,WAIT_STAGE)
#right lower leg z //was -2.8,-1,1  (-3,-1,1 is standing straight)
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,2,-8,-1,1,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,5000,WAIT_STAGE)


#FEET MOTORS (FEET)
#feet need to be last so their angles/TurnMotor commands override all the above angles/TurnMotor commands
#I give +-5 degrees because my main concern is just preventing the feet from colliding with the lower leg
#left foot x //was 2.2,-5,5
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,0,0,0.0,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,5000,WAIT_STAGE)
#right foot x //was 4.6,-5,5
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,0,0,0.0,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,5000,WAIT_STAGE)

#ANKLE MOTORS (FEET)
#left foot z //was 2.5,-5,5
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,0,2,0.0,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,5000,WAIT_STAGE)
#right foot z //was 2.5,-5,5
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,0,2,0.0,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,5000,WAIT_STAGE)


#left lower leg x -9.4 //-12.1 //-8.5 +- 1.0
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,0,-11.4,-7.4,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,3000,WAIT_STAGE)
#right lower leg x -11.0 //-13.4   //-9.3 +- 1.0
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,0,-13.0,-9.0,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,3000,WAIT_STAGE)

#LOWER LEG X
#spread legs
#note to extend the positive side of the range up to 0, because- pulsing the lower leg down take only a little force
#the lower leg being too positive is less common than being too negative 
#left lower leg x -10.7 -13.4 -8.5 -15.7 -14.5
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,0,-11.0,0,8,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,10000,WAIT_STAGE)
#right lower leg x -13.8 -10.6 -16.3 -14.6 -17.3
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,0,-11.0,0,8,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,10000,WAIT_STAGE)


#LOWER LEG Z
#spread legs
#left lower leg z 4.9
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,2,3.9,5.9,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,10000,WAIT_STAGE)
#right lower leg z -2.8
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,2,-3.8,-1.8,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,10000,WAIT_STAGE)
#close legs
#left lower leg z 2.7 0.4 3.0 2.8 1.8
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,2,1.7,3.7,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,5000,WAIT_STAGE)
#right lower leg z -3.6 -4.5 -3.4 -3.3 -4.0 
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,2,-4.6,-2.6,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,5000,WAIT_STAGE)


#torso x -6.6 //-3.6  //-5.7 +-1
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,0,-8.6,-4.6,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,5000,WAIT_STAGE)
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,0,-8.6,-4.6,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,5000,WAIT_STAGE)
#torso z 1.6 //-0.1  //-0.3 +-1
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,2,-0.6,3.6,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,5000,WAIT_STAGE)
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,2,-0.6,3.6,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,5000,WAIT_STAGE)


#feet need to be last so their angles/TurnMotor commands override all the above angles/TurnMotor commands
#give feet bigger range, perhaps 3-5 degrees ?
#left foot x 2.3 //2.4 +- 1.0
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,0,0,1.4,3.4,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,5000,WAIT_STAGE)
#right foot x 5.1  //5.3 +- 1.0
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,0,0,4.3,6.3,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,5000,WAIT_STAGE)
#left foot z 2.1 //2.7 +- 1.0
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,0,2,1.7,3.7,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,5000,WAIT_STAGE)
#right foot z 1.2 //-0.5 +- 1.0
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,0,2,-1.5,0.5,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,5000,WAIT_STAGE)


#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,2,2,5,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,10000,WAIT_STAGE)
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,2,2,5,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,10000,WAIT_STAGE)
#0,HoldAngleWithMotor(ETHACCELS_TORSO,1,0,-42,-38,ETHMOTORS_UPPER_BODY,MOTOR_HEAD,1000,WAIT_STAGE)
#1,HoldAngleWithMotor(ETHACCELS_TORSO,1,0,-10,-7,ETHMOTORS_UPPER_BODY,MOTOR_HEAD,1000,WAIT_STAGE)
#2,HoldAngleWithMotor(ETHACCELS_TORSO,1,0,-42,-38,ETHMOTORS_UPPER_BODY,MOTOR_HEAD,1000,WAIT_STAGE)
#3,HoldAngleWithMotor(ETHACCELS_TORSO,1,0,-10,-7,ETHMOTORS_UPPER_BODY,MOTOR_HEAD,1000,WAIT_STAGE)
4,StopLogging

