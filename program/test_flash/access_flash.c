
#define FLASHSIZE 16777216
#define SECTORSIZE 65536
#define APPICATIONFLASHADDRESS 0x00800000
#define READBUFFERSIZE 256


// addresses for flash
volatile unsigned int* flash_parameters = (unsigned int*)0x110800; // 24-bits data,3-bits address,write,request,reconf:0b101
volatile unsigned int* flash_parameters_read = (unsigned int*)0x110804; // 24-bits data,3-bits address,busy,error,illegal
volatile unsigned int* flash_data = (unsigned int*)0x110808; // 8-bits data, 24-bits address
volatile unsigned int* flash_read = (unsigned int*)0x11080c; // 8-bits data,valid,busy,error
volatile unsigned int* flash_access = (unsigned int*)0x110810; // enable,read_enable, write_enable on 0b101, erase_enable on 0b101, read_id, read_status

// Erase one sector. Sector size is defined by SECTORSIZE, depends on flash-type, probably 64k
//   Parameters :
//      unsigned int address : address inside the sector to erase
//      return : on error below zero, on success zero
int erase_flash_sector(unsigned int address) {
	unsigned int bf;
	int timeout=0;
	do {
		bf=*flash_read;
	} while ((bf & 0x00000200) && (timeout++<1000)); // wait till busy=0
	if (bf & 0x00000200) return -1; // error: still busy
	*flash_access=0x000000a1; // enable erasing to flash with bit0=1 (access enable) and bit7..5=101 (erase enable)
	*flash_data=(address<<8); // address in bits 31..8
	timeout=0;
	do {
		bf=*flash_read;
	} while ((bf & 0x00000200) && (timeout++<10000000)); // wait till busy=0, better to use sleep function
	if ((bf & 0x00000200)!=0) return -3;
	*flash_access=0x00000000;
	return 0;
}

// Erase the factory flash segment (caution!!)
// The factory flash segment starts always at address 0
//   Parameters :
//      return : on error below zero, on success zero
int erase_factory_flash(void) {
	int i,rval;
	unsigned int adr;
	for (i=0; i<FLASHSIZE/SECTORSIZE; i++) {
		adr=i*SECTORSIZE;		
		if (adr>=APPICATIONFLASHADDRESS) return 0; // ready if sector for application address reached
		rval=erase_flash_sector(adr);
		if (rval!=0) return rval;
		if (*flash_parameters_read & 0x40000000) return -20;
	}
	return 0;
}

// Erase the application flash segment
// The application flash segment start address is defined with APPICATIONFLASHADDRESS, probably halfway flash rom
//   Parameters :
//      return : on error below zero, on success zero
int erase_application_flash(void) {
	int i,rval;
	unsigned int adr;
	for (i=0; i<FLASHSIZE/SECTORSIZE; i++) {
		adr=APPICATIONFLASHADDRESS+i*SECTORSIZE;
		if (adr>=FLASHSIZE) return 0; // ready if last sector has been reached
		rval=erase_flash_sector(adr);
		if (rval!=0) return rval;
		if (*flash_parameters_read & 0x40000000) return -20;
	}
	return 0;
}

// write bytes to flash, Maximum is 256 bytes
//   Parameters :
//      unsigned int address : starting address
//      unsigned char bytes[] : bytes to write to flash
//      int nrofbytes : number of bytes to write, maximum is 256
//      return : on error below zero, on success zero
int write_flash(unsigned int address, unsigned char bytes[], int nrofbytes) {
	unsigned int bf,i;
	int timeout=0;
	if (nrofbytes<=0) return -1;
	do {
		bf=*flash_read;
	} while ((bf & 0x00000200) && (timeout++<1000)); // wait till busy=0
	if (bf & 0x00000200) return -2; // error: still busy
	*flash_access=0x00000015; // enable writing to flash with bit0=1 (access enable) and bit4..2=101 (write enable)
	for (i=0; i<nrofbytes; i++) {
		*flash_data=(address<<8) | (unsigned int) bytes[i]; // address in bits 31..8, data in bits 7..0
	}
	*flash_access=0x00000000; // disable writing; this will start writing process
	timeout=0;
	timeout=0;
	do {
		bf=*flash_read;
	} while ((bf & 0x00000200) && (timeout++<10000000)); // wait till busy=0
	if ((bf & 0x00000200)!=0) return -5;
	return 0;
}

