// This program test some functions of the KVI modules for the white rabbit project

#include "access_flash.h"

// address for LED register
volatile unsigned int* leds = (unsigned int*)0x100400;
// 262144


// addresses for single pulse generator
volatile unsigned int* singlepulse_delay = (unsigned int*)0x110000; // number of clock-cycles delay after trigger
volatile unsigned int* singlepulse_duration = (unsigned int*)0x110004; // number of clock-cycles duration of the pulse
volatile unsigned int* singlepulse_control = (unsigned int*)0x110008; // control bits 0..3 = enable,not_used,stop,softtrigger
volatile unsigned int* singlepulse_status = (unsigned int*)0x11000c; // status bits 0,1 = pulse_busy, pulse_active

// addresses for pattern generator
volatile unsigned int* pattern_data = (unsigned int*)0x110400; // parallel data to memory
volatile unsigned int* pattern_period = (unsigned int*)0x110404; // pattern-clock period in WR_clock cycles
volatile unsigned int* pattern_control = (unsigned int*)0x110408; // control bits 0..3 = enable, load, stop, softtrigger
volatile unsigned int* pattern_status = (unsigned int*)0x11040c; // status bit = busy, 23..16 = width, 31..24 = memory depth

// addresses for BuTiS clock module
volatile unsigned int* BuTiSclock_lw = (unsigned int*)0x110500; // Timestamp to set: low 32 bits of 64-bits timestamp
volatile unsigned int* BuTiSclock_hw = (unsigned int*)0x110504; // Timestamp to set: high 32 bits of 64-bits timestamp
volatile unsigned int* BuTiSclock_control = (unsigned int*)0x110508; // control bit 0,1,2 = set,sync,reset, bit 15..8 = phase
volatile unsigned int* BuTiSclock_status = (unsigned int*)0x11050c; // status bit 0,1 = set, phase of PPS signal

// addresses for simple rs232 module
volatile unsigned int* rs232_datasend = (unsigned int*)0x110600; // bit 7..0 = data to send
volatile unsigned int* rs232_dataread = (unsigned int*)0x110604; // bit 7..0 = received data
volatile unsigned int* rs232_readdone = (unsigned int*)0x110608; // write enables next receive data
volatile unsigned int* rs232_status = (unsigned int*)0x11060c; // status bit 0,1 = sending allowed, received data available
volatile unsigned int* rs232_control = (unsigned int*)0x110610; // control bit 2..0 = baudrate (0to7): clock,115k2,57k6,38k4,19k2,9k6,4k8,2k4

// addresses for timestamp reading
volatile unsigned int* readtime_highword = (unsigned int*)0x110700; // 64-bits timestamp received, bits 63..32
volatile unsigned int* readtime_lowword = (unsigned int*)0x110704; // 64-bits timestamp received, bits 31..0
volatile unsigned int* readtime_errors = (unsigned int*)0x110708; // number of errors
volatile unsigned int* readtime_corrections = (unsigned int*)0x11070c; // number of corrections
volatile unsigned int* readtime_control = (unsigned int*)0x110710; // control bit 0..3 = disable,error,correction,clear

/*
void _read(void) {}
void isatty(void) {}
void _sbrk(void) {}
void _write(void) {}
void _close(void) {}
void _fstat(void) {}
void _lseek(void) {}
*/

#define INT_DIGITS 19		/* enough for 64 bit integer */

// integer to ascii conversion
char *itoa(int i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1;	/* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {			/* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}


void _irq_entry(void) {
  /* Currently only triggered by DMA completion */
}

// send character using the simple rs232 module
int writechar_rs232module(char c) {// send character, return 0 on success
	int timeout=0;
	while ((*rs232_status & 0x1)==0) { // wait till previous character has been sent
		asm("# noop"); /* no-op the compiler can't optimize away */
		if (timeout++>=20000) return -1;
	}
	*rs232_datasend=c;
	return 0;
}

// send character using the pattern generator as rs232 transmitter
int writechar(char c) {// send character, return 0 on success
	int timeout=0;
	int i;
	while ((*pattern_status & 0x1)==1) { // wait till previous character has been sent
		asm("# noop"); /* no-op the compiler can't optimize away */
		if (timeout++>=20000) return -1;
	}
	*pattern_control = 2;
	*pattern_data = 1; // start bit
	for (i=0; i<8; i++) 
		if ((c >> i) & 1) *pattern_data = 0; else *pattern_data = 1; // put character as serial data in pattern generator
	*pattern_data = 0; // stop bit
	*pattern_data = 0; // stop bit
	*pattern_control = 0;
	*pattern_control = 8; // soft trigger
writechar_rs232module(c); // transmit character also with simple rs232 module, just for testing
	return 0;
}



