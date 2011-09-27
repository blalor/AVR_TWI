#ifndef TWI_H
#define TWI_H

#include <stdint.h>

/*
 * Initialize TWI as a master.
 */
void twi_master_init(void);

/*
 * Disables the TWI interface.
 */
void twi_disable(void);
     
/*
 * Transmits data to a slave.
 */
void twi_master_transmit(const uint8_t dest_addr, const uint8_t *data, const uint8_t data_len);

/*
 * Receives data from a slave.
 */
void twi_master_receive(const uint8_t src_addr, const uint8_t *data, const uint8_t data_len);

/*
 * Retrieves the status of the last operation.
 */
uint8_t twi_get_status(void);
     
#endif
