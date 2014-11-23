/*
 * APIPAGE.C - ATmega tinyUSBboard version
 * 
 * THIS FILE DOES NOT IMPLEMENT STUFF FOR THE PAGE
 * ITSELF. CONSULT OTHER SOURCES INSTEAD!
 * 
 * needs USBaspLoader with spminterface support
 * installed on the device !
 * 
 * 
 * This is version 20141122T2000ZSB
 * 
 * Stephan Baerwolf (matrixstorm@gmx.de), Ilmenau 2014
 */

#define __APIPAGE_C_6e4723fa71ec40528d1ab16d9ff806d7 1

#include "apipage.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <avr/boot.h>

#ifdef __AVR_XMEGA__
#	error XMega architecture not supported
#endif

#define	apipage_pagefillcode	(_BV(SPMEN))
#define apipage_pageerasecode	((_BV(PGERS)) | (_BV(SPMEN)))
#define	apipage_pagewritecode	((_BV(PGWRT)) | (_BV(SPMEN)))

#ifndef NVM_CMD_NO_OPERATION_gc
#	define NVM_CMD_NO_OPERATION_gc 0x00
#endif

uint8_t flash_read_Ex(const uint_farptr_t address, const uint8_t nvmCommand) {
  uint8_t result;

  asm volatile (
#ifdef __AVR_HAVE_RAMPZ__
    "lds	r17		,	%[rampz]	\n\t"
    "sts	%[rampz]	,	%[anyregister]	\n\t"
#endif
    "sts	%[nvmcmdreg]	,	%[nvmcmd]	\n\t"
#ifdef __AVR_HAVE_ELPM__
    "elpm						\n\t"
#else
    "lpm						\n\t"
#endif    
    "mov	%[dataout]	,	r0		\n\t"
#ifdef __AVR_HAVE_RAMPZ__
    "sts	%[rampz]	,	r17		\n\t"
#endif
//     "ldi	r17		,	%[nvmnoop]	\n\t"
//     "sts	%[nvmcmdreg]	,	r17		\n\t"
    : [dataout]		"=r"	(result)
    : [looper]		"z"	((uint16_t)address),
      [anyregister]	"r"	(address>>16),
      [nvmcmdreg]	"i"	(_SFR_MEM_ADDR(__SPM_REG)),
#ifdef __AVR_HAVE_RAMPZ__
      [rampz]		"i"	(_SFR_MEM_ADDR(RAMPZ)),
#endif
      [nvmcmd]		"r"	(nvmCommand),
      [nvmnoop]		"M"	(NVM_CMD_NO_OPERATION_gc)
#ifdef __AVR_HAVE_RAMPZ__
    : "r17", "r0", "memory"
#else
    : "r0", "memory"
#endif
   );
  return result;
}

size_t flash_readpage_Ex(uint8_t out_pageBuffer[SPM_PAGESIZE], const uint_farptr_t in_pageNr, const uint8_t nvmCommand) {
#if (SPM_PAGESIZE < 256)
  uint8_t i;
#else
  uint16_t i;
#endif
  uint_farptr_t	address		= in_pageNr * ((uint_farptr_t)SPM_PAGESIZE);

  for (i=0;i<SPM_PAGESIZE;i++) out_pageBuffer[i]=flash_read_Ex(address+i, nvmCommand);

  return SPM_PAGESIZE;
}

int flash_comparepage_Ex(const uint8_t in_pageBuffer[SPM_PAGESIZE], const uint_farptr_t in_pageNr, const uint8_t nvmCommand) {
#if (SPM_PAGESIZE < 256)
  uint8_t i;
#else
  uint16_t i;
#endif
  uint8_t	data;
  uint_farptr_t	address		= in_pageNr * ((uint_farptr_t)SPM_PAGESIZE);

  for (i=SPM_PAGESIZE;i>0;) {
    i--;
    data=flash_read_Ex(address+i, nvmCommand);
    if (in_pageBuffer[i] != data) {
      if (in_pageBuffer[i] < data)	return -1;
      else				return +1;
    }
  }

  return 0;
}

size_t flash_readpage(uint8_t out_pageBuffer[SPM_PAGESIZE], const uint_farptr_t in_pageNr) {
  return flash_readpage_Ex(out_pageBuffer, in_pageNr, NVM_CMD_NO_OPERATION_gc);
}

int flash_comparepage(const uint8_t in_pageBuffer[SPM_PAGESIZE], const uint_farptr_t in_pageNr) {
  return flash_comparepage_Ex(in_pageBuffer, in_pageNr, NVM_CMD_NO_OPERATION_gc);
}


void __do_spm(const uint_farptr_t flashword_ptr, const uint8_t nvmcommand, const uint16_t dataword) {
  uint8_t sreg_backup = SREG;
  /* since spminterface will create one anyway, we can use it */
  cli();
  do_spm((const uint32_t)flashword_ptr, (const uint8_t) nvmcommand, (const uint16_t) dataword);
  SREG=sreg_backup;
}

uint8_t __checkmagic(void) {
  /*TODO: on spminterface this is hard to determine - say always true for now */
  return 1;
}