// Read bytes from flash, starting at given address
// The nrofbytes is limited by the flash size and the bytes buffer
//   Parameters :
//      unsigned int address : starting address
//      unsigned char bytes[] : bytes read from flash
//      int nrofbytes : number of bytes to read
//      return : on error below zero, on success zero
int read_flash(unsigned int address, unsigned char bytes[], int nrofbytes) {
	unsigned int bf,adr;
	int i,j;
	int timeout=0;
	if (nrofbytes<=0) return -1;
	*flash_access=0x00000003; // enable reading from flash with bit0=1 (access enable) and bit1=1 (read enable)
	adr=address << 8; // address in bits 31..8, start flash reading
	*flash_data=adr;
	for (i=0; i<nrofbytes; i++) {
		timeout=0;
		do {
			bf=*flash_read;
		} while (((bf & 0x00000100)==0) && (timeout++<10000)); // wait till available or error
		if ((bf & 0x00000100)==0) { *flash_access=0x00000000; return -2; }
		if ((bf & 0x00000400)!=0) { *flash_access=0x00000000; return -3; }
		*flash_data=adr; // read from fifo
		bytes[i]=(unsigned char) *flash_read;
	}
	*flash_access=0x00000000;
	timeout=0;
	do {
		bf=*flash_read;
	} while ((bf & 0x00000200) && (timeout++<10000000)); // wait till busy=0
	if ((bf & 0x00000200)!=0) return -5;
	return 0;
}

// Read flash id, only one byte, not clear wath this should be
//   Parameters :
//      unsigned char *byte : flash id value
//      return : on error below zero, on success zero
int read_flash_id(unsigned char *byte) {
	unsigned int bf;
	int timeout=0;
	do {
		bf=*flash_read;
	} while ((bf & 0x00000200) && (timeout++<1000)); // wait till busy=0
	if (bf & 0x00000200) return -1; // error: still busy
	*flash_access=0x00000101; // start reading id from flash with bit0=1 (access enable) and bit8=1 (read id)
	timeout=0;
	timeout=0;
	do {
		bf=*flash_read;
	} while ((bf & 0x00000200) && (timeout++<100)); // wait till busy=0
	if (bf & 0x00000200) return -1;
	*byte=bf & 0xff;
	*flash_access=0x00000000;
	return 0;
}

// Read status register.
//   Parameters :
//      unsigned char *byte : status register value: bit4..2=Block_Protect_bits, bit1=Write_enable, bit0=Write_inProgress
//      return : on error below zero, on success zero
int read_flash_status(unsigned char *byte) {
	unsigned int bf;
	int timeout=0;
	do {
		bf=*flash_read;
	} while ((bf & 0x00000200) && (timeout++<1000)); // wait till busy=0
	if (bf & 0x00000200) return -1; // error: still busy
	*flash_access=0x00000201; // start reading id from flash with bit0=1 (access enable) and bit8=1 (read status)
	timeout=0;
	do {
		bf=*flash_read;
	} while ((bf & 0x00000200) && (timeout++<100)); // wait till busy=0
	if (bf & 0x00000200) return -1;
	*byte=bf & 0xff;
	*flash_access=0x00000000;
	return 0;
}

