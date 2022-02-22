	
void send_name(unsigned char *text){

	unsigned int current_DCOUNT;

	set_slave_addr(SLAVE_ADDR);

	set_num_databytes(NUM_OF_DBYTES);

	while(is_bus_free() != TRUE){

	}

	startstop_condition();

	while((HWREG(I2C1_BASE + I2C_BUFSTAT) & BUFSTAT_VAL) > 0){
		
		current_DCOUNT = HWREG(I2C1_BASE + I2C_BUFSTAT) & BUFSTAT_VAL;

				if(is_i2c_write_ready()){//If ready to write
					
				write_to_bus(*(text + data_byte_num));
				
				else {

				}
	}
}