//robot_accelmagtouchgps_pic_instructions.h
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
//TODO: Change to ROBOT_ETHACCELS_PCB_
#ifndef _ROBOT_ACCELMAGTOUCHGPS_PIC_INSTRUCTIONS_H
#define _ROBOT_ACCELMAGTOUCHGPS_PIC_INSTRUCTIONS_H

#define DEFAULT_TIMER_INTERVAL 10;//50;//100 //ms was: 500 msec - timer 2+3 interval for accelerometer polling and all touch sensor sampling
#define DEFAULT_ACCEL_THRESHOLD 100 //063 //126 //in mg

//USB_REQ 0-12 are already defined (see usb.h)
//perhaps there should be a "vendor request" - but perhaps using adress >=0xc0 is safe
//PCB 00-0f
#define ROBOT_ACCELMAGTOUCH_TEST 0x00 //send back 0x12345678
#define ROBOT_ACCELMAGTOUCH_PCB_NAME 0x01 //send back Name/ID of PCB ("Motor00", "Motor01", "Accel00", etc.)
//PIC 10-1f
#define ROBOT_ACCELMAGTOUCH_GET_MEM 0x11  //get PIC memory value
#define ROBOT_ACCELMAGTOUCH_SET_MEM 0x12  //set PIC memory value
#define ROBOT_ACCELMAGTOUCH_GET_TIMER_INTERVAL_IN_MSEC 0x13 //get the current timer interrupt interval (in ms)
#define ROBOT_ACCELMAGTOUCH_SET_TIMER_INTERVAL_IN_MSEC 0x14  //set the timer interrupt interval (in ms)

//ACCEL 20-3f
#define ROBOT_ACCELMAGTOUCH_GET_ACCEL_REG 0x20
#define ROBOT_ACCELMAGTOUCH_SET_ACCEL_REG 0x21
#define ROBOT_ACCELMAGTOUCH_RESET_ACCELEROMETER 0x22
#define ROBOT_ACCELMAGTOUCH_GET_ACCELEROMETER_VALUES 0x23 //sends a UDP packet with a sample for each active accelerometer (note: accels need to be set active before this call)
#define ROBOT_ACCELMAGTOUCH_START_POLLING_ACCELEROMETER 0x24 //sends a UDP packet when there is a large enough change in acceleration
#define ROBOT_ACCELMAGTOUCH_STOP_POLLING_ACCELEROMETER 0x25
#define ROBOT_ACCELMAGTOUCH_START_ACCELEROMETER_INTERRUPT 0x26
#define ROBOT_ACCELMAGTOUCH_STOP_ACCELEROMETER_INTERRUPT 0x27
#define ROBOT_ACCELMAGTOUCH_GET_ACCELEROMETER_INTERRUPT_THRESHOLD 0x28 //get accelerometer threshold- how much change in acceleration (in mg) until an interrupt (and UDP packet)is sent
#define ROBOT_ACCELMAGTOUCH_SET_ACCELEROMETER_INTERRUPT_THRESHOLD 0x29 //set accelerometer threshold
#define ROBOT_ACCELMAGTOUCH_ENABLE_ACCELMAG_AUTOCALIBRATION 0x2a  //enable autocalibration on the accelerometer+magnetometer
#define ROBOT_ACCELMAGTOUCH_DISABLE_ACCELMAG_AUTOCALIBRATION 0x2b  //enable autocalibration on the accelerometer+magnetometer
#define ROBOT_ACCELMAGTOUCH_GET_HARD_IRON_OFFSET 0x2c  //get the hard iron offset the magnetometer is using
#define ROBOT_ACCELMAGTOUCH_SET_HARD_IRON_OFFSET 0x2d  //set the hard iron offset the magnetometer will use

//TOUCH 40-5f
#define ROBOT_ACCELMAGTOUCH_ENABLE_ANALOG_SENSORS 0x40  //enable the ADC interrupt
#define ROBOT_ACCELMAGTOUCH_DISABLE_ANALOG_SENSORS 0x41  //disable the ADC interrupt
#define ROBOT_ACCELMAGTOUCH_GET_ANALOG_SENSOR_VALUES 0x42  //read the voltage on touch sensors (0-3ff=0-3.3v)
#define ROBOT_ACCELMAGTOUCH_START_POLLING_ANALOG_SENSORS 0x43  //enable the timer interrupt, send sample every 100ms
#define ROBOT_ACCELMAGTOUCH_STOP_POLLING_ANALOG_SENSORS 0x44  //disable the timer interrupt
#define ROBOT_ACCELMAGTOUCH_START_ANALOG_SENSORS_INTERRUPT 0x45  //enable the ADC interrupt, send sample whenever large change (depending on threshold) occurs
#define ROBOT_ACCELMAGTOUCH_STOP_ANALOG_SENSORS_INTERRUPT 0x46  //disable the ADC interrupt
#define ROBOT_ACCELMAGTOUCH_GET_ANALOG_SENSOR_THRESHOLD 0x47 //get the threshold for 1 or more touch or pot sensor
#define ROBOT_ACCELMAGTOUCH_SET_ANALOG_SENSOR_THRESHOLD 0x48 //set the threshold for 1 or more touch or pot sensor (.1v=
//(1 bit=0.003225806v, .1v=31 0x1f)
#define ROBOT_ACCELMAGTOUCH_GET_ANALOG_MINMAX 0x4b //get the min and max Voltage for 1 or more touch or pot sensors
#define ROBOT_ACCELMAGTOUCH_SET_ANALOG_MINMAX 0x4c  //set the min and max Voltage for 1 or more touch or pot sensors

//GPS 60-6f
#define ROBOT_ACCELMAGTOUCH_GET_GPS_DATA 0x60  //start sending GPS data
#define ROBOT_ACCELMAGTOUCH_STOP_GPS_DATA 0x61  //stop sending GPS data
#define ROBOT_ACCELMAGTOUCH_SET_SEND_ALL_GPS_DATA 0x62  //set 'send all GPS data' flag
#define ROBOT_ACCELMAGTOUCH_UNSET_SEND_ALL_GPS_DATA 0x63  //unset 'send all GPS data' flag
#define ROBOT_ACCELMAGTOUCH_ENABLE_STATIC_NAVIGATION 0x64  //
#define ROBOT_ACCELMAGTOUCH_DISABLE_STATIC_NAVIGATION 0x65  //
#endif //_ROBOT_ACCELMAGTOUCHGPS_PIC_INSTRUCTIONS_H
