/** @file eb-loadflash.c
 *  @brief A program which loads a file to a device.
 *
 *  Copyright (C) 2011-2012 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 *  A complete skeleton of an application using the Etherbone library.
 *
 *  @author Wesley W. Terpstra <w.terpstra@gsi.de>
 *  adjusted for programming flash on Pexaria2a Pcie card by Peter Schakel <p.schakel@rug.nl>
 *
 *  @bug None!
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

#define _POSIX_C_SOURCE 200112L /* strtoull */

#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>



#include "../etherbone.h"
#include "../glue/version.h"
#include "common.h"

#define OPERATIONS_PER_CYCLE 256

#define FLASHSIZE 16777216
#define SECTORSIZE 65536
#define APPICATIONFLASHADDRESS 0x00800000

// addresses for flash
#define FLASH_PARAMETERS 0x0 
	// 24-bits data,3-bits address,write,request,reconf:0b101
	
#define FLASH_PARAMETERS_READ 0x4
	// 24-bits data,3-bits address,busy,error,illegal
	
#define FLASH_DATA 0x8
	// 8-bits data, 24-bits address
	
#define FLASH_READ 0xc
	// 8-bits data,valid,busy,error
	
#define FLASH_ACCESS 0x10
	// enable,read_enable, write_enable on 0b101, erase_enable on 0b101, read_id, read_status

unsigned long long strtoull (const char * nptr, char ** endptr, int base);
extern int usleep (__useconds_t __useconds);

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <proto/host/port> <baseaddress> <flashaddress> <firmware>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -a <width>     acceptable address bus widths     (8/16/32/64)\n");
  fprintf(stderr, "  -d <width>     acceptable data bus widths        (8/16/32/64)\n");
  fprintf(stderr, "  -b             big-endian operation                    (auto)\n");
  fprintf(stderr, "  -l             little-endian operation                 (auto)\n");
  fprintf(stderr, "  -r <retries>   number of times to attempt autonegotiation (3)\n");
  fprintf(stderr, "  -f             force; ignore remote segfaults\n");
  fprintf(stderr, "  -p             disable self-describing wishbone device probe\n");
  fprintf(stderr, "  -v             verbose operation\n");
  fprintf(stderr, "  -c <cycles>    read cycles per verbose operation\n");
  fprintf(stderr, "  -q             quiet: do not display warnings\n");
  fprintf(stderr, "  -m             mirror byte: reverse bits, needed for Altera rbf-files\n");
  fprintf(stderr, "  -h             display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report Etherbone bugs to <etherbone-core@ohwr.org>\n");
  fprintf(stderr, "Version %"PRIx32" (%s). Licensed under the LGPL v3.\n", EB_VERSION_SHORT, EB_DATE_FULL);
}

static FILE* firmware_f;
static const char* firmware;
static int bitreverse;

static eb_data_t readdata;
static void set_stop_read(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status) {
  int* stop = (int*)user;
  *stop = 1;
  
  if (status != EB_OK) {
    fprintf(stderr, "%s: etherbone cycle error_1: %s\n", 
                    program, eb_status(status));
    exit(1);
  } else {
    readdata = 0;
    for (; op != EB_NULL; op = eb_operation_next(op)) {
      /* We read low bits first */
      readdata <<= (eb_operation_format(op) & EB_DATAX) * 8;
      readdata |= eb_operation_data(op);
      
      if (eb_operation_had_error(op))
        fprintf(stderr, "%s: wishbone segfault reading %s %s bits from address 0x%"EB_ADDR_FMT"\n",
                        program, width_str[eb_operation_format(op) & EB_DATAX], 
                        endian_str[eb_operation_format(op) >> 4], eb_operation_address(op));
    }
  }
}

