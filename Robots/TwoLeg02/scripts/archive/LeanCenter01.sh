#LeanCenter01.sh
0,StartLogging 

#legs spread 22cm from center to center

#upper left leg x - give wide DegreesPlus because otherwise will try to compensate (since accel just based on accelerometer currently)
#note too that DegreesMinus is to the right viewing robot
#use left leg as reference 7 
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,2,2,7,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,5000,WAIT_STAGE)
#use right leg as reference
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,2,2,-6,0,10,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,10000,WAIT_STAGE)
#upper right leg x -8
0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,2,2,-8,-3,3,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,5000,WAIT_STAGE)


4,StopLogging
