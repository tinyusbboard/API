
#ifndef __IOCOMFORT_H_2cb70d020b7f4d83b020d6fd1be7cc27
#define __IOCOMFORT_H_2cb70d020b7f4d83b020d6fd1be7cc27 1

#define		_internal_LETTER_OF(letter, bit)	(letter)
#define		LETTER_OF(arg)				_internal_LETTER_OF(arg)

#define		_internal_BIT_OF(letter, bit)		(bit)
#define		BIT_OF(arg)				_internal_BIT_OF(arg)


#define		DEFCONCAT(a,b)				a ## b
#define		DEFCONCAT3(a,b,c)			a ## b ## c

#define		_internal_PORT_OF(letter, bit)		DEFCONCAT(PORT, letter)
#define		_internal_DDR_OF(letter, bit)		DEFCONCAT(DDR, letter)
#define		_internal_PIN_OF(letter, bit)		DEFCONCAT(PIN, letter)

#define		PORT_OF(arg)				_internal_PORT_OF(arg)
#define		DDR_OF(arg)				_internal_DDR_OF(arg)
#define		PIN_OF(arg)				_internal_PIN_OF(arg)


#ifndef _BV
#	define _BV(x)	(1<<(x))
#endif

#define		CFG_INPUT(arg)				_internal_DDR_OF(arg)&=~(_BV(_internal_BIT_OF(arg)))
#define		CFG_OUTPUT(arg)				_internal_DDR_OF(arg)|= (_BV(_internal_BIT_OF(arg)))

#define		SET_LOW(arg)				_internal_PORT_OF(arg)&=~(_BV(_internal_BIT_OF(arg)))
#define		SET_HIGH(arg)				_internal_PORT_OF(arg)|= (_BV(_internal_BIT_OF(arg)))
#define		TOGGLE(arg)				_internal_PORT_OF(arg)^= (_BV(_internal_BIT_OF(arg)))

#define		CFG_PULLUP(arg)				{_internal_DDR_OF(arg)&=~(_BV(_internal_BIT_OF(arg))); _internal_PORT_OF(arg)|= (_BV(_internal_BIT_OF(arg)));}
#define		IS_PRESSED(arg)				(!(_internal_PIN_OF(arg) & (_BV(_internal_BIT_OF(arg)))))

#endif