/* spminterface based ATmegas don't implement this...
 * 
 * ...but since all ATmegas with spminterface don't do linker tricks, and
 * occupy only the whole BLS - return the should value (BLS size) here.
 */
uint16_t __reportBLSpagesize(void) {
  return ((BOOT_SECTION_SIZE) / (BOOT_SECTION_PAGE_SIZE));
}

/* spminterface based ATmegas don't implement this */
uint8_t __ctrlfunc(uint8_t callcode) {
  return 0xff;
}

size_t flash_updatepage_Ex(void *not_implemented_in_public__use_NULL, const uint8_t in_pageBuffer[SPM_PAGESIZE], const uint_farptr_t in_pageNr, const uint8_t nvmReadCommand, const uint8_t nvmWriteCommand) {
  if (flash_comparepage_Ex(in_pageBuffer, in_pageNr, nvmReadCommand) != 0) {
    return flash_writepage_Ex(not_implemented_in_public__use_NULL, in_pageBuffer, in_pageNr, nvmWriteCommand);
  }
  return SPM_PAGESIZE;
}

size_t flash_updatepage(const uint8_t in_pageBuffer[SPM_PAGESIZE], const uint_farptr_t in_pageNr) {
  if (flash_comparepage(in_pageBuffer, in_pageNr) != 0) {
    return flash_writepage(in_pageBuffer, in_pageNr);
  }
  return SPM_PAGESIZE;
}

size_t flash_writepage(const uint8_t in_pageBuffer[SPM_PAGESIZE], const uint_farptr_t in_pageNr) {
  return flash_writepage_Ex(NULL, in_pageBuffer, in_pageNr, apipage_pagewritecode);
}


/* taken from the USBaspLoader updater code */
size_t flash_writepage_Ex(void *not_implemented_in_public__use_NULL, const uint8_t in_pageBuffer[SPM_PAGESIZE], const uint_farptr_t in_pageNr, const uint8_t nvmCommand) {
  if (nvmCommand == apipage_pagewritecode) {
    size_t		pagesize	= ((size_t)SPM_PAGESIZE) >> 1;
    uint16_t		*pagedata	= (void*)in_pageBuffer;
    uint_farptr_t	pageaddr_bakup	= in_pageNr * ((uint_farptr_t)SPM_PAGESIZE);
    uint_farptr_t	pageaddr	= pageaddr_bakup;

    size_t	i;
      
    //do a page-erase, ATTANTION: flash only can be erased a limited number of times !
    __do_spm(pageaddr_bakup, apipage_pageerasecode, 0);

    // from now on, the page is assumed empty !! (hopefully our code is located somewhere else!)
    // now, fill the tempoary buffer
    // ATTANTION: see comment on "do_spm" !
    pageaddr	= pageaddr_bakup;
    for (i=0;i<pagesize;i+=1) {
      __do_spm(pageaddr, apipage_pagefillcode, pagedata[i]);
      pageaddr+=2;
    }
      
    // so, now finally write the page to the FLASH
    __do_spm(pageaddr_bakup, apipage_pagewritecode, 0);
    
    return SPM_PAGESIZE;
  }
  return 0;
}


#define __BOOTLOADERENTRY_FROMSOFTWARE__EXPECTEDADDRESS ((BOOT_SECTION_START) >> 1)
int8_t bootloader_startup(void) {
  wdt_enable(WDTO_15MS);
  wdt_reset();
  asm volatile (
    "cli					\n\t"
    "ldi	r29 ,		%[ramendhi]	\n\t"
    "ldi	r28 ,		%[ramendlo]	\n\t"
#if (FLASHEND>131071)
    "ldi	r18 ,		%[bootaddrhi]	\n\t"
    "st		Y+, 		r18		\n\t"
#endif
    "ldi	r18 ,		%[bootaddrme]	\n\t"
    "st		Y+,		r18		\n\t"
    "ldi	r18 ,		%[bootaddrlo]	\n\t"
    "st		Y+,		r18		\n\t"
    "out	%[mcucsrio],	__zero_reg__	\n\t"
    "bootloader_startup_loop%=:			\n\t"
    "rjmp bootloader_startup_loop%=		\n\t"
    : 
    : [mcucsrio]	"I"	(_SFR_IO_ADDR(MCUCSR)),
#if (FLASHEND>131071)
      [ramendhi]	"M"	(((RAMEND - 2) >> 8) & 0xff),
      [ramendlo]	"M"	(((RAMEND - 2) >> 0) & 0xff),
      [bootaddrhi]	"M"	(((__BOOTLOADERENTRY_FROMSOFTWARE__EXPECTEDADDRESS) >>16) & 0xff),
#else
      [ramendhi]	"M"	(((RAMEND - 1) >> 8) & 0xff),
      [ramendlo]	"M"	(((RAMEND - 1) >> 0) & 0xff),
#endif
      [bootaddrme]	"M"	(((__BOOTLOADERENTRY_FROMSOFTWARE__EXPECTEDADDRESS) >> 8) & 0xff),
      [bootaddrlo]	"M"	(((__BOOTLOADERENTRY_FROMSOFTWARE__EXPECTEDADDRESS) >> 0) & 0xff)
  );

  return -1;
};