// send a string over the rs232 line
int writestring(char *s) {// send characters, return 0 on success
	int i=0;
	while (s[i]) { if (writechar(s[i++])) return -1; }
	return 0;
}

// send received timestamp data over the rs232 line
char *int2hex(unsigned int w, int l) {
	static char buf[9];
	int i=0;
	char c;
	char *p = buf;
	for (i=0; i<l; i++) {
		c=(w >> ((l-i-1)*4)) & 0xf;
		if (c<10) buf[i]=c+'0'; else buf[i]=c-10+'A';
	}
	buf[l]=0;
	return p;
}


// send received timestamp data over the rs232 line
void printhex(unsigned int hw, unsigned int lw, unsigned int cw) {
	int i=0;
	char c;
	unsigned int n = 0;
	if (cw & 0x02) { // error detected in serially sent timestamp
		c='X';
	} else if (cw & 0x04) { // error succesfully corrected in serially sent timestamp
		c='`';
	} else {
		c=' ';
	}
	if (writechar(c)) return;
	
	for (i=0; i<18; i++) {
		if (i<8) {
			n=(hw >> ((7-i)*4)) & 0xf;
			if (n<10) c=n+'0';
			else c=n-10+'A';
		} 
		else if (i<16) {
			n=(lw >> ((7-i)*4)) & 0xf;
			if (n<10) c=n+'0';
			else c=n-10+'A';
		} 
		else if (i==16) c=13; 
		else c=10;
		if (writechar(c)) return;
	}
}




// write pattern into memory
void load_pattern(unsigned int *pattern, int nrofwords, int period) {
	unsigned int *p=pattern;
	int i;
	*pattern_period = period;
	*pattern_control = 3;
	for (i=0; i<nrofwords; i++) *pattern_data = *p++;
	*pattern_control = 1;
}

void sleep1s(void)
{
	int j;
	for (j = 0; j < 125000000/4; ++j) {
		asm("# noop"); /* no-op the compiler can't optimize away */
	}
}

unsigned char invbyte(unsigned char bt)
{
	unsigned char b=0;
	int i;
	for (i=0; i<8; i++) if (bt & (1<<i)) b |= 1<<(7-i);
	return b;
}
			