static void set_stop_write(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status) {
	int* stop = (int*)user;
	*stop = 1;
	if (status != EB_OK) {
		fprintf(stderr, "%s: etherbone cycle error_2: %s\n", 
		program, eb_status(status));
		exit(1);
	}
	for (; op != EB_NULL; op = eb_operation_next(op)) {
		if (eb_operation_had_error(op)) {
			fprintf(stderr, "%s: wishbone segfault %s %s %s bits to address 0x%"EB_ADDR_FMT"\n",
				program, eb_operation_is_read(op)?"reading":"writing",
				width_str[eb_operation_format(op) & EB_DATAX], 
				endian_str[eb_operation_format(op) >> 4], eb_operation_address(op));
			exit(1);
		}
	}
}

// Bit-reverse one byte, needed for Altera .rbf files
static unsigned char invbyte(unsigned char bt)
{
	unsigned char b=0;
	int i;
	for (i=0; i<8; i++) if (bt & (1<<i)) b |= 1<<(7-i);
	return b;
}

static int force;
static eb_socket_t socket;

static unsigned int eb_read(eb_device_t device, eb_address_t address, eb_format_t format)
{
	eb_cycle_t cycle;
	eb_status_t status;
	int stop;
	/* Begin the cycle */
	if ((status = eb_cycle_open(device, &stop, &set_stop_read, &cycle)) != EB_OK) {
		fprintf(stderr, "%s: failed to create cycle: %s\n", program, eb_status(status));
		exit(1);
	}
	eb_cycle_read(cycle, address, format, 0);
	if (force) eb_cycle_close_silently(cycle);
	else eb_cycle_close(cycle);
	stop = 0;
	eb_device_flush(device);
	while (!stop) { eb_socket_run(socket, -1); }
	return (unsigned int) readdata;
}

static void eb_write(eb_device_t device, eb_address_t address, eb_format_t format, unsigned int data)
{
	eb_cycle_t cycle;
	eb_status_t status;
	int stop;
	/* Begin the cycle */
	if ((status = eb_cycle_open(device, &stop, &set_stop_write, &cycle)) != EB_OK) {
		fprintf(stderr, "%s: failed to create cycle: %s\n", program, eb_status(status));
		exit(1);
	}
	eb_cycle_write(cycle, address, format, (eb_data_t) data);
	if (force) eb_cycle_close_silently(cycle);
	else eb_cycle_close(cycle);
	stop = 0;
	eb_device_flush(device);
	while (!stop) { eb_socket_run(socket, -1); }
}
  
// Read one of the parameters of the FPGA altremote_update component
// altremote_update parameters, defined by address:
// address=0 : read reconfiguration condition: bit4..0=WatchDog, external nCONFIG, by ArriaII self, error by nSTATUS, CRC error
// address=2 : rd/wr watchdog timer value, 12 bits
// address=3 : enable watchdog timer, bit0
// address=4 : rd/wr page select : bit6..0 = flash_address(22..16)  // for Active Serial
// address=5 : read application mode (= not factory mode), bit 0 
//   Parameters :
//      eb_device_t device : Etherbone device
//      eb_address_t baseaddress : Base address of the wishbone update_flash module
//      eb_format_t format : Format of the Etherbone bus access
//      unsigned int index : parameter index to read (0..7)
//      return : parameter value
static unsigned int read_flash_parameter(eb_device_t device, eb_address_t baseaddress, eb_format_t format, unsigned int index) {
	unsigned int bf,parbf;
	int timeout=0;
	do {
		bf=eb_read(device,baseaddress+FLASH_READ,format);
	} while ((bf & 0x08000000) && (timeout++<1000)); // wait till busy=0
	parbf = ((index & 0x7) << 24) | (0x10000000); // 3 bits parameter index plus bit28=read request;
	eb_write(device,baseaddress+FLASH_PARAMETERS,format,parbf);
	timeout=0;
	do {
		bf=eb_read(device,baseaddress+FLASH_PARAMETERS_READ,format);
	} while ((bf & 0x08000000) && (timeout++<1000)); // wait till busy=0
	if (((bf & 0x10000000)!=0) || (((bf >> 24) & 0x7) != index)) { 
		fprintf(stderr, "%s: warning: invalid flash image detected \n", program);
	}
	if (((bf & 0x20000000)!=0) || (((bf >> 24) & 0x7) != index)) { 
		fprintf(stderr, "%s: warning: illegal flash write detected\n", program);
	}
	if (((bf >> 24) & 0x7) != index) { 
		fprintf(stderr, "%s: error reading flash parameter\n", program);
		exit(1); 
	}
	return  bf & 0x00ffffff;
}

