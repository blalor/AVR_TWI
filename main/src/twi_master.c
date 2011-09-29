#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>

#include <stdbool.h>

#include "twi.h"

#if !(defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega168P__))
#    error Unsupported chip!
#endif

#if F_CPU != 8000000
#    error illegal CPU value
#endif

#define TWCR_ENABLE (_BV(TWINT) | _BV(TWEN) | _BV(TWIE))

typedef struct __tx_state {
    uint8_t dest_addr;
    uint8_t *data;
    uint8_t data_len;
    
    uint8_t data_index;
} TWITxState;

static volatile TWITxState txState;
static volatile TWIStatus  twiStatus;

// state variable used to determine if a send or receive operation is still
// in progress
static volatile bool twiBusy;

void twi_master_init(const TWISpeed busSpeed) {
    if (busSpeed == TWI_SPEED_100MHz) {
        // set 100kHz frequency; hard-coded for 8MHz
        TWSR = _BV(TWPS1); // set prescaler to 16
    } else if (busSpeed == TWI_SPEED_400MHz) {
        // set 400kHz frequency; hard-coded for 8MHz
        TWSR = 0; // set prescaler to 1
    }
    
    TWBR  = 2;         // do the needful to set 100kHz
    
    PRR  &= ~_BV(PRTWI); // disable power reduction for TWI
    
    twiBusy = false;
    twiStatus = TWI_STATUS_IDLE;

    // enable interrupts, enable TWI operation, clear interrupt flag
    TWCR = TWCR_ENABLE | _BV(TWSTO);
}

bool twi_master_busy() {
    return twiBusy;
}

TWIStatus twi_get_status() {
    TWIStatus rc = twiStatus;
    
    switch (rc) {
        // transient; status reset to TWI_STATUS_IDLE after read
        case TWI_STATUS_TX_SLA_NACK:
        case TWI_STATUS_TX_COMPLETE:
            twiStatus = TWI_STATUS_IDLE;
            
            break;
        
        default:
            // no-op
            break;
    }
    
    return rc;
}

void twi_master_transmit(const uint8_t dest_addr, const uint8_t *data, const uint8_t data_len) {
    // wait for pending operations to complete
    while (twiBusy)
        ;
    
    twiBusy = true;
    twiStatus = TWI_STATUS_TX_PENDING;
    
    txState.dest_addr = dest_addr;
    txState.data = (uint8_t *)data;
    txState.data_len = data_len;
    
    txState.data_index = 0;
    
    // enable TWI and interrupt, clear INT flag, send START
    TWCR = TWCR_ENABLE | _BV(TWSTA);
}

ISR(TWI_vect) {
    // default return value
    uint8_t tmpTWCR = TWCR_ENABLE;
    
    switch (TW_STATUS) { // status register with prescaler bits masked out
        case TW_START: // START received
        // case TW_REP_START: // repeated START received
            // load SLA+W
            TWDR = (txState.dest_addr << 1);
            
            break;
        
        case TW_MT_SLA_ACK:  // SLA+W sent, ACK received
        case TW_MT_DATA_ACK: // DATA sent, ACK received
            if (txState.data_index < txState.data_len) {
                // load data byte
                TWDR = txState.data[txState.data_index++];
            } else {
                // all done!
                twiBusy = false;
                twiStatus = TWI_STATUS_TX_COMPLETE;
                
                tmpTWCR |= _BV(TWSTO); // STOP
            }
            
            break;
        
        case TW_MT_SLA_NACK: // SLA+W sent, NACK received
            // maybe the slave addr doesn't exist? just bail
            twiBusy = false;
            twiStatus = TWI_STATUS_TX_SLA_NACK;
            tmpTWCR |= _BV(TWSTO); // STOP
            
            break;
        
        // case TW_MT_DATA_NACK: // DATA sent, NACK received
        // case TW_MT_ARB_LOST: // arbitration lost sending SLA+W or DATA sent
        default:
            // @todo
            
            break;
    }
    
    TWCR = tmpTWCR;
}
