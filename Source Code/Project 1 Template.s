
@ ECE 372 PR0ject ONE
@ Utilizes UART5 to communicate to DOUBLETALK IC.
@ Uses R0-R3
@ Student: Michael Escue
@ Date: 01/24/19
	
	.text
	.global _start
	.global INT_DIRECTOR
	_start:

STACK_INIT: 

	@Stack initialization
	
	LDR R13, =SVCSTACK 		@Point to base of STACK for SVC mode
	
	ADD R13, R13, #0x1000 	@Point to top of STACK
	
	CPS #0x12 				@Switch to IRQ mode
	
	LDR R13, =IRQSTACK		@Point to base of STACK for IRQ mode
	
	ADD R13, R13, #0x1000		@Point to top of STACK
	

BUTTON_INIT:	

	@Enabling GPIO2 Module
	
	MOV R0, #02				@Enable GPIO2 Module Value = 0x02
		
	LDR R1, =0x44E000B0		@CM_PER_GPIO2_CLKCTRL Effective Address = 0x44E0_0000
		 
	STR R0, [R1]			@Enable GPIO2 Module
	
	
	@Enable Falling Edge Detection

	MOV R0, #0x02			@GPIO2_FALLINGDETECT Enable Value for Pin 1 = 0x2
		
	LDR R1, =0x481AC14C		@GPIO2_FALLINGDETECT Effective Address =  0x481A_C14C
	
	STR R0, [R1]			@Enable GPIO2_FALLINGDETECT
	
		
	@Enable GPIO2_1 as an Interrupt Source

	MOV R0, #0x2			@GPIO2_IRQSTATUS_SET_0 Enable Value for Pin 1 = 0x2
		
	LDR R1, =0x481AC034		@GPIO2_IRQSTATUS_SET_0 Effective Address = 0x481A_C034
		
	STR R0, [R1]			@Enable IRQSTATUS_SET_0
	
		
	@Initialize INTC ContR0ller

	MOV R0, #0xFFFFFFFF		@INTC_MIR_CLEAR1 Value to Unmask Int 32 (bit 0), GPIOINT2A, POINTRPEND1, = 0x1
		
	LDR R1, =0x482000A8		@INTC_MIR_CLEAR1 Effective Address = 0x4820_00AC
	
	STR R0, [R1]			@Unmask GPIOINT2A
	
	
HOOK_AND_CHAIN_INT:	
	
	@Hooking and Chaining the interrupts

	LDR R1, =0x4030CE38		@Load address of first instruction after IRQ exception
	
	LDR R2, [R1]			@Read the SYS_IRQ address
	
	LDR R3, =SYS_IRQ		@Read address where we will store SYS_IRQ in memory
	
	STR R2, [R3]			@Save in memory in case its needed.
	
	LDR R2, =INT_DIRECTOR	@Load address of our IRQ
		
	STR R2, [R1]			@Store in system IRQ (But beaglebone has no SYS IRQ!, JIC)
	
		
TIMER_INIT:

	@Initialize Interrupt
		
	MOV R2, #0x10			@Value to unmask INTC INT 68, TIMER 2 Interrupt
	
	LDR R1, =0x482000C8		@INTC_MIR_CLEAR2 address
	
	STR R2, [R1]			@Unmask INT 68
	
	
	@Initialize Timer 2 CLock
	
	MOV R2, #0x2			@Value to enable TImer 2 Clock
	
	LDR R1, =0x44E00080		@CM_PER_TIMER2_CLKCTRL address
	
	STR R2, [R1]			@Write to enable clock.
	
	LDR R1, =0x44E00508		@PRCMCLKSEL_TIMER2 address
	
	STR R2, [R1]			@Write to select 32 KHz Clock frequency
	
	
	@Initialize Timer 2 Count, Overflow Interrupt Generation
	
	MOV R2, #0x1			@Value to reset Timer2 
	
	LDR R1, =0x48040010		@Timer2 CFG address
	
	STR R2, [R1]			@Write to reset Timer 2
	
	MOV R2, #0x2			@Value to Enable Overflow Interrupt
	
	LDR R1, =0x4804002C		@IRQENABLE_SET address
	
	STR R2, [R1]			@Write to enable Overflow Interrupt.
	
	LDR R2, =0xFFFFC000		@LDR instead of MOV for large value; 1s count value.
	
	LDR R1, =0x48040040		@TLDR Load Register (Reload Value Register) address
	
	STR R2, [R1]			@Write value to TLDR
	
	LDR R1, =0x4804003C		@TCRR count Register address
	
	STR R2, [R1]			@Write value to TCRR. 
	
		
