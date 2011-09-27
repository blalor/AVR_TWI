extern "C" {
    #include <avr/io.h>
    #include "twi.h"
    #include "8bit_binary.h"
    
    void ISR_TWI_vect(void);
}

#include <stdint.h>
#include "CppUTest/TestHarness.h"

#include <math.h>

TEST_GROUP(TWIMasterTests) {
    void setup() {
        PRR = 0xff;
        TWCR = 0;
        
        twi_master_init();
    }
};

TEST(TWIMasterTests, ClockSpeed) {
    CHECK_EQUAL(8000000, F_CPU);
}

TEST(TWIMasterTests, Initialization) {
    BYTES_EQUAL(PRR,  B01111111); // PRTWI must be cleared
    BYTES_EQUAL(TWCR, B10000101); // interrupt, TWI enabled; int flag cleared
    
    // prescaler values (TWPS1, TWPS0)
    //     00 == 1
    //     01 == 4
    //     10 == 16
    //     11 == 64
    // so 11 is 3, 2**(3 * 2) is 64.  holds for 0, 1, 2 and 3.
    unsigned int prescaler = pow(2, ((0x03 & TWSR) * 2));
    
    // assert 100kHz i2c rate
    LONGS_EQUAL(100000, F_CPU / (16 + (2 * TWBR * prescaler)));
    
    // !! INCOMPLETE !!
    // look at status flags?
}

/*
 * Tests transmission of a single byte.
 */
TEST(TWIMasterTests, TransmitByte) {
    uint8_t dest_addr = 0x2B;
    uint8_t data[] = {42};
    uint8_t data_len = 1;
    
    TWCR = 0;
    
    twi_master_transmit(dest_addr, data, data_len);

    CHECK_TRUE(twi_master_busy()); // still busy
    BYTES_EQUAL(B10100101, TWCR);  // TWI initialized, send start
    
    // START was sent, now trigger interrupt
    TWDR = 0;
    TWCR = 0;
    TWSR = 0x08; // START sent
    
    ISR_TWI_vect();
    
    CHECK_TRUE(twi_master_busy()); // still busy
    BYTES_EQUAL(B10000101, TWCR);  // TWINT, TWEN, TWIE
    BYTES_EQUAL(0x56,      TWDR);  // addr + W
    
    // SLA+W was sent successfully; trigger interrupt
    TWDR = 0;
    TWCR = 0;
    TWSR = 0x18; // SLA+W sent, ACK received
    
    ISR_TWI_vect();
    
    CHECK_TRUE(twi_master_busy()); // still busy
    BYTES_EQUAL(B10000101, TWCR);  // TWINT, TWEN, TWIE
    BYTES_EQUAL(42, TWDR);
    
    // data sent successfully; trigger interrupt
    TWDR = 0;
    TWCR = 0;
    TWSR = 0x28; // DATA sent, ACK received
    
    ISR_TWI_vect();
    
    CHECK_FALSE(twi_master_busy()); // operation complete
    BYTES_EQUAL(B10010101, TWCR);   // STOP condition
}

/*
 * Tests transmission of two bytes.
 */
TEST(TWIMasterTests, Transmit2Bytes) {
    uint8_t dest_addr = 0x2A;
    uint8_t data[] = {24, 42};
    uint8_t data_len = sizeof(data) / sizeof(data[0]);
    
    TWCR = 0;
    
    twi_master_transmit(dest_addr, data, data_len);
    
    CHECK_TRUE(twi_master_busy()); // still busy
    BYTES_EQUAL(B10100101, TWCR); // TWI initialized, send start
    
    // START was sent, now trigger interrupt
    TWDR = 0;
    TWCR = 0;
    TWSR = 0x08; // START sent
    
    ISR_TWI_vect();
    
    CHECK_TRUE(twi_master_busy()); // still busy
    BYTES_EQUAL(B10000101, TWCR);  // TWINT, TWEN, TWIE
    BYTES_EQUAL(0x54,      TWDR);  // addr + W
    
    // SLA+W was sent successfully; trigger interrupt
    TWDR = 0;
    TWCR = 0;
    TWSR = 0x18; // SLA+W sent, ACK received
    
    ISR_TWI_vect();
    
    CHECK_TRUE(twi_master_busy()); // still busy
    BYTES_EQUAL(B10000101, TWCR);  // TWINT, TWEN, TWIE
    BYTES_EQUAL(24, TWDR);
    
    // DATA was sent successfully; trigger interrupt
    TWDR = 0;
    TWCR = 0;
    TWSR = 0x28; // DATA sent, ACK received
    
    ISR_TWI_vect();
    
    CHECK_TRUE(twi_master_busy()); // still busy
    BYTES_EQUAL(B10000101, TWCR);  // TWINT, TWEN, TWIE
    BYTES_EQUAL(42, TWDR);
    
    // data sent successfully; trigger interrupt
    TWDR = 0;
    TWCR = 0;
    TWSR = 0x28; // DATA sent, ACK received
    
    ISR_TWI_vect();
    
    CHECK_FALSE(twi_master_busy()); // operation complete
    BYTES_EQUAL(B10010101, TWCR);   // STOP condition
}
