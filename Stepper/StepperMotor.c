#include "StepperMotor.h"
#include "StepperMotorConfig.c"


void StepperMotor_sendStep(uint8_t step,volatile TSTEPPERMOTOR_CONFIG* config);


void StepperMotor_init(void){
	for(int i=0; i<STEPPER_MOTOR_COUNT; i++)
	{
		StepperMotor_config[i].currentStep = 0;
		StepperMotor_sendStep(0,&(StepperMotor_config[i]));
	}
	
}
void StepperMotor_step(uint8_t motorNr,uint8_t dir){
	volatile TSTEPPERMOTOR_CONFIG *config = &StepperMotor_config[motorNr];
	int8_t currentStep = config->currentStep;
	if (dir){
		currentStep++;
		if (currentStep >= STEPPER_MOTOR_STEP_COUNT) currentStep = 0;
	} else {
		currentStep--;
		if (currentStep < 0) currentStep = STEPPER_MOTOR_STEP_COUNT-1;
	}
	config->currentStep = currentStep;
	
	StepperMotor_sendStep(currentStep,config);	
	
	
}

inline void StepperMotor_sendStep(uint8_t step,volatile TSTEPPERMOTOR_CONFIG* config){
	*config->port = (StepperMotor_steps[step] << config->firstPin) & config->mask;
}


void StepperMotor_off(uint8_t motorNr){
	volatile TSTEPPERMOTOR_CONFIG *config = &StepperMotor_config[motorNr];
	*config->port &= config->mask;
}

void StepperMotor_on(uint8_t motorNr){
	volatile TSTEPPERMOTOR_CONFIG *config = &StepperMotor_config[motorNr];
	StepperMotor_sendStep(config->currentStep,config);
}