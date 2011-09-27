#include <avr/io.h>
#include <avr/interrupt.h>

#include "twi.h"

#if !defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#    error Unsupported chip!
#endif

#if F_CPU != 8000000
#    error illegal CPU value
#endif

#define TWDR_ENABLE (_BV(TWIE) | _BV(TWEN) | _BV(TWINT))

typedef struct __tx_state {
    uint8_t dest_addr;
    uint8_t *data;
    uint8_t data_len;
    
    uint8_t data_index;
} TWITxState;

static TWITxState txState;

void twi_master_init() {
    
    // set 100kHz frequency; hard-coded for 8MHz
    TWSR = (TWSR & 0x03) | _BV(TWPS1); // set prescaler to 16
    TWBR  = 2;                         // do the needful to set 100kHz
    
    PRR  &= ~_BV(PRTWI); // disable power reduction for TWI
    
    // enable interrupts, enable TWI operation, clear interrupt flag
    TWCR = TWDR_ENABLE;
}

void twi_master_transmit(const uint8_t dest_addr, const uint8_t *data, const uint8_t data_len) {
    txState.dest_addr = dest_addr;
    txState.data = (uint8_t *)data;
    txState.data_len = data_len;
    
    txState.data_index = 0;
    
    // enable TWI and interrupt, clear INT flag, send START
    TWCR = TWDR_ENABLE | _BV(TWSTA);
}

ISR(TWI_vect) {
    switch (TWSR & 0xF8) { // status register with prescaler bits masked out
        case 0x08: // START received
            // load SLA+W
            TWDR = (txState.dest_addr << 1);
            TWCR = TWDR_ENABLE;
            break;
        
        case 0x18: // SLA+W sent, ACK received
            // load data
            TWDR = txState.data[txState.data_index++];
            TWCR = TWDR_ENABLE;
            break;
        
        case 0x28: // DATA sent, ACK received
            if (txState.data_index < txState.data_len) {
                TWDR = txState.data[txState.data_index++];
                TWCR = TWDR_ENABLE;
            } else {
                // all done!
                TWCR = TWDR_ENABLE | _BV(TWSTO);
            }
            break;
        
        default:
            // @todo
            break;
    }
    
}
