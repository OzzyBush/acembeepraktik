#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// IR payload structure
typedef struct {
    uint64_t code1;
    uint64_t code2;
    uint64_t code3;
    uint8_t len;
} ir_payload;

// Temperature-to-byte mapping
const static uint8_t temperature_table[] = {
    0b00000000,   // 16 C
    0b10000000,   // 17 C
    0b01000000,   // 18 C
    0b11000000,   // 19 C
    0b00100000,   // 20 C
    0b10100000,   // 21 C
    0b01100000,   // 22 C
    0b11100000,   // 23 C
    0b00010000,   // 24 C
    0b10010000,   // 25 C
    0b01010000,   // 26 C
    0b11010000,   // 27 C
    0b00110000,   // 28 C
    0b10110000,   // 29 C
    0b01110000    // 30 C
};

static uint8_t map_temp(uint8_t temp) {
    if (temp < 16) temp = 16;
    else if (temp > 30) temp = 30;
    return temperature_table[temp - 16];
}

const static uint8_t checksum_table[] = {
    0xF6,  // 16 C
    0x0E,  // 17 C
    0x8E,  // 18 C
    0x4E,  // 19 C
    0xCE,  // 20 C
    0x2E,  // 21 C
    0xAE,  // 22 C
    0x6E,  // 23 C
    0xEE,  // 24 C
    0xE1,  // 25 C
    0x9E,  // 26 C
    0x5E,  // 27 C
    0xDE,  // 28 C
    0x3E,  // 29 C
    0xBE   // 30 C
};

static uint8_t map_checksum(uint8_t sum) {
    if (sum < 16) sum = 16;
    else if (sum > 30) sum = 30;
    return checksum_table[sum - 16];
}

static inline uint64_t gen_code1() { return 0xC4D364800004; }
static inline uint64_t gen_code2() {return 0x10000C1E5100 | (uint64_t)map_temp(18) << 32; }
static inline uint64_t gen_code3() { return 0x000000000000 | (uint64_t)map_checksum(18) << 0; }

ir_payload mitsubishi_off_payload() {
    return (ir_payload){
        .code1 = 0x800220800000,
        .code2 = 0x00000016B600,
        .code3 = 0x0000000000B2,
        .len = 150
    };
}

ir_payload mitsubishi_payload(uint8_t temp) {
    if (temp == 0) {
        return mitsubishi_off_payload();}
    else {
        return (ir_payload){
        .code1 = gen_code1(),
        .code2 = gen_code2(),
        .code3 = gen_code3(),
        .len = 150
    };}
}

int main() {
    ir_payload test = mitsubishi_payload(18);
    printf("%llx %llx %llx \n", test.code1, test.code2, test.code3);

    ir_payload off_test = mitsubishi_off_payload();
    printf("Off Payload: %llx %llx %llx\n", off_test.code1, off_test.code2, off_test.code3);
    
    return 0;
}