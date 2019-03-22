/** ECE 372 Project 2
*	Utilizes I2C1 to communicate with display board.
*	Polling Version
*	Author: Michael Escue
*/

// Define Indirect Addressing Macro for Registers

#define HWREG(x) (*((volatile unsigned int *)(x)))

// Common Defines
#define TRUE 				1
#define FALSE 				0
#define DELAY_COUNT			100000

// Base Module Defines
#define	CTRLMOD_BASE		0x44E10000
#define CM_PER_BASE			0x44E00000
#define I2C1_BASE			0x4802A000
#define DEBUGSS_DRM_BASE	0x4B160000

// Control Module Defines
#define CONF_SPI0_CS0_SCL	0x95C
#define CONF_SPI0_D1_SDA	0x958
#define MODE2				0x22 //0x22 seemed to be working.
//#define MODE2_NOREC			0x2 BOth pins must be receivers.

// Peripheral Control Module Defines
#define CM_PER_I2C1_CLKCTRL 0x48
#define CLK_ENABLE			0x2

// DRM Register Offset Defines
#define I2C_1_SUSPEND_CTRL	0x22C

// Register Address Offset Defines
#define I2C_SA				0xAC
#define I2C_CNT 			0x98
#define I2C_DATA 			0x9C
#define I2C_IRQSTATUS_RAW	0x24
#define I2C_CON				0xA4
#define I2C_PSC				0xB0
#define I2C_SCLL			0xB4
#define I2C_SCLH			0xB8
#define I2C_OA				0xA8
#define I2C_IRQENABLE_SET	0x2C
#define I2C_BUF				0x94
#define	I2C_BUFSTAT			0xC0

// I2C Register Values
#define _12MHZ_CLK			0x03
#define _tLOW_5MICROSEC		0x35
#define _tHIGH_5MICROSEC	0x37
#define OWNADDR				0x01
#define I2C1_ENABLE			0x8000
#define IRQ_DISABLED		0x0000

// Data Byte Defines
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

// Mask Defines
#define DCOUNT_VAL	 		0x0000FFFF
#define XRDY_BIT			0x00000010
#define XRDY_RDY			0x00000010
#define RRDY_BIT			0x00000008
#define RRDY_RDY			0x00000008
#define BF_BIT				0x00001000
#define BUS_IS_FREE			0
#define TXTRSH_VAL			0x0000003F
#define RXTRSH_VAL			0x00003F00
#define AERR_BIT			0x00000080
#define DATA_VAL			0xFF
#define BUFSTAT_VAL			0x0000003F

//I2C Communication Defines
#define SLAVE_ADDR			0b0111100
#define NUM_OF_DBYTES		0x8
#define START_COND			0x00000001
#define STOP_COND			0x00000002
#define MASTER_TX_MODE		0x600
#define NAME_BYTE_LENGTH	0xA



// Variables
unsigned int x;
unsigned int y;
volatile unsigned int USR_STACK[100];
volatile unsigned int IRQ_STACK[100];

void wait(void){
	while(1){
		// Endless loop
	};
}

void delay(unsigned long int y){
	while(y>0){
		y--;
	}
}

void clear_access_err(void){
	HWREG(I2C1_BASE + I2C_IRQSTATUS_RAW) = AERR_BIT;
}