UART_INIT:

	@Initialize UART (Base Address = 0x481A_A000); Turn on UART5 Clock
	
	LDR R0, =0x44E00038 	@Effective address of CM_PER_UART5_CLKCTRL Register
	
	MOV R1, #0x2			 @Initialize
	
	STR R1, [R0]			@Write to CMP_PER_UART5_CLKTRL to Enable Module
	
		
	@Map P8 Connector pins to processor; Write Modes to Control Module 

	LDR R0, =0x44E10800	@Base Address of the Control Module Register
	

	@Map TxD	Connected to pin 37 (U1), lcd_data8, UART5_TXD

	MOV R2, #0x4			@Initialize to Output, mode4.
	
	MOV R1, #0xC0			@lcd_data8 Register offset.
	
	STR R2, [R0, R1]		@Write to lcd_data8 Register
	
 
	@Map RxD	Connected to pin 38 (U2), lcd_data9, UART5_RXD

	MOV R2, #0x14			@Initialize to input, mode4.
	
	MOV R1, #0xC4			@lcd_data9 Register offset.
	
	STR R2, [R0, R1]		@Write to lcd_data9 Register
	

	@Map CTS	Connected to pin 31 (V4), lcd_data14, UART5_CTSN

	MOV R2, #0x16			@Initialize to Input, mode6.
	
	MOV R1, #0xD8			@lcd_data14 Register offset.
	
	STR R2, [R0, R1]		@Write to lcd_data14 Register
	

	@Map RTS	Connected to pin 32 (T5), lcd_data15, UART5_RTSN

	MOV R2, #0x6			@Initialize to Onput, mode6.
	
	MOV R1, #0xDC			@lcd_data9 Register offset.
	
	STR R2, [R0, R1]		@Write to lcd_data9 Register
	
	
	@Set UART Mode and Configure Frame in Line Control Register 
	@Select Mode A; UART_LCR[7] = 0x1 and UART_LCR[7:0] != 0x83

	LDR R0, =0x481AA000		@Base Address for UART5 Registers
	
	@Write 0x83 to LCR to enable Mode A (offset = 0xC).

	MOV R1, #0xC			@LCR offset.
	
	MOV R2, #0x83			@Mode A select value
	
	STRB R2, [R0, R1]		@Write to LCR Register
	
	
	@Set Baud Rate using Divisor Latch Low, Divisor Latch High, and Mode Definition Register 1 (respective offsets = 0x0, 0x4, 0x20)
	@To achieve 38.4 Kbps Baud Rate write 0x00 to DLH

	MOV R1, #0x4			@DLH offset
	
	MOV R2, #0x0			@DLH write value
	
	STR R2, [R0, R1]		@Write to DLH Register, R0 is UART5 Base
	

	@and Write 0x4E to DLL

	MOV R1, #0x0			@DLL offset while in mode A.
	
	MOV R2, #0x4E			@DLL write value
	
	STR R2, [R0, R1]		@Write to DLL Register, R0 is UART5 Base
	
	
	@To achieve a 16x divisor write 0x00 to MDR1

	MOV R1, #0x20			@MDR1 offset
	
	MOV R2, #0x00			@MDR1 write value
	
	STR R2, [R0, R1]		@Write to MDR1 Register, R0 is UART5 Base
	

	@Set UART to Operation Mode in LCR Register (offset = 0xC)
	@UART_LCR = 0x03

	MOV R1, #0xC			@LCR offset
	
	MOV R2, #0x03			@MDR1 write value to enable 7 bit data transfer, no parity, one stop bit.
	
	STR R2, [R0, R1]		@Write to LCR Register, R0 is UART5 Base
	

	@Clear Transmit buffer and Disable it using FIFO Control Register (offset = 0x8) Disable Rx FIFO, Tx FIFO, and FIFO_EN. 
	@Write 0x6 

	MOV R1, #0x8			@FCR Register offset
	
	MOV R2, #0x0			@Disable and Clear FIFO Value
	
	STR R2, [R0, R1]		@Write to FCR Register, clear and disable Tx and Rx FIFOs.
	
	MOV R2, #0x6
	
	STR R2, [R0, R1]
	
	LDR R2, [R0, R1]	@Debugging.
	


	