// Erase one sector in the flash. Sector size is defined by SECTORSIZE, depends on flash-type, probably 64k
//   Parameters :
//      eb_device_t device : Etherbone device
//      eb_address_t baseaddress : Base address of the wishbone update_flash module
//      unsigned long flash_address : Address of the sector to be erased in the Pexaria2a flash
//      eb_format_t format : Format of the Etherbone bus access
//      eb_device_t device : Etherbone device
static void erase_flash_sector(eb_device_t device, eb_address_t baseaddress, unsigned long flash_address, eb_format_t format) {
	unsigned int bf;
	int timeout=0;
	if (verbose) fprintf(stdout,"\rErase segment 0x%"EB_ADDR_FMT"... ",(int) flash_address);
	fflush(stdout);
	do {
		bf=eb_read(device,baseaddress+FLASH_READ,format);
	} while ((bf & 0x00000200) && (timeout++<1000)); // wait till busy=0
	if (bf & 0x00000200) { // error: still busy
		fprintf(stderr, "%s: error accessing flash: still busy\n", program);
		exit(1);
	}
	eb_write(device,baseaddress+FLASH_ACCESS,format,0x000000a1);
	eb_write(device,baseaddress+FLASH_DATA,format,flash_address<<8); // address in bits 31..8
	timeout=0;
	do {
		usleep(100);
		bf=eb_read(device,baseaddress+FLASH_READ,format);
	} while ((bf & 0x00000200) && (timeout++<5000)); // wait till busy=0
	if ((bf & 0x00000200)!=0) { 
		fprintf(stderr, "%s: error accessing flash: still busy\n", program);
		exit(1);
	}
	eb_write(device,baseaddress+FLASH_ACCESS,format,0x00000000);
}

// Erase a part of the flash
//   Parameters :
//      eb_device_t device : Etherbone device
//      eb_address_t baseaddress : Base address of the wishbone update_flash module
//      unsigned int startaddress : Address of first sector to be erased in the Pexaria2a flash
//      unsigned int size : Size of the part in the Pexaria2a flash that has to be erased
//      eb_format_t format : Format of the Etherbone bus access
static void erase_flash(eb_device_t device, eb_address_t baseaddress, unsigned int startaddress, eb_format_t format, unsigned int size) 
{
	unsigned int adr;
	for (adr=startaddress; adr<startaddress+size; adr+=SECTORSIZE) {
		if (adr>=FLASHSIZE) return; // ready if last sector has been reached
		erase_flash_sector(device,baseaddress,adr,format);
		if (eb_read(device,baseaddress+FLASH_PARAMETERS_READ,format) & 0x40000000) {
			fprintf(stderr, "%s: error erasing flash\n", program);
			exit(1);
		}
	}
}

