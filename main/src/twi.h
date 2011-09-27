#ifndef TWI_H
#define TWI_H

#include <stdint.h>
#include <stdbool.h>

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
 */
uint8_t twi_get_last_status(void);
     
/*
 * Transmits data to a slave.
 */
void twi_master_transmit(const uint8_t dest_addr, const uint8_t *data, const uint8_t data_len);

// /*
//  * Receives data from a slave.
//  */
// void twi_master_receive(const uint8_t src_addr, const uint8_t *data, const uint8_t data_len);

#endif
