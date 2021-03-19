#LeanLeftRightStep03.sh
0,StartLogging 

#legs spread 22cm from center to center

#stage 0, lean left
#note too that DegreesPlus is to the right viewing robot 2 degrees or less
#
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,2,Z,3,-10,0,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,1000,WAIT_STAGE)
#Turn left side motor until Z of left upper leg is between -7 and 3 (standing Z=7)
#0,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,50,1000,ETHACCELS_LEFT_LEG,2,Z,BETWEEN,3,-10,0,WAIT_STAGE)
0,TurnMotorUntilAngle(MOTOR_LEFT_SIDE,50,1000,ACCEL_LEFT_UPPER_LEG,Z,BETWEEN,3,-10,0,WAIT_STAGE)
#0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,2,Z,3,-10,0,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,1000,WAIT_STAGE)
#0,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,50,1000,ETHACCELS_LEFT_LEG,2,Z,BETWEEN,3,-10,0,WAIT_STAGE)
#0,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,50,1000,ETHACCELS_RIGHT_LEG,2,Z,BETWEEN,3,-10,0,WAIT_STAGE)
#upper right leg x
#use right leg accel as reference -14 or less
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,2,2,-10,-10,0,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,5000,WAIT_STAGE)
#0,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,1,5000,ETHACCELS_RIGHT_LEG,2,2,BETWEEN,-11,-11,0,WAIT_STAGE)

#stage 1, lift right leg
#possibly change to LOWER_LEFT_LEG_ACCEL too- so side and ankle work toward same goal
1,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,2,Z,3,-10,0,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,1000,WAIT_STAGE)

#and have left foot hold lower left leg Z vertical
1,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,1,Z,0,-2,2,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,1000,WAIT_STAGE)

#use the left side motor to hold the torso between TODO: DETERMINE (or upper left leg between Z=-7 and 3)
#turn torso in negative Z (roll)
1,HoldAngleWithMotor(ETHACCELS_TORSO,0,Z,0,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,1000,WAIT_STAGE) #WAIT_STAGE_TURN

#1,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,50,1000,ETHACCELS_TORSO,0,Z,BETWEEN,0,-10,1,WAIT_STAGE)
#use right side motor too - to hold the torso between TODO: DETERMINE 
1,HoldAngleWithMotor(ETHACCELS_TORSO,0,Z,0,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,1000,WAIT_STAGE) #WAIT_STAGE_TURN

#1,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,50,1000,ETHACCELS_TORSO,0,Z,BETWEEN,0,-10,1,WAIT_STAGE)


#lift right leg - the TurnMotorUntilAngle instructions signal the end of the stage: when they are done the HoldAngleWithMotor instructions will end if their flag is WAIT_STAGE_TURN (or WAIT_STAGE_UNTIL?) WAIT_STAGE_UNTIL_TIMEOUT, WAIT_STAGE_UNTIL_TURN_MOTOR_UNTIL_END, WAIT_STAGE_TIMEOUT or WAIT_STAGE_TURNMOTOR
#x=8.8 is normal
#right leg (standing x=13 x=15 x=19) move to between 19 and 24
1,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,100,1000,ETHACCELS_RIGHT_LEG,2,X,BETWEEN,19,0,5,WAIT_STAGE)
#right knee -13.2 regularly
1,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,100,1000,ETHACCELS_RIGHT_LEG,1,X,BETWEEN,-14,-10,0,WAIT_STAGE)
#because right leg motor swings rt leg forward, torso would fall bkwd, so Hold torso X with left leg motor (standing x=-5)
1,HoldAngleWithMotor(ETHACCELS_TORSO,0,X,-5,-2,2,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,1000,WAIT_STAGE)
#need to hold left foot Z or else foot can turn too far- hold -2 > x > 2
1,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,0,Z,0,-2,2,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,1000,WAIT_STAGE)


#put right leg back down
#right leg back (standing x=15)
2,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,50,1000,ETHACCELS_RIGHT_LEG,2,X,BETWEEN,15,-10,0,WAIT_STAGE)
#right knee back
2,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,50,1000,ETHACCELS_RIGHT_LEG,1,X,BETWEEN,-12,0,10,WAIT_STAGE)


4,StopLogging
