#include "JediCutFirmware.h"
#include <stdbool.h>

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber         = 0,

				.DataINEndpointNumber           = CDC_TX_EPNUM,
				.DataINEndpointSize             = CDC_TXRX_EPSIZE,
				.DataINEndpointDoubleBank       = false,

				.DataOUTEndpointNumber          = CDC_RX_EPNUM,
				.DataOUTEndpointSize            = CDC_TXRX_EPSIZE,
				.DataOUTEndpointDoubleBank      = false,

				.NotificationEndpointNumber     = CDC_NOTIFICATION_EPNUM,
				.NotificationEndpointSize       = CDC_NOTIFICATION_EPSIZE,
				.NotificationEndpointDoubleBank = false,
			},
	};

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs
 */
//static FILE USBSerialStream;

#define CMD_BUFFER_SIZE 300  // must be even !
volatile char cmdArray[CMD_BUFFER_SIZE];
volatile int arrayIdxRead  = 0;
volatile int arrayIdxWrite = 0;
volatile int cmdCounter = 0;
volatile bool ovf = false;

//volatile bool isrActive = false;




void serialHandler(){
	if (ovf && arrayIdxRead == arrayIdxWrite){ //if buffer is full, function will not do anything, only return and light up overflow led
		LEDs_TurnOnLEDs(LEDMASK_OVERFLOW);
		return;
	}
	
	if (CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface) < 1) return; //nothing received, nothing to do
	
	char command[2];
	command[0] = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
	
	while(CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface) < 1){}//wait until second byte of command is received
	command[1] = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
	
	
	
	// Each command consists of 2 bytes
	cmdArray[arrayIdxWrite] = command[0];
	cmdArray[arrayIdxWrite+1] = command[1];
	
	 // correct the write index
	 arrayIdxWrite+=2;
	 if(arrayIdxWrite==CMD_BUFFER_SIZE) arrayIdxWrite=0;
	 
	 cli();
	 cmdCounter++;
	 sei();

	 
	 // check for oncoming overflow
	 if(cmdCounter >= CMD_BUFFER_SIZE/2-20)
	 {
		 ovf = true;
		 CDC_Device_SendByte(&VirtualSerial_CDC_Interface,'S');// Stop transmission, Buffer full
		 CDC_Device_Flush(&VirtualSerial_CDC_Interface);
	 }
}


int main(void)
{
	SetupHardware();

	/*character stream - for debugging purposes*/
	//CDC_Device_CreateBlockingStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	sei();

	for (;;)
	{
		serialHandler();

		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals*/
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	DDRB  |=  0xFF;
	PORTB &=  0x00;
	DDRD  |=  0xFF;
	PORTD &=  0x00;
	


	StepperMotor_init();
	
	LEDs_Init();
	USB_Init();

	
	//timer1 configuration (16bit timer, CTC mode, 256/clk prescaled clock source*/
	TCCR1A  = (0 << WGM11) | (0 << WGM10);
	TCCR1B  = (0 << WGM13) | (1 << WGM12) | (1 << CS12) | (0 << CS11) | (0 << CS10);
	OCR1A  = 255;           // Timer compare register. 
	TIMSK1 = (1 << OCIE1A); // Timer interrupt enable for match to OCR1A
	TIFR1  = 0;             // Clear flags.
	TCNT1  = 0;
	sei();

}

/**********************************************************************************/
void sendMotorCmd(uint8_t cmd)
{
	uint8_t directions = (cmd & 0xf0) >> 4;
	uint8_t steps = (cmd & 0x0f);
	
	uint8_t motor0Mask = (1<<0);
	if (steps & motor0Mask){
		StepperMotor_step(0,directions & motor0Mask);
	}
	uint8_t motor1Mask = (1<<1);
	if (steps & motor1Mask){
		StepperMotor_step(1,directions & motor1Mask);
	}
	uint8_t motor2Mask = (1<<2);
	if (steps & motor2Mask){
		StepperMotor_step(2,directions & motor2Mask);
	}
	uint8_t motor3Mask = (1<<3);
	if (steps & motor3Mask){
		StepperMotor_step(3,directions & motor3Mask);
	}	
	
	/**
	PORTD = (PORTD & 0x0F) | (cmd & 0xf0); // Directions first!
	PORTB = (PORTB & 0xF0) | (cmd & 0x0f); // and step
	delayMicroseconds(25); // eventually wait a little bit
	// and falling edge of step pulse
	PORTB = (PORTB & 0xF0);
	*/
	
}


/**********************************************************************************/
void handleCommand()
{

	char val = cmdArray[arrayIdxRead+1]; // The command parameter value
	
	//fprintf(&USBSerialStream,"Command from buffer: %c%c\r\n",cmdArray[arrayIdxRead],val);
	
	switch(cmdArray[arrayIdxRead])
	{
		case 'A':   // All Motors on/off
			for(uint8_t i=0;i<STEPPER_MOTOR_COUNT;i++){
				if(val == '1')
					StepperMotor_on(i);
				else
					StepperMotor_off(i);
			}
			break;	
		case 'H':   // Wire Heat ON/OFF (may be programmed as PWM (analog out))
		  /*if(val > 0)   {digitalWrite(2, LOW);}
		  else          {digitalWrite(2, HIGH);}
		  analogWrite(3,val*2.55); // PWM for wire heating (stretch 0-100% to a range of 0-255)*/
		break;
    case 'M':   // Motor step Command
		sendMotorCmd(val);
		break;
    case 'F':   // Change the timer frequency, the time between two steps
		OCR1A = (uint16_t)val*(F_CPU/1000/256);//256 is timer's prescaler value, F_CPU is frequency
    break;
  }

}


/**********************************************************************************/
ISR(TIMER1_COMPA_vect) {
	
	do {
		// check if the buffer is empty
		if((arrayIdxRead != arrayIdxWrite) || ovf)
		{
			
			handleCommand();
			arrayIdxRead += 2;
			if(arrayIdxRead==CMD_BUFFER_SIZE) arrayIdxRead=0;
			
			cmdCounter--;
			
			if (ovf && (cmdCounter<CMD_BUFFER_SIZE/2-25))
			{
				CDC_Device_SendByte(&VirtualSerial_CDC_Interface,'C');
				CDC_Device_Flush(&VirtualSerial_CDC_Interface);
				
				ovf = false;
			}
			LEDs_TurnOffLEDs(LEDMASK_UNDERFLOW);
			//digitalWrite(13, LOW); // underflow led off
		}
		else
		{
			// underflow !!
			//digitalWrite(13, HIGH); // underflow led on
			LEDs_TurnOnLEDs(LEDMASK_UNDERFLOW);
			break;
		}
	} while(cmdArray[arrayIdxRead] != 'M'); // only motor commands will wait for next sync, all others can be handled immediately
}




/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}
