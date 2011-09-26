#include <avr/io.h>
#include <avr/interrupt.h>

#include "twi.h"

#if !defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#    error Unsupported chip!
#endif

#if F_CPU != 8000000
#    error illegal CPU value
#endif

void twi_master_init() {
    
    // set 100kHz frequency; hard-coded for 8MHz
    TWSR = (TWSR & 0x03) | _BV(TWPS1); // set prescaler to 16
    TWBR  = 2;                         // do the needful to set 100kHz
    
    PRR  &= ~_BV(PRTWI); // disable power reduction for TWI
    
    // enable interrupts, enable TWI operation, clear interrupt flag
    TWCR |= _BV(TWIE) | _BV(TWEN) | _BV(TWINT); 
}

ISR(TWI_vect) {
    // TWCR |= _BV(TWINT); // clear TWI interrupt flag
}
