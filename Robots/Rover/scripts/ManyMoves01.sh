#ManyMoves01.sh
#0,StartLogging 
#0,LogScriptFileName


#left arm left
0,TurnMotor(MOTOR_LEFT_LOWER_ARM_INSIDE,-10,1000)
0,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE,-10,1000)
#right arm down
0,TurnMotor(MOTOR_RIGHT_LOWER_ARM,-18,2000)
#turn right shoulder out
0,TurnMotor(MOTOR_RIGHT_SHOULDER,8,2000)
#right upper arm up
0,TurnMotor(MOTOR_RIGHT_UPPER_ARM_INSIDE,18,2000)
0,TurnMotor(MOTOR_RIGHT_UPPER_ARM_OUTSIDE,18,2000)
#turn wrists
0,TurnMotor(MOTOR_LEFT_WRIST,-3,1000)
0,TurnMotor(MOTOR_RIGHT_WRIST,-3,1000)


#turn head right
0,TurnMotor(MOTOR_NECK,-5,1000)
#turn head down
0,TurnMotor(MOTOR_HEAD,-3,1000)


#left arm right
1000,TurnMotor(MOTOR_LEFT_LOWER_ARM_INSIDE,10,1000)
1000,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE,10,1000)
#turn head left
1000,TurnMotor(MOTOR_NECK,5,1000)
#turn head down
1000,TurnMotor(MOTOR_HEAD,3,1000)
#turn wrists
1000,TurnMotor(MOTOR_LEFT_WRIST,3,1000)
1000,TurnMotor(MOTOR_RIGHT_WRIST,3,1000)

#left arm left
2000,TurnMotor(MOTOR_LEFT_LOWER_ARM_INSIDE,-10,1000)
2000,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE,-10,1000)
#right arm up
2000,TurnMotor(MOTOR_RIGHT_LOWER_ARM,18,2000)
#right upper arm up
2000,TurnMotor(MOTOR_RIGHT_UPPER_ARM_INSIDE,-10,2000)
2000,TurnMotor(MOTOR_RIGHT_UPPER_ARM_OUTSIDE,-10,2000)
#turn right shoulder in
2000,TurnMotor(MOTOR_RIGHT_SHOULDER,-5,2000)
#turn head right
2000,TurnMotor(MOTOR_NECK,-5,1000)
#turn head down
2000,TurnMotor(MOTOR_HEAD,-3,1000)

#left arm right
3000,TurnMotor(MOTOR_LEFT_LOWER_ARM_INSIDE,10,1000)
3000,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE,10,1000)
#turn head left
3000,TurnMotor(MOTOR_NECK,5,1000)
#turn head down
3000,TurnMotor(MOTOR_HEAD,3,1000)


#move fingers
4000,TurnMotor(MOTOR_LEFT_FINGER1_SEGMENT1,10,-2000)
4000,TurnMotor(MOTOR_LEFT_FINGER2_SEGMENT1,10,-2000)
4000,TurnMotor(MOTOR_LEFT_FINGER3_SEGMENT1,10,-2000)
4000,TurnMotor(MOTOR_RIGHT_FINGER1_SEGMENT1,10,-2000)
4000,TurnMotor(MOTOR_RIGHT_FINGER2_SEGMENT1,10,-2000)
4000,TurnMotor(MOTOR_RIGHT_FINGER3_SEGMENT1,10,-2000)

#right down
#0,TurnMotor(MOTOR_RIGHT_WRIST,-14,100)
#left up
#0,TurnMotor(MOTOR_LEFT_WRIST,-14,100)
#right up
#100,TurnMotor(MOTOR_RIGHT_WRIST,16,100)
#left down
#100,TurnMotor(MOTOR_LEFT_WRIST,19,100)
#200,TurnMotor(MOTOR_RIGHT_WRIST,-14,100)
#200,TurnMotor(MOTOR_LEFT_WRIST,-14,100)
#300,TurnMotor(MOTOR_RIGHT_WRIST,16,100)
#300,TurnMotor(MOTOR_LEFT_WRIST,19,100)
#400,TurnMotor(MOTOR_RIGHT_WRIST,-14,100)
#400,TurnMotor(MOTOR_LEFT_WRIST,-14,100)
#500,TurnMotor(MOTOR_RIGHT_WRIST,16,100)
#500,TurnMotor(MOTOR_LEFT_WRIST,19,100)
#600,TurnMotor(MOTOR_RIGHT_WRIST,-14,100)
#600,TurnMotor(MOTOR_LEFT_WRIST,-14,100)
#700,TurnMotor(MOTOR_RIGHT_WRIST,16,100)
#700,TurnMotor(MOTOR_LEFT_WRIST,19,100)
#800,TurnMotor(MOTOR_RIGHT_WRIST,-14,100)
#800,TurnMotor(MOTOR_LEFT_WRIST,-14,100)
#900,TurnMotor(MOTOR_RIGHT_WRIST,16,100)
#900,TurnMotor(MOTOR_LEFT_WRIST,19,100)
#2000,TurnMotor(MOTOR_LEFT_WHEEL,3,1000)
#2000,TurnMotor(MOTOR_RIGHT_WHEEL,3,1000)
6000,StopLogging
