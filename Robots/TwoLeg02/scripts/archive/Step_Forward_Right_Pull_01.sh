#Step_Forward_Right_Pull_01.sh
0,StartLogging
#0,StartAccelerometer(ETHACCELS_LEFT_LEG,7,POLLING)
#0,StartAccelerometer(ETHACCELS_RIGHT_LEG,7,POLLING)
#0,StartAccelerometer(ETHACCELS_TORSO,1,POLLING)
#0,StartTouchSensor(ETHACCELS_LEFT_LEG,0x3,POLLING)
#0,StartTouchSensor(ETHACCELS_RIGHT_LEG,0x3,POLLING)
#gently lean left 
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,1,50) #left ankle lean left
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,1,100) #left hip lean left
#0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,4,300) #right ankle lean left
#left foot needs to lean forward to throw robot weight forward
#lean forward here
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,-1,100) #left leg lean forward
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,1,100) #rleg lean forward
#turn left foot to push robot forward
#100,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,1,100)


#start of step
#bend knee back
100,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,6,500) 
#push right leg forward - note if torso is thrown forward- the right foot did not leave the ground-
#there was not enough leaning to the side or knee and leg turning
100,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,6,500) #200 


#notice that the right foot must be off the ground by this time
#the leg needs to start swinging before the body falls forward
#the forward leg then stops the fall forward

#right leg swings leg forward
#if the robot falls backwards- there was not enough weight thrown forward here.

#swing right knee forward - knee should be straight with upper leg, 
#and should not need to be turned back after this 
600,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,-6,300)

#now pull body forward with upper leg
900,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,-6,300)
#and push left foot forward
900,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,1,100)

#robot needs to push torso back to try and balance after step
#850,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,-4,400) 

#800,TurnMotor(ETHMOTORS_LOWER_BODY,0,-1,250) #left foot lean backward
#800,TurnMotor(ETHMOTORS_LOWER_BODY,6,1,250) #right foot lean backward
#END,StopAllTouchSensorsInterrupt
#END,StopAllAccelerometersInterrupt
1500,StopLogging
