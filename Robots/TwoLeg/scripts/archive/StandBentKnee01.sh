#StandBentKnee01.sh
0,StartLogging 
#left lower leg x -9.4 //-12.1 //-8.5 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,0,-11.4,-7.4,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,5000,WAIT_STAGE)
#right lower leg x -11.0 //-13.4   //-9.3 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,0,-13.0,-9.0,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,5000,WAIT_STAGE)
#left lower leg z 3.0 //2.8 //1.8 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,2,1.0,5.0,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,5000,WAIT_STAGE)
#right lower leg z -3.4 //-3.3  //-4.0 +- 1.0
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,1,2,-5.4,-1.4,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,5000,WAIT_STAGE)
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