IRQ_MODE_INIT:

	@Enable IRQ mode

	MRS R0, CPSR			@Read CPSR
	
	BIC R3, #0x80			@Modify CPSR, Clear bit 7, enables PR0ccessor IRQ input
	
	MSR CPSR_c, R3			@Write to CPSR
	
	
_LOOP:				@Loop
			
	NOP
			
	B _LOOP 		@Always and forever.
	
INT_DIRECTOR:

	STMFD SP!, {R0-R4, LR} 		@Save All used registers on the stack.
	

UART_INT_CHECK:
	
	@Check interrupt bit 14 (UART5)
					
	LDR R0, =0x482000B8			@Load Address of INTC_PEND_IRQ1 =0x482000B8
				
	LDR R1, [R0]				@Read INTC_PEND_IRQ1
	
	MOV R2, #0x4000				@Mask bit 14
	
	AND R1, R2					@Mask bit 14
	
	CMP R1, #0x4000				
		
	BEQ TALKER_SVC				@UART5 interrupt! Branch to Talker Service Routine. Otherwise fall through.
	
	
TIMER_INT_CHECK:	
	
	LDR R1, =0x482000D8			@INTC_PENDING_IRQ2
	
	MOV R2, #0x10				@Test Value
	
	LDR R0, [R1]				@Read Register
	
	CMP R0, R2					@Test bit 4.
	
	BNE BTN_INT_CHECK			@Check button if not Timer Interrupt.
	
	LDR R1, =0x48040028			@IRQSTATUS Register address of Timer 2.
	
	MOV R2, #0x2				@Test value for bit 1
	
	LDR R0, [R1]				@Read Register
	
	CMP R0, R2					@Check bit 1
	
	BNE BTN_INT_CHECK 			@If Not overflow, go to BTN check.
	
	MOV R2, #0x2				@Value to reset Timer overflow IRQ request.
	
	LDR R1, = 0x48040028		@Load Timer2 IRQSTATUS Register
	
	STR R2, [R1]				@Write reset to IRQStatus Register.
	
	@Enable UART Interrupt at Interrupt Enable Register (offset = 0x4)
	@Write 0x0A

	LDR R0, =0x481AA000			@Load base address of UART5 Registers.
	
	MOV R1, #0x4				@IER offset
		
	MOV R2, #0xA				@Enable Interrupt value
		
	STR R2, [R0, R1]			@Write to IER, enable interrupt.
	
	b _RETURN
	
	
	
BTN_INT_CHECK:
	
	@Check interrupt bit 0 (BTN)
	
	LDR R0, =0x482000B8			@Load Address of INTC_PEND_IRQ1 =0x482000B8

	LDR R1, [R0]				@Read INTC_PEND_IRQ1
	
	MOV R2, #0x0001				@Mask bit 0
	
	AND R1, R2					@Mask bit 0
	
	CMP R1, #0x1				
	
	BNE PASS_ON				@Not a Button Interrupt! Pass to next step.
	
	LDR R0, =0x481AC02C			@Load Address of GPIO2_IRQSTATUS_0 Register
				
	LDR R1, [R0]				@Read GPIO2_IRQSTATUS_0 Register
				
	CMP R1, #0x2				@Compare the value to pin 1 (bit 1)
				
	BEQ BTN_SVC					@IF Bit 0 == 1, button pushed! Goto BTN_SVC, Otherwise fall-through to return.
	

PASS_ON:
	
	LDMFD SP!, {R0-R4, LR}		@Resotre Registers saved on interrupt
				
	SUBS PC, LR, #4				@Return to Idle Loop. 
	
						