void stack_init(void){
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


void irq_enable(void){
	asm("mrs r0, CPSR");
	asm("bic r0, r0, #0x80");
	asm("msr CPSR_c, R0");
}

//Not used in this polling version
void int_handler(void){
	if(HWREG(0x482000D8) == 0x20000000)
	{

	}
	asm("LDMFD SP!, {LR}");
	asm("LDMFD SP!, {LR}");
	asm("SUBS PC, LR, #0x4");
}

void set_debug(void){
	HWREG(DEBUGSS_DRM_BASE + I2C_1_SUSPEND_CTRL) = 0x9;
}

int is_bus_free(void){
	x = HWREG(I2C1_BASE + I2C_IRQSTATUS_RAW);		//"Read mask 0x00001000 from I2C_IRQSTATUS_RAW (I2C Status Raw  Register) offset 0x24 to check bus status.
	x = (x & BF_BIT);				//Mask.
	if(x == BUS_IS_FREE) return 1;
	else return 0;
}

int is_i2c_write_ready(void){
			x = HWREG(I2C1_BASE + I2C_IRQSTATUS_RAW);		//�Read mask 0x00000010 from I2C_IRQSTATUS_RAW (I2C Status Raw  Register) offset 0x24 to see if write ready�
			x = (x & XRDY_BIT);				//Mask.
			if(x == XRDY_RDY) return 1;
			else return 0;
}

void clear_i2c_write_ready(void){
			HWREG(I2C1_BASE + I2C_IRQSTATUS_RAW) = XRDY_BIT;
}

int	is_i2c_read_ready(void){
	x = HWREG(I2C1_BASE + I2C_IRQSTATUS_RAW);		//�Read mask 0x00000008 I2C_IRQSTATUS_RAW (I2C Status Raw Register) offset 0x24 see if data is ready.�
	x = (x & RRDY_BIT);				//Mask.
	if(x == RRDY_RDY) return 1;
	else return 0;
}


void startstop_condition(void){
		x = HWREG(I2C1_BASE + I2C_CON);	//"Read-Modify-Write 0x3 to I2C_CON (Configuration Register) offset 0xA4 to queue Start/Stop Condition.�
		x = (x | START_COND | STOP_COND);			//Mask.
		HWREG(I2C1_BASE + I2C_CON) = x;	//Write back.
}

void start_condition(void){
		x = HWREG(I2C1_BASE + I2C_CON);	//"Read-Modify-Write 0x3 to I2C_CON (Configuration Register) offset 0xA4 to queue Start/Stop Condition.�
		x = (x | START_COND);			//Mask.
		HWREG(I2C1_BASE + I2C_CON) = x;	//Write back.
}

void stop_condition(void){
		x = HWREG(I2C1_BASE + I2C_CON);	//"Read-Modify-Write 0x3 to I2C_CON (Configuration Register) offset 0xA4 to queue Start/Stop Condition.�
		x = (x | STOP_COND);			//Mask.
		HWREG(I2C1_BASE + I2C_CON) = x;	//Write back.
}

void config_master_transmitter(void){
	x = HWREG(I2C1_BASE + I2C_CON);		//"Read-Modify-Write 0xE00  to  I2C_CON (Configuration Register)offset 0xA4  to configure mode."
	x = (x | MASTER_TX_MODE);					//Mask.
	HWREG(I2C1_BASE + I2C_CON) = x;		//Write back.
}

void set_buf_txtrsh(unsigned int y){
	y = (y & TXTRSH_VAL);
	x = HWREG(I2C1_BASE + I2C_BUF);
	y = (y | x);
	HWREG(I2C1_BASE + I2C_BUF) = y;	//"Write 0x0000 to I2C_BUF (Buffer Configuration Register) offset 0x94 to set Transmit and Receive .�
}

void set_buf_rxtrsh(unsigned int y){
	y = ((y<<8) & RXTRSH_VAL);
	x = HWREG(I2C1_BASE + I2C_BUF);
	y = (y | x);
	HWREG(I2C1_BASE + I2C_BUF) = y;	//"Write 0x0000 to I2C_BUF (Buffer Configuration Register) offset 0x94 to set Transmit and Receive .�
}

void set_num_databytes(unsigned int y){
		y = (y & DCOUNT_VAL);
	//Number of Data Bytes pre-transmission.
	HWREG(I2C1_BASE + I2C_CNT) = y;		//"Write 0x9 to I2C_CNT (Data Count Register) offset 0x98  to set number of transmission Bytes"

}

void write_to_bus(unsigned char x){
	x = (x & DATA_VAL );
	HWREG(I2C1_BASE + I2C_DATA) = x;	//Write to data bus.

}

void set_slave_addr(unsigned int x){
//Slave address pre-transmission.
	HWREG(I2C1_BASE + I2C_SA) = x;	//"Write 0x78 to I2C_SA (Slave Address  Register) offset 0xAC  Slave address value"

}

void i2c_init(void){

	//P9 Connector settings.
	HWREG(CTRLMOD_BASE + CONF_SPI0_CS0_SCL) = MODE2;	//�Write 0x2 to conf_spi0_cs0 offset 0x95C to enable (SCL) for MODE2 w/o pullup�
	HWREG(CTRLMOD_BASE + CONF_SPI0_D1_SDA) = MODE2;	//�Write 0x2 to conf_spi0_d1 offset 0x958 to enable  (SDA) for MODE2 w/o pullup�

	//Enable Clock to I2C1.
	HWREG(CM_PER_BASE + CM_PER_I2C1_CLKCTRL) = CLK_ENABLE;	//�Write 0x2 to CM_PER_I2C1_CLKCTRL offset 0x48  to enable I2C1 Clock.�

	//Configure I2C1.
	HWREG(I2C1_BASE + I2C_PSC) = _12MHZ_CLK;	//�Write 0x03 to I2C_PSC (Clock Prescalar Register) offset 0xB0  for ICLK of 12 MHz�
	HWREG(I2C1_BASE + I2C_SCLL) = _tLOW_5MICROSEC;	//"Write 0x35 to I2C_SCLL (SCL Low Time Register) offset 0xB4  for tLOW to get 100kbps (5us-Low)"
	HWREG(I2C1_BASE + I2C_SCLH) = _tHIGH_5MICROSEC;	//"Write 0x37 to I2C_SCLH (SCL High Time Register) offset 0xB8  for tHIGH to get 100kbps (5us-High)"
	HWREG(I2C1_BASE + I2C_OA) = OWNADDR;	//�Write 0x001 to I2C_OA (Own Address Register) offset 0xA8 to configure Own Address�
	HWREG(I2C1_BASE + I2C_CON) = I2C1_ENABLE;	//�Write 0x8000 to I2C_CON (Configuration Register) offset 0xA4 to take out of reset, enable I2C1 module�
	config_master_transmitter();
	HWREG(I2C1_BASE + I2C_IRQENABLE_SET) = IRQ_DISABLED;	//"Write 0x0000 to I2C_IRQENABLE_SET (Interrupt Enable Set Register) offset 0x2C  to enable Polling�
	set_buf_txtrsh(0);
	set_buf_rxtrsh(0);
}

void init_display(void){

	unsigned int current_DCOUNT;

	set_slave_addr(SLAVE_ADDR);

	set_num_databytes(NUM_OF_DBYTES);

	while(is_bus_free() != TRUE){

	}

	startstop_condition();

	while((HWREG(I2C1_BASE + I2C_BUFSTAT) & BUFSTAT_VAL) > 0){
		
		current_DCOUNT = HWREG(I2C1_BASE + I2C_BUFSTAT) & BUFSTAT_VAL;

				if(is_i2c_write_ready()){//If ready to write
					
					switch(NUM_OF_DBYTES-current_DCOUNT){
						case 0:
							write_to_bus(FUNCTION_SET0);
							clear_i2c_write_ready();
							break;
						case 1:
							write_to_bus(FUNCTION_SET0);
							clear_i2c_write_ready();
							break;
						case 2:
							write_to_bus(FUNCTION_SET1);
							clear_i2c_write_ready();
							break;
						case 3:
							write_to_bus(BIAS_SET);
							clear_i2c_write_ready();
							break;
						case 4:
							write_to_bus(CONTRAST_SET);
							clear_i2c_write_ready();
							break;
						case 5:
							write_to_bus(PWR_ICON_CON_SET);
							clear_i2c_write_ready();
							break;
						case 6:
							write_to_bus(FOLLOWER_SET_ON);
							clear_i2c_write_ready();
							break;
						case 7:
							write_to_bus(DISPLAY_SET_ON);
							clear_i2c_write_ready();
							break;
						default:
							break;
					}
				}
				else {

				}
	}
	while((HWREG(I2C1_BASE + I2C_CNT) & BUFSTAT_VAL) != 0){
		current_DCOUNT = HWREG(I2C1_BASE + I2C_CNT) & BUFSTAT_VAL;
		switch(current_DCOUNT){
						case 0:
							delay(1);
							break;
						case 1:
							delay(1);
							break;
						case 2:
							delay(1);
							break;
						case 3:
							delay(1);
							break;
						case 4:
							delay(1);
							break;
						case 5:
							delay(1);
							break;
						case 6:
							delay(1);
							break;
						case 7:
							delay(1);
							break;
						default:
							delay(1);
							break;
			}
	}

}


	
void send_name(unsigned char *text){

	unsigned int current_DCOUNT;

	set_slave_addr(SLAVE_ADDR);

	set_num_databytes(NAME_BYTE_LENGTH);

	while(is_bus_free() != TRUE){
	}

	startstop_condition();

	while((HWREG(I2C1_BASE + I2C_BUFSTAT) & BUFSTAT_VAL) > 0){
		
		current_DCOUNT = HWREG(I2C1_BASE + I2C_BUFSTAT) & BUFSTAT_VAL;

				if(is_i2c_write_ready()){//If ready to write
					
				write_to_bus(*(text + (NAME_BYTE_LENGTH - current_DCOUNT)));

				}
				else {

				}
	}
}


int main(void){

	unsigned char mystring[] = "Mike Escue";
	unsigned char *text;
	text = mystring;

	set_debug();

	stack_init();

	i2c_init();

	init_display();

	while(is_bus_free() != TRUE){
	}
	send_name(text);

	wait();
    return 1;
}
