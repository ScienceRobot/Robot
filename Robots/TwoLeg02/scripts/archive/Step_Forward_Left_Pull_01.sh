#Step_Forward_Left_Pull_01.sh
0,StartLogging
#0,StartAccelerometer(ETHACCELS_LEFT_LEG,7,POLLING)
#0,StartAccelerometer(ETHACCELS_RIGHT_LEG,7,POLLING)
#0,StartAccelerometer(ETHACCELS_TORSO,1,POLLING)
#0,StartTouchSensor(ETHACCELS_LEFT_LEG,0x3,POLLING)
#0,StartTouchSensor(ETHACCELS_RIGHT_LEG,0x3,POLLING)
#gently lean right 
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,-1,50) #right ankle lean right
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,1,100) #right hip lean right
#0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,4,300) #right ankle lean left
#right foot needs to lean forward to throw robot weight forward
#lean forward here
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,1,100) #right leg lean forward
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,-1,100) #left leg lean forward

#bend knee back
100,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_KNEE,-6,300) 
#push leg forward
100,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,-6,300) #200 

#lean back- otherwise the robot will fall over to the left
100,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,1,50) #right ankle lean right
100,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,-1,100) #right hip lean right

#notice that the left foot must be off the ground by this time
#the leg needs to start swinging before the body falls forward
#the forward leg then stops the fall forward

#swing knee forward - knee should be straight with upper leg, 
#and should not need to be turned back after this 
400,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_KNEE,6,300)

#now pull body forward with upper leg
700,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,6,300)
#and push left foot forward
700,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,1,100)

#and push left ankle to lean robot back to right
700,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,1,50)

#robot needs to push torso back to try and balance after step
#850,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,-4,400) 

#800,TurnMotor(ETHMOTORS_LOWER_BODY,0,-1,250) #left foot lean backward
#800,TurnMotor(ETHMOTORS_LOWER_BODY,6,1,250) #right foot lean backward
#END,StopAllTouchSensorsInterrupt
#END,StopAllAccelerometersInterrupt
1500,StopLogging
