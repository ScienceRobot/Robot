#LowerLegs01.sh
0,StartLogging 

#LOWER LEG X
#non-spread legs
#left lower leg x -9.5 -12.6
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,0,-12.0,0,8,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,10000,WAIT_STAGE)
#right lower leg x -9.5 -12.2
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,0,-12.0,0,8,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,10000,WAIT_STAGE)
#spread legs
#note to extend the positive side of the range up to 0, because- pulsing the lower leg down take only a little force
#the lower leg being too positive is less common than being too negative 
#left lower leg x -10.4 -14.7 -10.7 -13.4 -8.5 -15.7 -14.5
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,0,-11.0,0,11,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,15000,WAIT_STAGE)
#right lower leg x -10.3 -15.5 -12.1 -13.8 -10.6 -16.3 -14.6 -17.3
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,0,-11.0,0,11,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,15000,WAIT_STAGE)

#LOWER LEG Z
#non-spread legs
#left lower leg z 0.5 0.6
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,2,0.5,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,15000,WAIT_STAGE)
#right lower leg z -3.4 -3.9 
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,2,-3.4,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,15000,WAIT_STAGE)
#spread legs
#left lower leg z 5.9 4.9
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,2,5.9,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,10000,WAIT_STAGE)
#right lower leg z -9.8 
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,2,-9.8,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,10000,WAIT_STAGE)

#feet need to be last so their angles/TurnMotor commands override all the above angles/TurnMotor commands
#I give +-5 degrees because my main concern is just preventing the feet from colliding with the lower leg
#left foot x 2.2 2.3 //2.4 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,0,0,2.2,-5,5,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,15000,WAIT_STAGE)
#right foot x 4.6 5.1 /5.3 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,0,0,4.6,-5,5,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,15000,WAIT_STAGE)
#left foot z 2.5 2.1 //2.7 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,0,2,2.5,-5,5,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,15000,WAIT_STAGE)
#right foot z 0.4 1.2 -0.5 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,0,2,0.4,-5,5,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,15000,WAIT_STAGE)


#Knee
#left upper leg x -9.6
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,2,0,-9.6,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,10000,WAIT_STAGE)
#right upper leg x -8.9
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,2,0,-8.9,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,10000,WAIT_STAGE)



#torso x -6.6 //-3.6  //-5.7 +-1
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,0,-8.6,-4.6,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,5000,WAIT_STAGE)
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,0,-8.6,-4.6,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,5000,WAIT_STAGE)
#torso z 1.6 //-0.1  //-0.3 +-1
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,2,-0.6,3.6,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,5000,WAIT_STAGE)
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,2,-0.6,3.6,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,5000,WAIT_STAGE)

#upper left leg x 9.0 //13.3 +-1
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,2,0,12.3,14.3,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_KNEE,5000,WAIT_STAGE)
#upper right leg x 2.1  //7.7 +-1 
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,2,0,6.7,8.7,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,5000,WAIT_STAGE)



#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,2,2,5,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,10000,WAIT_STAGE)
#0,HoldAngleWithMotor(ETHACCELS_TORSO,0,2,2,5,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,10000,WAIT_STAGE)
#0,HoldAngleWithMotor(ETHACCELS_TORSO,1,0,-42,-38,ETHMOTORS_UPPER_BODY,MOTOR_HEAD,1000,WAIT_STAGE)
#1,HoldAngleWithMotor(ETHACCELS_TORSO,1,0,-10,-7,ETHMOTORS_UPPER_BODY,MOTOR_HEAD,1000,WAIT_STAGE)
#2,HoldAngleWithMotor(ETHACCELS_TORSO,1,0,-42,-38,ETHMOTORS_UPPER_BODY,MOTOR_HEAD,1000,WAIT_STAGE)
#3,HoldAngleWithMotor(ETHACCELS_TORSO,1,0,-10,-7,ETHMOTORS_UPPER_BODY,MOTOR_HEAD,1000,WAIT_STAGE)
4,StopLogging
