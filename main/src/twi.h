#ifndef TWI_H
#define TWI_H

#include <stdint.h>
#include <stdbool.h>

typedef enum __twi_statii {
    // default state
    TWI_STATUS_IDLE,
    
    // transmit in progress
    TWI_STATUS_TX_PENDING,
    
    // transient; status reset to TWI_STATUS_IDLE after read
    TWI_STATUS_TX_SLA_NACK, // NACK received sending slave address
    TWI_STATUS_TX_COMPLETE, // transmit completed successfully
} TWIStatus;

/*
 * Initialize TWI as a master.
 */
void twi_master_init(void);

// /*
//  * Disables the TWI interface.
//  */
// void twi_disable(void);

/*
 * Tests for a pending transmit or receive operation.
 */
bool twi_master_busy(void);

/*
 * Retrieves the status of the last operation.
 *
 * If a final (completed) state value is returned (such as
 * TWI_STATUS_TX_COMPLETE), subsequent queries will return TWI_STATUS_IDLE.
 */
TWIStatus twi_get_status(void);
     
/*
 * Transmits data to a slave.
 */
void twi_master_transmit(const uint8_t dest_addr, const uint8_t *data, const uint8_t data_len);

// /*
//  * Receives data from a slave.
//  */
// void twi_master_receive(const uint8_t src_addr, const uint8_t *data, const uint8_t data_len);

#endif