// Checks if the flash is accessible and that the right flash is connected
// The parameters which must be checked are still unknown
// his function is now only used to display some parameters
//   Parameters :
//      eb_device_t device : Etherbone device
//      eb_address_t baseaddress : Base address of the wishbone update_flash module
//      eb_format_t format : Format of the Etherbone bus access
//      return : zero on ok
static int check_flash(eb_device_t device, eb_address_t baseaddress, eb_format_t format)
{
	int rval;
	rval=read_flash_parameter(device,baseaddress,format,5);
	if (verbose) {
		if (rval==0) fprintf(stdout,"application mode\n"); else fprintf(stdout,"factory mode\n"); 
	}
	rval=read_flash_parameter(device,baseaddress,format,0);
	if (verbose) fprintf(stdout,"Flash parameter nr %d : %08x\n",0,rval);
	rval=read_flash_parameter(device,baseaddress,format,2);
	if (verbose) fprintf(stdout,"Flash parameter nr %d : %08x\n",2,rval);
	rval=read_flash_parameter(device,baseaddress,format,3);
	if (verbose) fprintf(stdout,"Flash parameter nr %d : %08x\n",3,rval);
	rval=read_flash_parameter(device,baseaddress,format,4);
	if (verbose) fprintf(stdout,"Flash parameter nr %d : %08x\n",4,rval);
	rval=read_flash_parameter(device,baseaddress,format,5);
	if (verbose) fprintf(stdout,"Flash parameter nr %d : %08x\n",5,rval);
	
//	rval=read_flash_id(device,baseaddress,format);
//	if (verbose) fprintf(stdout,"Flash identification number : %0.8x\n",rval);
//	rval=read_flash_status(device,baseaddress,format);
//	if (verbose) fprintf(stdout,"Flash pstatus : %0.8x\n",rval);
	return 0; // zero: ok
}

// Transfer data from a file (firmware_f) to the pexaria2a flash
//   Parameters :
//      eb_device_t device : Etherbone device
//      eb_address_t baseaddress : Base address of the wishbone update_flash module
//      unsigned long flash_address : Address in the flash to write the data to
//      eb_format_t format : Format of the Etherbone bus access
//      int count : Number of bytes to write
static void transfer(eb_device_t device, eb_address_t baseaddress, unsigned long flash_address, eb_format_t format, int count) {
  eb_data_t data;
  eb_format_t size;
  uint8_t buffer[16];
  int i, timeout;
  eb_data_t bf;
	size = 1; // only byte by byte

	timeout=0;
	do {
		bf=eb_read(device,baseaddress+FLASH_READ,format);
	} while ((bf & 0x00000200) && (timeout++<1000)); // wait till busy=0
	if (bf & 0x00000200) {
		fprintf(stderr, "\r%s: flash busy timeout '%s'\n",program, firmware);
		exit(1); // error: still busy
	}
 		// enable writing to flash with bit0=1 (access enable) and bit4..2=101 (write enable) :
	eb_write(device,baseaddress+FLASH_ACCESS,format,0x00000015);
	for (i = 0; i < count; ++i) {
		if (fread(buffer, 1, size, firmware_f) != size) {
			fprintf(stderr, "\r%s: short read from '%s'\n",program, firmware);
			exit(1);
		}
		/* Construct value */
		if (bitreverse)
			data = (flash_address<<8) | (unsigned int) (invbyte(buffer[0])); // address in bits 31..8, data in bits 7..0, invert bytes from rbf file
		else 
			data = (flash_address<<8) | (unsigned int)(buffer[0]); // address in bits 31..8, data in bits 7..0, invert bytes from rbf file
		eb_write(device,baseaddress+FLASH_DATA,format,data);
	}
	eb_write(device,baseaddress+FLASH_ACCESS,format,0x00000000); // disable writing; this will start writing process
	timeout=0;
	do {
		bf=eb_read(device,baseaddress+FLASH_READ,format);
	} while ((bf & 0x00000200) && (timeout++<1000000)); // wait till busy=0
	if ((bf & 0x00000200)!=0) {
		fprintf(stderr, "\r%s: error accessing flash\n", program);
		exit(1);
	}
}
  
