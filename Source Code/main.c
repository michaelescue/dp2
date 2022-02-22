/** ECE 372 Project 2
*	Utilizes I2C1 to communicate with display board.
*	Polling Version
*	Author: Michael Escue
*/

// Define Indirect Addressing Macro for Registers
#define HWREG(x) (*((volatile unsigned int *)(x)))

#define	CTRLMOD_BASE		0x44E10000 

#define CM_PER_BASE			0x44E00000

#define I2C1_BASE			0x4802A000

#define SLAVE_ADDR			0x78 

#define WRITE_TO			0x00 

#define FUNCTION_SET0		0x38 

#define FUNCTION_SET1		0x39 

#define BIAS_SET			0x14 

#define CONTRAST_SET		0x78 

#define PWR_ICON_CON_SET	0x5E 

#define FOLLOWER_SET_ON		0x6D 

#define DISPLAY_SET_ON		0x0C 

#define CLR_DISPLAY			0x01 

#define ENTRY_MODE			0x06 

#define CGRAM_SET			0x40 

// Variables
unsigned int x;
unsigned int y;
volatile unsigned int USR_STACK[100];
volatile unsigned int INT_STACK[100];

void wait(void){
	while(1);		// Endless loop
}

/* void stack_init(void){
	//SET UP STACKS
	//init USR stack
	asm("LDR R13, =USR_STACK");
	asm("ADD R13, R13, #0x1000");
	//init IRQ stack
	asm("CPS #0x12");	//Switch to IRQ mode
	asm("LDR R13, =IRQ_STACK");
	asm("ADD R13, R13, #0x1000");
	asm("CPS #0x13");	//Switch to User Mode
}


void irq_enable(void)
{
	asm("mrs r0, CPSR");
	asm("bic r0, r0, #0x80");
	asm("msr CPSR_c, R0");
}


void int_handler(void)
{
	if(HWREG(0x482000D8) == 0x20000000)
	{
		timer5_int();
	}
	asm("LDMFD SP!, {LR}");
	asm("LDMFD SP!, {LR}");
	asm("SUBS PC, LR, #0x4");
} */


void i2c_init(void){

	//P9 Connector settings.
	HWREG(CTRLMOD_BASE + 0x95C) = 0x2;	//“Write 0x2 to conf_spi0_cs0 offset 0x95C to enable (SCL) for MODE2 w/o pullup” 
	HWREG(CTRLMOD_BASE + 0x958) = 0x2;	//“Write 0x2 to conf_spi0_d1 offset 0x958 to enable  (SDA) for MODE2 w/o pullup” 

	//Enable Clock to I2C1.
	HWREG(CM_PER_BASE + 0x48) = 0x2;	//“Write 0x2 to CM_PER_I2C1_CLKCTRL offset 0x48  to enable I2C1 Clock.” 

	//Configure I2C1.
	HWREG(I2C1_BASE + 0xB0 ) = 0x03;	//“Write 0x03 to I2C_PSC (Clock Prescalar Register) offset 0xB0  for ICLK of 12 MHz” 
	HWREG(I2C1_BASE + 0xB4 ) = 0x35;	//"Write 0x35 to I2C_SCLL (SCL Low Time Register) offset 0xB4  for tLOW to get 100kbps (5us-Low)" 
	HWREG(I2C1_BASE + 0xB8 ) = 0x37;	//"Write 0x37 to I2C_SCLH (SCL High Time Register) offset 0xB8  for tHIGH to get 100kbps (5us-High)" 
	HWREG(I2C1_BASE + 0xA8 ) = 0x001;	//“Write 0x001 to I2C_OA (Own Address Register) offset 0xA8 to configure Own Address” 
	HWREG(I2C1_BASE + 0xA4 ) = 0x8000;	//“Write 0x8000 to I2C_CON (Configuration Register) offset 0xA4 to take out of reset, enable I2C1 module” 
	x = HWREG(I2C1_BASE + 0xA4 );		//"Read-Modify-Write 0xE00  to  I2C_CON (Configuration Register)offset 0xA4  to configure mode."
	x = (x | 0xE00 );					//Mask.
	HWREG(I2C1_BASE + 0xA4 ) = x;		//Write back.
	HWREG(I2C1_BASE + 0x2C ) = 0x0000;	//"Write 0x0000 to I2C_IRQENABLE_SET (Interrupt Enable Set Register) offset 0x2C  to enable Polling” 
	HWREG(I2C1_BASE + 0x94 ) = 0x0000;	//"Write 0x0000 to I2C_BUF (Buffer Configuration Register) offset 0x94 to set Transmit and Receive .” 

}

