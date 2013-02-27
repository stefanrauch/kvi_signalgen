// address for LED register
volatile unsigned int* leds = (unsigned int*)0x100400;

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

void _read(void) {}
void isatty(void) {}
void _sbrk(void) {}
void _write(void) {}
void _close(void) {}
void _fstat(void) {}
void _lseek(void) {}


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
void printhex(unsigned int hw, unsigned int lw, unsigned int cw) {
	int i=0;
	char c;
	unsigned int n;
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

void main(void) {
	int i, j;
	int phase=0;
	unsigned int errors=0;
	unsigned int corrections=0;
	unsigned int hw,lw,cw,phasestat,prev_phasestat;
	int nrofwords=10;
	int n=0; 
	int phasedownwards=0;
	char k;
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
	writestring("Start while loop\n");
	
	while (1) {

		for (i = 0; i < 8; ++i) {
		
			// print errors/correction if amount has changed
			if ((*readtime_errors!=errors) || (*readtime_corrections!=corrections)) {
				errors=*readtime_errors;
				corrections=*readtime_corrections;
				writestring("errors=");
				writestring(itoa(*readtime_errors));
				writestring("  corrections=");
				writestring(itoa(*readtime_corrections));
				writestring("\n\r");
			}
			
			// reading of BuTiS received timestamp and error counters
			*readtime_control = 0x1; // disable updating timestamp reading registers
			hw = *readtime_highword;
			lw = *readtime_lowword;
			cw = *readtime_control;
			*readtime_control = 0x0; // enable updating timestamp reading registers
			printhex(hw,lw,cw); // send over rs232
			
			// change phase upwards and downwards for testing behaviour
			phasestat=*BuTiSclock_status & 0x3;
			if ((phasestat & 0x2) && (!(prev_phasestat & 0x2))) {
				if (phasedownwards) {
					if (--phase<=0) phasedownwards=0;
				}
				else {
					if (++phase>=27) phasedownwards=1;
				}
				*BuTiSclock_control = phase << 8;
				n=0;
				writestring("phase="); 
				writestring(itoa(phase)); // send actual phase over rs232
				writestring("\n\r");
			}
			
			// check PPS : first half second or second half second
			prev_phasestat=phasestat;
			switch(phasestat) {
				case 0x0 : k=' '; break; // first half second of PPS phase
				case 0x1 : k='s'; break; // first half second of PPS phase and timestamp setting waiting for PPS
				case 0x2 : k='.'; break; // second half second of PPS phase
				case 0x3 : k='S'; break; // second half second of PPS phase and timestamp setting waiting for PPS
				default  : k='*';
			}
			if (writechar(k)) return;
			
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