int main(int argc, char** argv) {
  long value;
  char* value_end;
  int opt, cycle, error;
  
  eb_status_t status;
  eb_device_t device;
  eb_width_t line_width;
  eb_format_t line_widths;
  eb_format_t device_support;
  eb_format_t write_sizes;
  eb_format_t format;
  eb_format_t size;
  eb_address_t end_address, step, baseaddress;

  
  /* Specific command-line options */
  int attempts, probe, cycles;
  const char* netaddress;
  eb_address_t firmware_length;
  
  unsigned int flashaddress;
  
  /* Default arguments */
  program = argv[0];
  address_width = EB_ADDRX;
  data_width = EB_DATAX;
  endian = 0; /* auto-detect */
  attempts = 3;
  probe = 1;
  quiet = 0;
  verbose = 0;
  error = 0;
  cycles = 100;
  force = 0;
  size = 4;
  
  /* Process the command-line arguments */
  while ((opt = getopt(argc, argv, "a:d:c:blr:fpvqmh")) != -1) {
    switch (opt) {
    case 'a':
      value = parse_width(optarg);
      if (value < 0) {
        fprintf(stderr, "%s: invalid address width -- '%s'\n", program, optarg);
        return 1;
      }
      address_width = value << 4;
      break;
    case 'd':
      value = parse_width(optarg);
      if (value < 0) {
        fprintf(stderr, "%s: invalid data width -- '%s'\n", program, optarg);
        return 1;
      }
      data_width = value;
      break;
    case 'c':
      value = strtol(optarg, &value_end, 0);
      if (*value_end || cycles < 0 || cycles > 100) {
        fprintf(stderr, "%s: invalid cycle count -- '%s'\n", program, optarg);
        return 1;
      }
      cycles = value;
      break;
    case 'b':
      endian = EB_BIG_ENDIAN;
      break;
    case 'l':
      endian = EB_LITTLE_ENDIAN;
      break;
    case 'r':
      value = strtol(optarg, &value_end, 0);
      if (*value_end || value < 0 || value > 100) {
        fprintf(stderr, "%s: invalid number of retries -- '%s'\n", program, optarg);
        return 1;
      }
      attempts = value;
      break;
    case 'f':
      force = 1;
      break;
    case 'p':
      probe = 0;
      break;
    case 'v':
      verbose = 1;
      break;
    case 'q':
      quiet = 1;
      break;
    case 'm':
      bitreverse = 1;
      break;
    case 'h':
      help();
      return 1;
    case ':':
    case '?':
      error = 1;
      break;
    default:
      fprintf(stderr, "%s: bad getopt result\n", program);
      return 1;
    }
  }
  
  if (error) return 1;
  
  if (optind + 4 != argc) {
    fprintf(stderr, "%s: expecting four non-optional arguments: <proto/host/port> <baseaddress> <flashaddress> <firmware>\n", program);
    return 1;
  }
  
  netaddress = argv[optind];

  baseaddress = strtoull(argv[optind+1], &value_end, 0);
  if (*value_end != 0) {
    fprintf(stderr, "%s: argument is not an unsigned value -- '%s'\n",
                    program, argv[optind+1]);
    return 1;
  }

  
  flashaddress = strtoull(argv[optind+2], &value_end, 0);
  if (*value_end != 0) {
    fprintf(stderr, "%s: argument is not an unsigned value -- '%s'\n",
                    program, argv[optind+2]);
    return 1;
  }
  
  firmware = argv[optind+3];
  if ((firmware_f = fopen(firmware, "r")) == 0) {
    fprintf(stderr, "%s: fopen, %s -- '%s'\n",
                    program, strerror(errno), firmware);
    return 1;
  }
  
  if (fseek(firmware_f, 0, SEEK_END) != 0) {
    fprintf(stderr, "%s: fseek, %s -- '%s'\n",
                    program, strerror(errno), firmware);
  }
  
  firmware_length = ftell(firmware_f);
  rewind(firmware_f);
  
  if (verbose)
    fprintf(stdout, "Opening socket with %s-bit address and %s-bit data widths\n", 
                    width_str[address_width>>4], width_str[data_width]);
  
  if ((status = eb_socket_open(EB_ABI_CODE, 0, address_width|data_width, &socket)) != EB_OK) {
    fprintf(stderr, "%s: failed to open Etherbone socket: %s\n", program, eb_status(status));
    return 1;
  }
  
  if (verbose)
    fprintf(stdout, "Connecting to '%s' with %d retry attempts...\n", netaddress, attempts);
  
  if ((status = eb_device_open(socket, netaddress, EB_ADDRX|EB_DATAX, attempts, &device)) != EB_OK) {
    fprintf(stderr, "%s: failed to open Etherbone device: %s\n", program, eb_status(status));
    return 1;
  }
  
  line_width = eb_device_width(device);
  if (verbose)
    fprintf(stdout, "  negotiated %s-bit address and %s-bit data session.\n", 
                    width_str[line_width >> 4], width_str[line_width & EB_DATAX]);
  
  address=baseaddress;
  if (probe) {
    if (verbose)
      fprintf(stdout, "Scanning remote bus for Wishbone devices...\n");
    device_support = 0;
    if ((status = eb_sdb_scan_root(device, &device_support, &find_device)) != EB_OK) {
      fprintf(stderr, "%s: failed to scan remote bus: %s\n", program, eb_status(status));
    }
    while (device_support == 0) {
      eb_socket_run(socket, -1);
    }
  } else {
    device_support = endian | EB_DATAX;
  }
  
  /* Did the user request a bad endian? We use it anyway, but issue warning. */
  if (endian != 0 && (device_support & EB_ENDIAN_MASK) != endian) {
    if (!quiet)
      fprintf(stderr, "%s: warning: target device is %s (writing as %s).\n",
                      program, endian_str[device_support >> 4], endian_str[endian >> 4]);
  }
  
  if (endian == 0) {
    /* Select the probed endian. May still be 0 if device not found. */
    endian = device_support & EB_ENDIAN_MASK;
  }
  
  /* We need to know endian if it's not aligned to the line size */
  if (endian == 0) {
    fprintf(stderr, "%s: error: must know endian to program firmware\n",program);
    return 1;
  }
  
  /* We need to pick the operation width we use.
   * It must be supported both by the device and the line.
   */
  line_widths = ((line_width & EB_DATAX) << 1) - 1; /* Link can support any access smaller than line_width */
  write_sizes = line_widths & device_support;
    
  /* We cannot work with a device that requires larger access than we support */
  if (write_sizes == 0) {
    fprintf(stderr, "%s: error: device's %s-bit data port cannot be used via a %s-bit wire format\n",
                    program, width_str[device_support & EB_DATAX], width_str[line_width & EB_DATAX]);
    return 1;
  }

  /* Final operation endian has been chosen. If 0 the access had better be a full data width access! */
  format = endian;

  if (verbose) fprintf(stdout, "Programming using batches of %d bytes\n",OPERATIONS_PER_CYCLE);

  /* Can the operation be performed with fidelity? */
  if ((size & write_sizes) == 0) {
    fprintf(stderr, "%s: error: unsupported bus width\n",program);
	exit(1);
  }
  format |= (size & write_sizes);
  end_address = flashaddress + firmware_length;

  // check registers
  if (check_flash(device,baseaddress,format)) {
    fprintf(stderr, "%s: error: reading from flash\n",program);
	exit(1);
  }

  // erase flash first
  erase_flash(device,baseaddress,flashaddress,format,firmware_length);
  if (verbose) fprintf(stdout, "\n");
   
  /* Begin the transfer */
  for (cycle = 0; flashaddress < end_address; flashaddress += step) {
    step = end_address - flashaddress;
    if (step > OPERATIONS_PER_CYCLE) step = OPERATIONS_PER_CYCLE;
    transfer(device, baseaddress, flashaddress, format, step);
    
    /* Flush? */
    if (++cycle == cycles) {
      if (verbose) {
        fprintf(stdout, "\rProgramming 0x%"EB_ADDR_FMT"... ", flashaddress);
        fflush(stdout);
      }
      cycle = 0;
    }
  }
  
  if (verbose) fprintf(stdout, "\ndone!\n");

  
  if ((status = eb_device_close(device)) != EB_OK) {
    fprintf(stderr, "%s: failed to close Etherbone device: %s\n", program, eb_status(status));
    return 1;
  }
  
  if ((status = eb_socket_close(socket)) != EB_OK) {
    fprintf(stderr, "%s: failed to close Etherbone socket: %s\n", program, eb_status(status));
    return 1;
  }
  
  return 0;
}