void main(void) {
	int i, j;
	int phase=0;
	int nrofwords=10;
	unsigned int param,adr;
	int rval;
	unsigned char bf[256];
	unsigned char bt;
	unsigned int pattern[10] = {0xff,0,0xff,0,0x55,0xaa,0,0xff,0xff,0};
	
	// initialize single pulse generator
	*singlepulse_control = 1; // enable
	*singlepulse_delay = 100; // delay 100 clock-cycles
	*singlepulse_duration = 200; // pulse-width 200 clock-cycles
	
	*rs232_control = 1;	// set baudrate to 115k2
	*readtime_control = 8; // clear timpestamp receiver error counters
	*BuTiSclock_control = 0x2; // start re-synchronizing on PPS
	for (j = 0; j < 1250000/4/5; ++j)  // 0.002s
		asm("# noop"); /* no-op the compiler can't optimize away */
	*BuTiSclock_control = 0x4; // reset phase PLL
	*BuTiSclock_lw = 0x00000000; // timestamp low-word
	*BuTiSclock_hw = 0x00000000; // timestamp high-word
	*BuTiSclock_control = 0x1; // set the timestamp (hw & lw) on the next PPS-pulse
	phase=0;
	*BuTiSclock_control = phase << 8; // set phase
	*pattern_period = 1085; // pattern period : 1085/125MHz => 8.68us => 115k2 baud
	
	// example how to load a pattern
	// this is now overwritten by the rs232 data in writechar()
	load_pattern(pattern,nrofwords,1085); // load pattern

	writestring("-------------------- start ----------------------\r\n");
	writestring("status: "); rval=read_status(); writestring(int2hex(rval,2)); writestring("\r\n"); 

	rval=factory_mode();
	if (rval<0) { 
		writestring("factory_mode returns "); writestring(itoa(rval)); writestring("\r\n"); 
	} else if (rval==0) {
		writestring("application mode\r\n"); 
	} else {
		writestring("factory mode\r\n"); 
	}
	writestring("reset !!!!!!!!\r\n");
	reset_flashupdate_module()	;
	writestring("status: "); rval=read_status(); writestring(int2hex(rval,2)); writestring("\r\n"); 

	
	adr=0x00000000;
//	rval=write_flash_parameter(4,adr>>16);
//	if (rval) { writestring("write_flash_parameter returns "); writestring(itoa(rval)); writestring("\r\n"); }
//	rval=read_flash_parameter(4,&param);
//	if (rval) { writestring("read_flash_parameter returns "); writestring(itoa(rval)); writestring("\r\n"); }
//	writestring("param[4]="); writestring(int2hex(param,6)); writestring("\r\n");

	
	rval=read_flash_parameter(0,&param);
	if (rval) { writestring("read_flash_parameter returns "); writestring(itoa(rval)); writestring("\r\n"); }
	writestring("param[0]="); writestring(int2hex(param,6)); writestring("\r\n");

	rval=read_flash_parameter(2,&param);
	if (rval) { writestring("read_flash_parameter returns "); writestring(itoa(rval)); writestring("\r\n"); }
	writestring("param[2]="); writestring(int2hex(param,6)); writestring("\r\n");

	rval=read_flash_parameter(3,&param);
	if (rval) { writestring("read_flash_parameter returns "); writestring(itoa(rval)); writestring("\r\n"); }
	writestring("param[3]="); writestring(int2hex(param,6)); writestring("\r\n");

	rval=read_flash_parameter(4,&param);
	if (rval) { writestring("read_flash_parameter returns "); writestring(itoa(rval)); writestring("\r\n"); }
	writestring("param[4]="); writestring(int2hex(param,6)); writestring("\r\n");
	
	rval=read_flash_parameter(5,&param);
	if (rval) { writestring("read_flash_parameter returns "); writestring(itoa(rval)); writestring("\r\n"); }
	writestring("param[5]="); writestring(int2hex(param,6)); writestring("\r\n");

	rval=read_flash_id(&bt);
	if (rval) { writestring("read_flash_id returns "); writestring(itoa(rval)); writestring("\r\n"); }
	writestring("flash ID="); writestring(int2hex(bt,2)); writestring("\r\n");

	rval=read_flash_status(&bt);
	if (rval) { writestring("read_flash_status returns "); writestring(itoa(rval)); writestring("\r\n"); }
	writestring("flash status="); writestring(int2hex(bt,2)); writestring("\r\n");

	writestring("status: "); rval=read_status(); writestring(int2hex(rval,2)); writestring("\r\n"); 

/*
	writestring("start erase_factory_flash\r\n");
	rval=erase_factory_flash();
	if (rval) { writestring("erase_factory_flash returns "); writestring(itoa(rval)); writestring("\r\n"); }
	writestring("\r\nerase_factory_flash done\r\n");
	
	writestring("start erase_application_flash\r\n");
	rval=erase_application_flash();
	if (rval) { writestring("erase_application_flash returns "); writestring(itoa(rval)); writestring("\r\n"); }
	writestring("\r\nerase_application_flash done\r\n");

	adr=0x00800000;

	rval=erase_flash_sector(adr);
	if (rval) { writestring("erase_flash_sector returns "); writestring(itoa(rval)); writestring("\r\n"); }
	writestring("\r\nerase_flash_sector done\r\n");
	for (i=0; i<256; i++) bf[i]=i;
	adr=0x00800000;
	rval=write_flash(adr,bf,8);
	if (rval) { writestring("write_flash returns "); writestring(itoa(rval)); writestring("\r\n"); }

*/
/*
	writestring("erase_flash\r\n");	
	for (i=0; i<FLASHSIZE/SECTORSIZE; i++) {
writestring(".");
		adr=i*SECTORSIZE;
		rval=erase_flash(adr);
		if (rval) { writestring("\r\nerase_flash returns "); writestring(itoa(rval)); writestring("\r\n"); }
		if (*flash_parameters_read & 0x40000000) { writestring("\r\nerase_flash error : "); writestring(int2hex(*flash_parameters_read,8));	 writestring("\r\n"); }
	}
writestring("\r\n");
*/

/*
adr=0x00800000;
rval=erase_flash(adr);
if (rval) { writestring("\r\nerase_flash returns "); writestring(itoa(rval)); writestring("\r\n"); }
writestring("\r\nflash_parameters_read=");		
writestring(int2hex(*flash_parameters_read,8));	
*/
/*
adr=0x00800000;
rval=erase_flash(adr);
if (rval) { writestring("\r\nerase_flash returns "); writestring(itoa(rval)); writestring("\r\n"); }
writestring("\r\nflash_parameters_read=");		
writestring(int2hex(*flash_parameters_read,8));		

	adr=0x00000000;
	for (i=0; i<20; i++) {
		writestring("\r\nadr["); writestring(int2hex(adr,6)); writestring("]: ");
		for (j=0; j<16; j++) {
			rval=read_flash(adr,&bt);
			if (rval) { writestring("\r\nread_flash returns "); writestring(itoa(rval)); writestring("\r\n"); }
			writestring(" "); writestring(int2hex(bt,2)); 
			adr++;
		}
	}

	
	adr=0x00000000;
	for (i=0; i<20; i++) {
		writestring("\r\nadr["); writestring(int2hex(adr,6)); writestring("]: ");
		for (j=0; j<16; j++) {
			rval=read_flash(adr,&bt);
			if (rval) { writestring("\r\nread_flash returns "); writestring(itoa(rval)); writestring("\r\n"); }
			writestring(" "); writestring(int2hex(bt,2)); 
			adr++;
		}
	}

	adr=SECTORSIZE; // -40*16;
	for (i=0; i<20; i++) {
		writestring("\r\nadr["); writestring(int2hex(adr,6)); writestring("]: ");
		for (j=0; j<16; j++) {
			rval=read_flash(adr,&bt);
			if (rval) { writestring("\r\nread_flash returns "); writestring(itoa(rval)); writestring("\r\n"); }
			writestring(" "); writestring(int2hex(bt,2)); 
			adr++;
		}
	}
*/


	adr=0x00800000;
	for (i=0; i<10; i++) {
		rval=read_flash(adr,bf,256);
		if (rval) { writestring("\r\nread_flash returns "); writestring(itoa(rval)); writestring("\r\n"); }
		for (j=0; j<256; j++) {
			if ((adr & 0xf) == 0) { writestring("\r\nadr["); writestring(int2hex(adr,6)); writestring("]: "); }
			writestring(" "); writestring(int2hex(bf[j],2)); // writestring(int2hex(invbyte(bt),2)); 
			adr++;
		}
	}
	writestring("\r\n");

	writestring("status: "); rval=read_status(); writestring(int2hex(rval,2)); writestring("\r\n"); 
//	writestring("\r\nstart_reconfiguration\r\n"); start_reconfiguration();
	writestring("status: "); rval=read_status(); writestring(int2hex(rval,2)); writestring("\r\n"); 
	
	rval=read_flash_parameter(4,&param);
	if (rval) { writestring("read_flash_parameter returns "); writestring(itoa(rval)); writestring("\r\n"); }
	writestring("param[4]="); writestring(int2hex(param,6)); writestring("\r\n");

	rval=factory_mode();
	if (rval<0) { 
		writestring("factory_mode returns "); writestring(itoa(rval)); writestring("\r\n"); 
	} else if (rval==0) {
		writestring("application mode\r\n"); 
	} else {
		writestring("factory mode\r\n"); 
	}

	
	writestring("\r\nStart while loop\r\n");
	while (1) {

//		rval=read_flash_id(&bt);
//		if (rval) { writestring("read_flash_id returns "); writestring(itoa(rval)); writestring("\r\n"); }
//		writestring("flash ID="); writestring(int2hex(bt,2)); writestring("\r\n");
		for (i = 0; i < 8; ++i) {
			
			/* Rotate the LEDs */
			*leds = 1 << i;
			*singlepulse_delay = 100+i*10;
			*singlepulse_duration = 200+i*20;

			/* Each loop iteration takes 4 cycles.
			* It runs at 125MHz.
			* Sleep 0.2 second.
			*/
			for (j = 0; j < 125000000/4/5; ++j) {
				asm("# noop"); /* no-op the compiler can't optimize away */
			}
		}
	}
}
