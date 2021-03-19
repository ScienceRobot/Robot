#Step_Forward_Right_03.sh
0,StartLogging
0,StartAccelerometer(ETHACCELS_LEFT_LEG,7,POLLING)
0,StartAccelerometer(ETHACCELS_RIGHT_LEG,7,POLLING)
0,StartAccelerometer(ETHACCELS_TORSO,1,POLLING)
0,StartTouchSensor(ETHACCELS_LEFT_LEG,0x3,POLLING)
0,StartTouchSensor(ETHACCELS_RIGHT_LEG,0x3,POLLING)
#gently lean left and forward
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,1,100) #left ankle lean left
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,1,100) #left hip lean left
#0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,4,300) #right ankle lean left
#left foot needs to lean forward to throw robot weight forward
#

#right knee bends slightly (left knee does not bend back currently)- 
#bending knee will cause robot to tilt (robot) right, 
#if robot falls to the (robot) right after the right knee bends the robot
# did not shift it's weight enough before bending its rt knee
#the leg should not lift until the foot is off the ground

#bend right knee to help foot clear floor
#at 100 helped the r leg to clear
#0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,1,100) 


#lean back right
#at 300 could be late sometimes
#300,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,-4,270) #left ankle lean right
#300,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,-4,270) #right ankle lean right


#200 is too soon
#right knee bends the lower leg back slightly, the upper leg swings the right leg forward,
#and the right knee then swings the lower leg forward.
#bend right knee - note that right knee does not even need to be bent as can be seen with a human
#but it does provide a little bit of clearance
#300,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,5,400)

#bend knee back
100,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,6,200) 
#push right leg forward
100,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,6,200) 
#push left leg back- needed to counter so torso does not fall forward
100,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,-6,200) 


#turn left foot to push robot forward
#100,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,1,100)

#notice that the right foot must be off the ground by this time
#the leg needs to start swinging before the body falls forward
#the forward leg then stops the fall forward

#right leg swings leg forward
#400ms is a small step
#700ms is too far
#accel 2x reached < -20degrees in 200ms- it's a medium sized step
#300,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,4,700,ETHACCELS_RIGHT_LEG,2,0,LT,-20) 
#300,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,4,600) 
#left leg needs to thrust a little in opposite direction or else torso falls forward
#300,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,4,400) #and left leg swings backwards, pushes torso forward


#if the robot falls backwards- there was not enough weight thrown forward here.
#400ms was too much- not a lot of push forward is needed here
#at 300ms is a little early maybe
#400,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,1,200) #left foot lean forward 200ms not enough
#0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,1,200) #right foot tip pushes down (lean backward)


#swing right knee forward
300,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,-6,400)


#try to push weight back to right side
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,-1,100) #left ankle lean left
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,-1,100) #left hip lean left


#robot needs to push torso back to try and balance after step
#850,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,-4,400) 

#800,TurnMotor(ETHMOTORS_LOWER_BODY,0,-1,250) #left foot lean backward
#800,TurnMotor(ETHMOTORS_LOWER_BODY,6,1,250) #right foot lean backward
#END,StopAllTouchSensorsInterrupt
#END,StopAllAccelerometersInterrupt
1500,StopLogging
