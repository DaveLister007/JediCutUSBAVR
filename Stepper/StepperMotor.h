/*
 * StepperMotor.h
 *
 * Created: 7.4.2012 19:27:44
 *  Author: DaveLister
 */ 


#ifndef _STEPPER_MOTOR_H_
#define _STEPPER_MOTOR_H_
	#include <avr/io.h>
	//#include "JediCutFirmware.h"
	
	
	#define STEPPER_MOTOR_COUNT 4
	
	void StepperMotor_init(void);
	
	void StepperMotor_off(uint8_t motorNr);
	void StepperMotor_on(uint8_t motorNr);
	
	void StepperMotor_step(uint8_t motorNr,uint8_t dir);
#endif //_STEPPER_MOTOR_H_