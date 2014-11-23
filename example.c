/*
 * TINYUSBBOARD APIPAGE DEMO IMPLEMENTATION
 * 
 * This firmware demonstrates some functions of the
 * tinyUSBboard bootloader API, which is
 * exported by and accessable via APIPAGE.H
 * 
 * The demonstration does the following:
 * 
 * (1)	Calculate the page before the laste page
 * 	of user firmware
 * (2)	Read "BOOT_SECTION_PAGE_SIZE" bytes from the
 * 	EEPROM into SRAM
 * (3)	Check, if the page from (1) contains the same
 *	data, if so: light up LED_LEFT. 
 * (4)	Generate some informtions and write them to
 * 	the flash (page from (1))
 * (5)	Wait 3sek and then go back to the bootloader
 * 
 * 
 * USBaspLoader also supports starting the firmware
 * from softwarecommand (instead pressing PROG again).
 * Simply send the ISP raw-command "0xff 0 0 0".
 * For example type "send 0xff 0 0 0" in AVRDUDE's
 * console via call "avrdude -c usbasp -p atmega8 -t"
 * After quitting the console, the firmware is started.
 * (When PROG is still/already release.)
 * 
 * Stephan Baerwolf (matrixstorm@gmx.de), Ilmenau 2014
 */

#include "tinyusbboard.h"
#include "apipage.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#ifndef PROGMEM_PAGECOUNT
#	define PROGMEM_PAGECOUNT	(PROGMEM_SIZE/BOOT_SECTION_PAGE_SIZE)
#endif

static uint8_t message[BOOT_SECTION_PAGE_SIZE];

int main(void) {
  uint_farptr_t page_in_question;
  uint16_t bls_pagecount;

  CFG_OUTPUT(LED_LEFT);
  CFG_OUTPUT(LED_RIGHT);

  SET_HIGH(LED_LEFT);
  SET_HIGH(LED_RIGHT);

  /* (0) - check, if running on the correct hardware */
  if (__checkmagic()==0) {
    /* remain in error */
    while (1) {
      TOGGLE(LED_LEFT);
      _delay_ms(100);
    }
  }



  /* (1) - get the size of the current installed bootloader */
  bls_pagecount = __reportBLSpagesize();

  /* (1) - calculate the pagenumber, remember: always do boundary checking! */
  if (bls_pagecount < (PROGMEM_PAGECOUNT-1)) page_in_question=(uint_farptr_t)PROGMEM_PAGECOUNT-(uint_farptr_t)bls_pagecount;
  else {
    /* remain in error */
    while (1) {
      TOGGLE(LED_LEFT);
      _delay_ms(200);
    }
  }

  /* (1) - decrease "page_in_question" by two pages to point to the page before the last page in user firmware
   * 	--> at least two pages are not used by the bootloader (bls_pagecount < (PROGMEM_PAGECOUNT-1))
   * 	--> currently page_in_question points to the first page, the bootloader is on
   */
  page_in_question-=2;


  /* (2) - read the eeprom into "message" */
  eeprom_busy_wait();
  eeprom_read_block(message, 0x0000, sizeof(message));

  /* (3) */
  if (flash_comparepage(message, page_in_question)==0) {
      SET_LOW(LED_LEFT);
  }


  /* (4) - generate some infos */
  memset(message, 0, sizeof(message));
  snprintf((void*)message, sizeof(message)-1,	"BLSpgsize = %i, "
						"PAGECOUNT = %i, "
						"blsize: %i, "
						"herepage: %i" , 
						(int)BOOT_SECTION_PAGE_SIZE, (int)PROGMEM_PAGECOUNT, (int)__reportBLSpagesize(), (int)page_in_question);

  /* (4) - write them to the page */
  flash_updatepage(message, page_in_question);

  
  /* (5) */
  _delay_ms(3000);
  bootloader_startup();
  
  /* the installed bootloader might not implement the startup syscall */
  return 0;
}