// Write one of the parameters of the FPGA altremote_update component
// altremote_update parameters, defined by address:
// address=0 : only read reconfiguration condition: bit4..0=WatchDog, external nCONFIG, by ArriaII self, error by nSTATUS, CRC error
// address=2 : rd/wr watchdog timer value, 12 bits
// address=3 : enable watchdog timer, bit0
// address=4 : rd/wr page select : bit6..0 = flash_address(22..16)  // for Active Serial
// address=5 : only read application mode (= not factory mode), bit 0 
//   Parameters :
//      unsigned int address : parameter index to write (0..7)
//      unsigned int param : parameter value to write
//      return : on error below zero, on success zero
int write_flash_parameter(unsigned int address, unsigned int param) {
	unsigned int bf;
	int timeout=0;
	do {
		bf=*flash_parameters_read;
	} while ((bf & 0x08000000) && (timeout++<1000)); // wait till busy=0
	bf = ((address & 0x7) << 24) | (param & 0x00ffffff) | (0x08000000); // param address and data and bit27=write
	*flash_parameters = bf;
	timeout=0;
	do {
		bf=*flash_parameters_read;
	} while (((bf & 0x08000000)!=0) && (timeout++<1000)); // wait till busy=0 
	if ((bf & 0x08000000)!=0) return -1;
	return 0;
}


// Read one of the parameters of the FPGA altremote_update component
// altremote_update parameters, defined by address:
// address=0 : read reconfiguration condition: bit4..0=WatchDog, external nCONFIG, by ArriaII self, error by nSTATUS, CRC error
// address=2 : rd/wr watchdog timer value, 12 bits
// address=3 : enable watchdog timer, bit0
// address=4 : rd/wr page select : bit6..0 = flash_address(22..16)  // for Active Serial
// address=5 : read application mode (= not factory mode), bit 0 
//   Parameters :
//      unsigned int address : parameter index to read (0..7)
//      unsigned int *param : parameter value
//      return : on error below zero, on success zero
int read_flash_parameter(unsigned int address, unsigned int *param) {
	unsigned int bf,parbf;
	int timeout=0;
	do {
		bf=*flash_parameters_read;
	} while ((bf & 0x08000000) && (timeout++<1000)); // wait till busy=0
	if (bf & 0x08000000) return -2;
	parbf = ((address & 0x7) << 24) | (0x10000000); // 3 bits parameter address plus bit28=read request;
	*flash_parameters = parbf;
	timeout=0;
	do {
		bf=*flash_parameters_read; 
	} while ((bf & 0x08000000) && (timeout++<100)); // wait till busy=0
	*param =  bf & 0x00ffffff;
//	if (bf & 0x30000000) return bf>>28;
	if (((bf >> 24) & 0x7) != address) return -2;
	return 0;
}

//status : 
//    bit0=remote upgrade module busy
//    bit1=configuration data error 
//    bit2=illegal write to flash
//    bit3=illegal flash erase
//    bit4=data read from flash is valid
//    bit5=accessing flash is busy
//    bit6=error accesssing flash: buffer ovverrun
unsigned char read_status(void) {
	return ((*flash_parameters_read>>27) & 0x0f) | ((*flash_read>>4) & 0x7f);
}

// Reports if the device has loaded the factory configuration, or the application configuration
//   Parameters :
//      return : on error below zero, larger than zero: factory mode, zero: application mode
int factory_mode(void) {
	unsigned int param;
	int rval = read_flash_parameter(5,&param);
	if (rval<0) return rval;
	if (rval>0) return -rval;
	if (param & 1) return 0; else return 1;
}

// Start reconfiguration with application flash part
//   Parameters :
//      return : on error below zero, on success zero
// If this funcion is called from the lm32 soft-core cpu, the function will not return, the cpu is stopped
int start_reconfiguration(void) {
	unsigned int bf;
	int timeout;
	int rval;
//	rval=write_flash_parameter(4,APPICATIONFLASHADDRESS >> 16); // according to the manual, but this does not work
	rval=write_flash_parameter(4,APPICATIONFLASHADDRESS);
	if (rval<0) return rval;
	rval=write_flash_parameter(5,1); // set to application page
	if (rval<0) return rval;

	timeout=0;
	do {
		bf=*flash_parameters_read;
	} while ((bf & 0x08000000) && (timeout++<1000)); // wait till busy=0
	if ((bf & 0x08000000)!=0) return -10;
	*flash_parameters = 0xa0000000; 
	return 0;
}


// Reset the ALTREMOTE_UPDATE module
//   Parameters :
void reset_flashupdate_module(void) {
	*flash_parameters = 0xe0000000; 
}


