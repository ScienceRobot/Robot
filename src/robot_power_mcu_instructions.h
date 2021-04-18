//robot_power_mcu_instructions.h
/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
*/

#ifndef _ROBOT_POWER_MCU_INSTRUCTIONS_H
#define _ROBOT_POWER_MCU_INSTRUCTIONS_H

//instructions used by the PIC that controls the power on the Robot

//PCB instructions 0x00-0x0f
#define ROBOT_POWER_TEST 0x00 //send back 0x12345678
#define ROBOT_POWER_PCB_NAME 0x01 //send back Name/ID of PCB ("Power", "Motor", "Accel", etc.)
//PIC instructions 0x10-0x1f
#define ROBOT_POWER_GET_MEM 0x10  //get PIC memory value
#define ROBOT_POWER_SET_MEM 0x11  //set PIC memory value
//Power instructions 0x20-0xff
#define ROBOT_POWER_TURN_ON_3_3V 0x20  //turn on 3.3V
#define ROBOT_POWER_TURN_OFF_3_3V 0x21  //turn off 3.3V
#define ROBOT_POWER_TURN_ON_36V 0x22  //turn on 36V
#define ROBOT_POWER_TURN_OFF_36V 0x23  //turn off 36V
//
#define ROBOT_POWER_GET_ALL 0x30  //read all power settings (returns all voltage and current readings)
#define ROBOT_POWER_GET_VOLTAGE_36V 0x31  //read voltage divided resistor on 36V
#define ROBOT_POWER_GET_VOLTAGE_12V 0x32  //read voltage divided resistor on 12V
//
#define ROBOT_POWER_GET_CURRENT_36V 0x40  //read current sense resistor on 36V
#define ROBOT_POWER_GET_CURRENT_12V 0x41  //read current sense resistor on 12V
#define ROBOT_POWER_GET_CURRENT_3_3V 0x42  //read current sense resistor on 3.3V

#endif //_ROBOT_POWER_MCU_INSTRUCTIONS_H
