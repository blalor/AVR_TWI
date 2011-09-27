#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdbool.h>

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

// state variable used to determine if a send or receive operation is still
// in progress
static volatile bool twiBusy;

void twi_master_init() {
    // set 100kHz frequency; hard-coded for 8MHz
    TWSR = (TWSR & 0x03) | _BV(TWPS1); // set prescaler to 16
    TWBR  = 2;                         // do the needful to set 100kHz
    
    PRR  &= ~_BV(PRTWI); // disable power reduction for TWI
    
    // enable interrupts, enable TWI operation, clear interrupt flag
    TWCR = TWDR_ENABLE;
    
    twiBusy = false;
}

bool twi_master_busy() {
    return twiBusy;
}

void twi_master_transmit(const uint8_t dest_addr, const uint8_t *data, const uint8_t data_len) {
    // wait for pending operations to complete
    while (twiBusy)
        ;
    
    twiBusy = true;
    
    txState.dest_addr = dest_addr;
    txState.data = (uint8_t *)data;
    txState.data_len = data_len;
    
    txState.data_index = 0;
    
    // enable TWI and interrupt, clear INT flag, send START
    TWCR = TWDR_ENABLE | _BV(TWSTA);
}

ISR(TWI_vect) {
    // default return value
    uint8_t tmpTWCR = TWDR_ENABLE;
    
    switch (TWSR & 0xF8) { // status register with prescaler bits masked out
        case 0x08: // START received
        // case 0x10: // repeated START received
            // load SLA+W
            TWDR = (txState.dest_addr << 1);
            
            break;
        
        case 0x18: // SLA+W sent, ACK received
        case 0x28: // DATA sent, ACK received
            if (txState.data_index < txState.data_len) {
                // load data byte
                TWDR = txState.data[txState.data_index++];
            } else {
                // all done!
                twiBusy = false;
                tmpTWCR |= _BV(TWSTO); // STOP
            }
            
            break;
        
        // case 0x20: // SLA+W sent, NACK received
        // case 0x30: // DATA sent, NACK received
        // case 0x38: // arbitration lost sending SLA+W or DATA sent
        default:
            // @todo
            
            break;
    }
    
    TWCR = tmpTWCR;
}