void init_display(void){
	//Slave address pre-transmission.
	HWREG(I2C1_BASE + 0xAC ) = 0x78;	//"Write 0x78 to I2C_SA (Slave Address  Register) offset 0xAC  Slave address value" 

	//Number of Data Bytes pre-transmission.
	HWREG(I2C1_BASE + 0x98 ) = 0x6;		//"Write 0x9 to I2C_CNT (Data Count Register) offset 0x98  to set number of transmission Bytes" 

	//Initiate Transmission
	x = HWREG(I2C1_BASE + 0x24 );		//"Read mask 0x00001000 from I2C_IRQSTATUS_RAW (I2C Status Raw  Register) offset 0x24 to check bus status.
	x = (x & 0x00001000)				//Mask.

	if(x == 0x00001000){				//"If BB is 0, stop condition has been generated."
		
		x = HWREG(I2C1_BASE + 0xA4);	//"Read-Modify-Write 0x3 to I2C_CON (Configuration Register) offset 0xA4 to queue Start/Stop Condition.” 
		x = (x | 0x00000003);			//Mask.
		HWREG(I2C1_BASE + 0xA4) = x;	//Write back.
		x = HWREG(I2C1_BASE + 0x24 );		//“Read mask 0x00000008 I2C_IRQSTATUS_RAW (I2C Status Raw Register) offset 0x24 see if data is ready.” 
		x = (x & 0x00000008)				//Mask.

		if(x == 0x00000008){				//"If RRDY is "1", data is ready for read."
		
			y = HWREG(I2C1_BASE + 0x9C);	//"Read I2C_DATA (Data Access Register) offset 0x9C." 

		}
		unsigned int starting_DCOUNT_val = HWREG(I2C1_BASE + 0x98);		//Initial DCOUNT value

		while(HWREG(I2C1_BASE + 0x98) > 0){		//While DCOUNT > 0

			unsigned int current_DCOUNT_val = HWREG(I2C1_BASE + 0x98);		//Update DCOUNT value
			x = HWREG(I2C1_BASE + 0x24);		//“Read mask 0x00000010 from I2C_IRQSTATUS_RAW (I2C Status Raw  Register) offset 0x24 to see if write ready” 
			x = (x & 0x00000010)				//Mask.

			if(x == 0x00000010){				//If ready to write

				unsigned int data_byte_num = starting_DCOUNT_val - current_DCOUNT_val;
				switch(data_byte_num){
					case 0;
						write_to_bus(WRITE_TO);
						break;
					case 1:
						write_to_bus(FUNCTION_SET0);
						break;
					case 2:
						write_to_bus(FUNCTION_SET1);
						break;
					case 3:
						write_to_bus(BIAS_SET);
						break;
					case 4:
						write_to_bus(CONTRAST_SET);
						break;
					case 5:
						write_to_bus(PWR_ICON_CON_SET);
						break;
					case 6:
						write_to_bus(FOLLOWER_SET_ON);
						break;
					case 7:
						write_to_bus(DISPLAY_SET_ON);
						break;
					default:
						return 0;
				}
			}
		}
	}
}

void send_name(unsigned char *text){
	//Slave address pre-transmission.
	HWREG(I2C1_BASE + 0xAC ) = 0x78;	//"Write 0x78 to I2C_SA (Slave Address  Register) offset 0xAC  Slave address value" 

	//Number of Data Bytes pre-transmission.
	HWREG(I2C1_BASE + 0x98 ) = 0xB;		//"Write 0x9 to I2C_CNT (Data Count Register) offset 0x98  to set number of transmission Bytes" 

	//Initiate Transmission
	x = HWREG(I2C1_BASE + 0x24 );		//"Read mask 0x00001000 from I2C_IRQSTATUS_RAW (I2C Status Raw  Register) offset 0x24 to check bus status.
	x = (x & 0x00001000)				//Mask.

	if(x == 0x00001000){				//"If BB is 0, stop condition has been generated."
		
		x = HWREG(I2C1_BASE + 0xA4);	//"Read-Modify-Write 0x3 to I2C_CON (Configuration Register) offset 0xA4 to queue Start/Stop Condition.” 
		x = (x | 0x00000003);			//Mask.
		HWREG(I2C1_BASE + 0xA4) = x;	//Write back.
		x = HWREG(I2C1_BASE + 0x24 );		//“Read mask 0x00000008 I2C_IRQSTATUS_RAW (I2C Status Raw Register) offset 0x24 see if data is ready.” 
		x = (x & 0x00000008)				//Mask.

		if(x == 0x00000008){				//"If RRDY is "1", data is ready for read."
			
			y = HWREG(I2C1_BASE + 0x9C);	//"Read I2C_DATA (Data Access Register) offset 0x9C." 

		}

		if(x == 0x00000010){				//If ready to write

			write_to_bus(CGRAM_SET);
					
		}

		unsigned int starting_DCOUNT_val = HWREG(I2C1_BASE + 0x98);		//Initial DCOUNT value (after single send)

		while(HWREG(I2C1_BASE + 0x98) > 0){		//While DCOUNT > 0

			unsigned int current_DCOUNT_val = HWREG(I2C1_BASE + 0x98);		//Update DCOUNT value
			x = HWREG(I2C1_BASE + 0x24);		//“Read mask 0x00000010 from I2C_IRQSTATUS_RAW (I2C Status Raw  Register) offset 0x24 to see if write ready” 
			x = (x & 0x00000010)				//Mask.

			if(x == 0x00000010){				//If ready to write

				unsigned int data_byte_num = starting_DCOUNT_val - current_DCOUNT_val;
				write_to_bus(*(text + data_byte_num);

				}
			}
		}
	}
}

void write_to_bus(unsigned int x){
			HWREG(I2C1_BASE + 0x9C) = x;	//Write to data bus.
}


int main(void){

	unsigned char mystring[] = "Mike Escue";
	unsigned *text;
	text = mystring;

	// stack_init(); // For an interrupt based version of the application.

	i2c_init();
	init_display();
	send_name(text);

	wait();
    return 1;
}
