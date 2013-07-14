/*
 * CProgram1.c
 *
 * Created: 7.4.2012 21:18:50
 *  Author: DaveLister
 */ 


/*uint8_t StepperMotor_steps[4] = {
	0b00001000,
	0b00000100,
	0b00000010,
	0b00000001
	};*/

		
uint8_t StepperMotor_steps[] = {
	0b00001000,
	0b00001100,
	0b00000100,
	0b00000110,
	0b00000010,
	0b00000011,
	0b00000001,
	0b00001001,
	
	};
#define STEPPER_MOTOR_STEP_COUNT sizeof(StepperMotor_steps)


typedef struct{
	volatile uint8_t *port;
	uint8_t mask;
	uint8_t firstPin;
	int8_t currentStep;
} TSTEPPERMOTOR_CONFIG;

		
		

volatile TSTEPPERMOTOR_CONFIG StepperMotor_config[STEPPER_MOTOR_COUNT] = {
	{
		.port = &PORTD,
		.mask = 0b00001111, //mask of bits that stepper uses in port
		.firstPin = 0, //first pin of stepper, determines what pin of port is first used for stepper output
		.currentStep = 0,
	},
	{
		.port = &PORTD,
		.mask = 0b11110000,
		.firstPin = 4,
		.currentStep = 0,
	},
	{
		.port = &PORTB,
		.mask = 0b00001111,
		.firstPin = 0,
		.currentStep = 0,
	},
	{
		.port = &PORTB,
		.mask = 0b11110000,
		.firstPin = 4,
		.currentStep = 0,
	}
};
