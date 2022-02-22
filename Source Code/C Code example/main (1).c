//Defines Section
#define HWREG(x) (*((volatile unsigned int *)(x)))

//GPIO defines
#define GPIO1BA 0x4804C000
#define GPIO_SET_DATA_OUT 0x194
#define GPIO_CLEAR_DATA_OUT 0x190

//INTC defines
#define INTCBA 0x48200000

//Timer 5 defines
#define TIMER5_BA 0x48046000

//other defines
#define CLKWKUPS 0x44E00000
#define LIGHT_BITS 0x01E00000

//Function Declarations

void IntMasterIRQEnable();
void int_handler();
void turn_off_leds();
void turn_on_leds();
void timer5_int();
void wait_loop();
void return_from_int();

//global variables
int current_state = 1;
int x;
volatile unsigned int USR_STACK[100];
volatile unsigned int INT_STACK[100];

int main(void) {
	//SET UP STACKS
	//init USR stack
	asm("LDR R13, =USR_STACK");
	asm("ADD R13, R13, #0x100");
	//init IRQ stack
	asm("CPS #0x12");
	asm("LDR R13, =INT_STACK");
	asm("ADD R13, R13, #0x100");
	asm("CPS #0x13");
	//LED INIT
	HWREG(CLKWKUPS + 0xAC) = 0x2;				//GPIO1 initialization code
	HWREG(GPIO1BA + GPIO_CLEAR_DATA_OUT) = LIGHT_BITS;		//Set initial GPIO values
	HWREG(GPIO1BA + 0x134) &= 0xFE1FFFFF;					// set output enable

	//TIMER 5 INIT
	HWREG(CLKWKUPS + 0xEC) = 0x2;			//wakeup timer 5
	HWREG(CLKWKUPS + 0x518) = 0x2;			//set clock speed
	HWREG(TIMER5_BA + 0x10) = 0x1;			//software reset
	HWREG(TIMER5_BA + 0x28) = 0x7;			//clear irqs
	HWREG(TIMER5_BA + 0x2C) = 0x2;			//enable overflow IRQ

	//INTC INIT
	HWREG(INTCBA + 0x10) = 0x2;				//reset INTC
	HWREG(INTCBA + 0xC8) = 0x20000000;		//unmast INTC_TINT5

	//ENABLE IRQ
	IntMasterIRQEnable();

	//INIT INTERNAL STATE
	HWREG(TIMER5_BA + 0x3C) = 0xFFFFE000;	//set timer for 250 ms
	HWREG(TIMER5_BA + 0x38) = 0x1;			//start timer

	wait_loop();
	return 0;
}

void wait_loop(void)
{
	while(1)
	{
		//do nothing loop
	}
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
}

void timer5_int(void)
{
	HWREG(TIMER5_BA + 0x28) = 0x7;		//clear timer5 interrupts
	HWREG(INTCBA + 0x48) = 0x1;			//clear NEWIRQ bit in INTC
	if(current_state == 1)				//toggle current state
	{
		current_state = 0;
		HWREG(TIMER5_BA + 0x3C) = 0xFFFFE000;	//set timer for 250 ms
		HWREG(TIMER5_BA + 0x38) = 0x1;			//start timer
		turn_off_leds();
	}
	else
	{
		current_state = 1;
		HWREG(TIMER5_BA + 0x3C) = 0xFFFFA002;	//set timer for 750 ms
		HWREG(TIMER5_BA + 0x38) = 0x1;			//start timer
		turn_on_leds();
	}									//toggle finished

}

void turn_on_leds(void)
{
	HWREG(GPIO1BA + GPIO_SET_DATA_OUT) = LIGHT_BITS;	//turn on leds
}

void turn_off_leds(void)
{
	HWREG(GPIO1BA + GPIO_CLEAR_DATA_OUT) = LIGHT_BITS;	//turn off leds
}


void IntMasterIRQEnable(void)
{
	asm("	mrs		r0, CPSR\n\t"
		"	bic		r0, r0, #0x80\n\t"
		"	msr		CPSR_c, R0");
}
