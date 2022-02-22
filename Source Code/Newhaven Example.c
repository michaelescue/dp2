
/****************************************************
* Initialization For ST7036i *
*****************************************************/
void init_LCD()
{
I2C_Start();
I2C_out(Slave);		//Slave=0x78, 0111_1000
						//	R/W = 0 (Write active)
I2C_out(Comsend);	//Comsend = 0x00, reserved for several functions including software reset.
						/*
						*	Control Byte 0
						*	Co = 0  (Last control byte, data streaming, to be followed by stop.)
						*	Co & Rs = 0 (Write instruction code into IR.)
						*/

					//876543210
					//CR6543210
I2C_out(0x38); 		//‭0011_1000‬
						/*
						*	Instruction Data Byte 1
						*	"Function Set"
						*	DL = 1 	(8-bit bus mode with MPU)
						*	N = 1 	(2-line display mode is set)
						*	DH = 0 	(Double height mode disabled, cannot be high when 2 lines are used, i.e N =1)
						*	(only when EXT option pin connected to Vss is IS bits used: Assuming it is here:)
						*	IS[2:1] = 00 Normal instruction table 0 selected.
						*/
						
I2C_out(0x39);		//‭0011_1001‬
						/*
						*	Instruction Data Byte 2
						*	"Function Set"
						*	DL = 1	(8-bit bus mode with MPU)
						*	N = 1 	(2-line display mode is set)
						*	DH = 0 	(Double height mode disabled, cannot be high when 2 lines are used, i.e N =1)
						*	(only when EXT option pin connected to Vss is IS bits used: Assuming it is here:)
						*	IS[2:1] = 01 Extension instruction table 1 selected.
						*/
delay(10);
I2C_out(0x14);		//‭0001_0100‬
						/*
						*	Instruction Data 3
						*	"Bias Set" from instruction table 1.
						*	BS = 0 	(1/5 bias)
						*	FX = 		(non-3-line application, 2-line in fact.)
						*/
						
I2C_out(0x78);		//0111_1000
						/*
						*	Instruction Data 4
						*	"Contrast Set" (lower 4 bytes)
						*	Can only be set when internal follower is used (OPF1 = 0, OPF2 = 0)
						*	These bits tune the input reference voltage:
						*	C3 = 1
						*	C2 = 0
						*	C1 = 0
						*	C0 = 0
						*/
						
I2C_out(0x5E);		//‭0101_1110‬
						/*
						*	"Power/ICON Control/ Contrast Set"
						*	Ion = 1	(Icon display on)
						*	Bon can only be set when internal follower is used (OPF1 = 0, OPF2 = 0)
						*	Bon = 1	(booster circuit turned on)
						*	These bits tune the input reference voltage:
						*	C5 = 1	
						*	C4 = 0
						*/
						
I2C_out(0x6D);		//‭0110_1101‬
						/*
						*	"Follower Control"
						*	Fon = 1		(Internal follower is on)
						*	These bits control the amplified ratio of V0 Generator.
						*	Rab2 = 1	
						*	Rab1 = 0	
						*	Rab0 = 1	
						*/						
					
I2C_out(0x0C);		//0000_‭1100‬
						/*
						*	"Display On/Off"
						*	D = 1 	(Entire Display on)
						*	C = 0	(Cursor off)
						*	B = 0	(Cursor position off)
						*/
						
/*****************************************************************************************************
*	Manual Description of initialization ends here.
*****************************************************************************************************/
						
I2C_out(0x01);		//0000_0001
						/*
						*	"Clear Display"
						*	DDRAM address is set to 00h from AC.
						*/
						
I2C_out(0x06);		//0000_0110
						/*
						*	"Entry Mode Set"
						*	Sets cursor move direction and specifies display shift
						*	These operations are performed during data write and read.
						*	I/D = 1		(Cursor/blink moves to right and DDRAM address is increased by 1)
						*	S = 0		(No shift during DDRAM read operation)
						*	(If S = 1 during DDRAM write operation, shift of entire display is performed according to I/D *		value)
						*/
delay(10);			
I2C_Stop();
}
/*****************************************************/

/****************************************************
* Output command or data via I2C *
*****************************************************/
void I2C_out(unsigned char j) //I2C Output
{
	int n;
	unsigned char d;
	d=j;
	for(n=0;n<8;n++){
		if((d&0x80)==0x80)
			SDA=1;
		else
			SDA=0;
			d=(d<<1);
			SCL = 0;
			SCL = 1;
			SCL = 0;
	}
	SCL = 1;
	while(SDA==1){
		SCL=0;
		SCL=1;
	}
	SCL=0;
}
/*****************************************************/

/****************************************************
* I2C Start *
*****************************************************/
void I2C_Start(void)
{
SCL=1;
SDA=1;
SDA=0;
SCL=0;
}
/*****************************************************/

/****************************************************
* I2C Stop *
*****************************************************/
void I2C_Stop(void)
{
SDA=0;
SCL=0;
SCL=1;
SDA=1;
}
/*****************************************************/

/****************************************************
* Send string of ASCII data to LCD *
*****************************************************/
void Show(unsigned char *text)
{
int n,d;
d=0x00;
I2C_Start();
I2C_out(Slave); 	//Slave=0x78, 0111_1000
						//	R/W = 0
I2C_out(Datasend);	//Datasend=0x40, 0100_0000
						/*
						*	Co = 0
						*	RS = 1
						*	Rs = 1 and R/W = 0 instructs a data write into internal RAM (DDRAM/CGRAM)
						*	"Set CGRAM"
						*	Sets CGRAM address in address counter
						*	AC[6:0] = 0
						*	Any following data transmissions are written then the cursor moves to the right per init.
						*/
for(n=0;n<20;n++){
	I2C_out(*text);
	++text;
}
I2C_Stop();
}
/*****************************************************/
/*****************************************************/
/*****************************************************/