BTN_SVC:
	
	MOV R1, #0x2				@Load Value to disable interrupt Request on GPIO2_IRQSTATUS_0 = 0x2
	
	LDR R0, =0x481AC02C			@Load Address of GPIO2_IRQSTATUS_0 Register
				
	STR R1, [R0]				@Write to GPIO2_IRQSTATUS_0  to disable Interrupt req.
				
	LDR R0, =0x48200048			@Load address of INTC_CONTR0L register = 0x48200048
				
	MOV R1, #01					@Load Write Value of 1 to clear bit 0 of INTC_CONTR0L REG, "NewIRQAgr"
				
	STR R1, [R0]				@Write Value to INTC_CONTR0L Register
	
	
	@Enable UART Interrupt at Interrupt Enable Register (offset = 0x4)
	@Write 0x0A

	LDR R0, =0x481AA000			@Load base address of UART5 Registers.
	
	MOV R1, #0x4				@IER offset
		
	MOV R2, #0xA				@Enable Interrupt value
		
	STR R2, [R0, R1]			@Write to IER, enable interrupt.	

	LDMFD SP!, {R0-R4, LR}		@Restore saved Registers
				
	SUBS PC, LR, #4				@Return PC to interrupted instruction.
	
	
TALKER_SVC:

	LDR R0, =0x48200048			@Load address of INTC_CONTR0L register = 0x48200048
				
	MOV R1, #01					@Load Write Value of 1 to clear bit 0 of INTC_CONTR0L REG, "NewIRQAgr"
				
	STR R1, [R0]				@Write Value to INTC_CONTR0L Register
	
	LDR R0, =0x481AA018			@MSR Address
	
	LDR R2, [R0]				@Read MSR
	
	MOV R3, #0x0010				@bit 4 mask
	
	AND R2, R3 					@Mask all other bits.
	
	MOV R1, #0x10				@Bit 4 is 1 for compare
	
	CMP R2, R1					@Compare Bit 4 to see if CTS# was asserted.

	BNE CTS_NOTASSERT			@IF CTS# was not asserted low, Check THR contents.
	

	@CTS# bit == 1, THEN;

	LDR R0, =0x481AA014			@LSR Address
	
	LDR R2, [R0]				@Read LSR
	
	MOV R3, #0x0020				@bit 5 mask
	
	AND R2, R3 					@Mask all other bits.
	
	MOV R1, #0x20				@Bit 5 is 1 for compare
	
	CMP R2, R1					@Compare Bit 5 to see if THR is EMPTY.

	BNE _RETURN					@IF THR bit == 0 is not empty, return to wait loop.
	
	
	@SEND CHAR
	
	LDR R0, =MESSAGE			@Address of Message
		
	LDR R3, =CHAR_COUNT				@Address of COUNT
	
	LDR R3, [R3]				@Value of COUNT
	
	LDRB R1, [R0, R3]			@Load Char_Count For Relative Address
	
	LDR R2, =0x481AA000			@Base address of UART5, includes offset for THR.
	
	STRB R1, [R2]				@Transmit word
	
	LDR R2, =CHAR_COUNT			@Address of CHAR_COUNT
	
	LDR R3, [R2] 				@Load Value of CHAR_COUNT
	
	ADD R3, R3, #1				@Increment Count
	
	STR R3, [R2]				@Store Value of Count
	
	LDR R2, =COUNT				@Address of COUNT
		
	LDR R3, [R2] 				@Load Value of COUNT
	
	ADD R3, R3, #1				@Increment Count
	
	STR R3, [R2]				@Store Value of Count 
	
	
	@CHECK LAST CHAR
	
	LDR R2, =MESSAGE_LENGTH		@Load MESSAGE_LENGTH ADDRESS
	
	LDR R2, [R2]				@Load MESSAGE_LENGTH value
	
	CMP R2, R3					@Compare MESSAGE_LENGTH to Value of CHAR_COUNT
	
	BNE _RETURN 				@IF COUNT == MESSAGE LENGTH, Return.
	
	@CHECK LAST LINE
	
	LDR R2, =CHAR_COUNT			@Address of CHAR_COUNT
	
	LDR R3, =END_CHAR_COUNT
	
	LDR R2, [R2] 				@Load Value of CHAR_COUNT
	
	LDR R3, [R3]				@Load Value of END_CHAR_COUNT
	
	CMP R2, R3					@Compare Counts to see if Tx is complete.
	
	BNE RESET_COUNT
	
	MOV R2, #0x0				@Value to stop timer
	
	LDR R1, =0x48040038			@Timer2 CFG address
	
	STR R2, [R1]				@Write to Control Register for auto reload and stop.
	
	@MOV R2, #0x2				@Value to reset Timer overflow IRQ request.
	
	@LDR R1, = 0x48040028		@Load Timer2 IRQSTATUS Register
	
	@STR R2, [R1]				@Write reset to IRQStatus Register.
	

				
	LDR R0, =0x481AA000			@Load base address of UART5 Registers.
	
	MOV R1, #0x4				@IER offset
		
	MOV R2, #0x0				@Enable Interrupt value
	
	STR R2, [R0, R1]			@Write to IER.
	
	MOV R1, #0x0
	
	LDR R2, =CHAR_COUNT			@load Address of CHAR_COUNT
	
	STR R1, [R2]				@set count to 0, back to first char.
		
	MOV R1, #0x0
	
	LDR R2, =COUNT				@load Address of COUNT
	
	STR R1, [R2]				@set count to 0, back to first char.
	
	B	_RETURN 				@End of Send branch, return to Idle Loop.
	
