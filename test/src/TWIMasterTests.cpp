extern "C" {
    #include <avr/io.h>
    #include "twi.h"
    #include "8bit_binary.h"
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
    BYTES_EQUAL(TWCR, B10000000); // TWINT must be set
    
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
    
    // check for TWI interrupt enabled
    // look at status flags?
}
