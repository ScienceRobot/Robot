#LeanLeft01.sh
0,StartLogging 
#HoldAngleWithMotor(EthAccelPCB,accel#,XYorZ,MinDegree,MaxDegree,?,EthMotorsPCB,MotorName,Timeout,flags)
#WAIT_STAGE= follow instruction until timeout

#legs spread 22cm from center to center

#upper left leg x - give wide DegreesMinus because otherwise will try to compensate (since accel just based on accelerometer currently)
#note too that DegreesPlus is to the right viewing robot 2 degrees or less
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,2,2,6,-10,0,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,5000,WAIT_STAGE)
#upper right leg x
#use right leg accel as reference -14 or less
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,2,2,-10,-10,0,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,5000,WAIT_STAGE)
#use left leg accel as reference
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,2,2,2,-10,0,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,5000,WAIT_STAGE)

#spread legs:
#hold lower legs X and Z with feet motors
#left lower leg x --10.7 -13.4 -8.5 -15.7 -14.5
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,0,-11.0,0,8,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,10000,WAIT_STAGE)
#right lower leg x -12.1 -13.8 -10.6 -16.3 -14.6 -17.3
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,0,-11.0,0,8,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,10000,WAIT_STAGE)
#hold lower legs Z with feet
#left lower leg z 4.9
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,2,4.9,-1,1,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,10000,WAIT_STAGE)
#right lower leg z -1.3 -2.8
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,2,-2.8,-1,1,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,10000,WAIT_STAGE)

#feet need to be last so their angles/TurnMotor commands override all the above angles/TurnMotor commands
#I give +-5 degrees because my main concern is just preventing the feet from colliding with the lower leg
#left foot x 2.2 2.3 //2.4 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,0,0,2.2,-5,5,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,5000,WAIT_STAGE)
#right foot x 4.6 5.1 /5.3 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,0,0,4.6,-5,5,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,5000,WAIT_STAGE)
#left foot z 2.5 2.1 //2.7 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,0,2,2.5,-5,5,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,5000,WAIT_STAGE)
#right foot z 0.4 1.2 -0.5 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,0,2,0.4,-5,5,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,5000,WAIT_STAGE)


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