RESET_COUNT:

	LDR R2, =COUNT				@load Address of COUNT
				
	MOV R1, #0x0
	
	STR R1, [R2]			@set count to 0, back to first char.
	
	
	@DISABLE UART INTERRUPT.
	LDR R0, =0x481AA000			@Base address of UART5,

	MOV R1, #0x4 				@IER Register Offset
	
	MOV R2, #0x00 				@Disable THR Interupt.
	
	STR R2, [R0, R1] 			@Write 0x00 to IER.
	
	@RESET TIMER
	
	MOV R2, #0x3			@Value for one-shot timer
	
	LDR R1, =0x48040038		@Timer2 CFG address
	
	STR R2, [R1]			@Write to Control Register for auto reload and stop.
	
	B	_RETURN 			@End of Send branch, return to Idle Loop.
	
			
CTS_NOTASSERT:
			
	@CTS# bit == 0;
	
	MOV R1, #0x14				@LSR Offset
	
	LDR R2, [R0, R1]			@Read LSR
	
	MOV R1, #0x20				@Bit 5 is 1 for compare
	
	@IF (THR bit == 1);
	
	BNE _RETURN					@IF THR is not empty, return to wait loop.

	@THR bit == 1, THEN;
	
	MOV R1, #0x4 				@IER Register Offset
	
	MOV R2, #0x00 				@Disable THR Interupt.
	
	STR R2, [R0, R1] 			@Write 0x00 to IER.
	
	
_RETURN:
				
	LDMFD SP!, {R0-R4, LR}		@Restore saved Registers
				
	SUBS PC, LR, #4				@Return PC to interrupted instruction.	
	

VARIABLES:
	
	.data
	
	.align 2
	SYS_IRQ:		.word	0x0	
	COUNT:			.word	0x0
	CHAR_COUNT:			.word	0x0

		.align 2
	
	MESSAGE:
		.byte 0x01
		.ascii "1O         "
		.byte 0x0d
		.ascii "10.        "
		.byte 0x0d
		.byte 0x0d
		.ascii "9.         "
		.byte 0x0d
		.byte 0x0d
		.ascii "8.         "
		.byte 0x0d
		.byte 0x0d
		.ascii "7.         "
		.byte 0x0d
		.byte 0x0d
		.ascii "6.         "
		.byte 0x0d
		.byte 0x0d
		.ascii "5.         "
		.byte 0x0d
		.byte 0x0d
		.ascii "4.         "
		.byte 0x0d
		.byte 0x0d
		.ascii "3.         "
		.byte 0x0d
		.byte 0x0d
		.ascii "2.         "
		.byte 0x0d
		.byte 0x0d
		.ascii "1.         "
		.byte 0x0d
		.byte 0x0d
		.ascii "0 blast off"
		.byte 0x0d
		.align 2
	MESSAGE_LENGTH: .word 13
	END_CHAR_COUNT:		.word	0x9B
	
	.align 3
	
	SVCSTACK:	.rept	1024
				.word	0x0000
				.endr
				
	IRQSTACK:	.rept	1024
				.word	0x0000
				.endr
	

		
	.end