extern "C" {
    #include <avr/io.h>
    
    #include "8bit_binary.h"
}

#include <stdint.h>
#include "CppUTest/TestHarness.h"

TEST_GROUP(TWITests) {
    void setup() {
        PRR = 0xff;
        
    }
}

TEST(TWITests, Initialization) {
    // PRTWI must be cleared
    BYTES_EQUAL(PRR, B01111111);
}