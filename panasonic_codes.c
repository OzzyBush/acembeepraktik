#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// IR payload structure
typedef struct {
    uint64_t code1;
    uint64_t code2;
    uint64_t code3;
    uint64_t code4;
    uint8_t len;
} ir_payload;

bool one_flag = true;

// Temperature-to-byte mapping
const static uint8_t temperature_table[] = {
    0b00000100,   // 16 C
    0b01000100,   // 17 C
    0b00100100,   // 18 C
    0b01100100,   // 19 C
    0b00010100,   // 20 C
    0b01010100,   // 21 C
    0b00110100,   // 22 C
    0b01110100,   // 23 C
    0b00001100,   // 24 C
    0b01001100,   // 25 C
    0b00101100,   // 26 C
    0b01101100,   // 27 C
    0b00011100,   // 28 C
    0b01011100,   // 29 C
    0b00111100    // 30 C
};

static uint8_t map_temp(uint8_t temp) {
    if (temp < 16) temp = 16;
    else if (temp > 30) temp = 30;
    return temperature_table[temp - 16];
}

const static uint8_t checksum_table[] = {
    0x49,  // 16 C
    0x29,  // 17 C
    0x69,  // 18 C
    0x19,  // 19 C
    0x59,  // 20 C
    0x39,  // 21 C
    0x79,  // 22 C
    0x05,  // 23 C
    0x45,  // 24 C
    0x25,  // 25 C
    0x65,  // 26 C
    0x15,  // 27 C
    0x55,  // 28 C
    0x35,  // 29 C
    0x75   // 30 C
};

static uint8_t map_checksum(uint8_t sum) {
    if (sum < 16) sum = 16;
    else if (sum > 30) sum = 30;
    return checksum_table[sum - 16];
}

static inline uint64_t gen_code1() { return 0x4004072000000060; }
static inline uint64_t gen_code2() {
    if(one_flag == true) {
        return 0x4004072000920001 | (uint64_t)map_temp(29) << 8;
    }
    return 0x4004072000820001 | (uint64_t)map_temp(29) << 8; 
}

static inline uint64_t gen_code3() { return 0xF5B0007007000080; }
static inline uint64_t gen_code4() { return 0x0000000000000000 | (uint64_t)map_checksum(29) << 40;}

ir_payload panasonic_off_payload() {
    return (ir_payload){
        .code1 = 0x4004072000000060,
        .code2 = 0x4004072000023C01,
        .code3 = 0xF5B0007007000080,
        .code4 = 0x0000B50000000000,
        .len = 150
    };
}

ir_payload panasonic_payload(uint8_t temp) {
    if (temp == 0) {
        return panasonic_off_payload();}
    else {
        return (ir_payload){
        .code1 = gen_code1(),
        .code2 = gen_code2(),
        .code3 = gen_code3(),
        .code4 = gen_code4(),
        .len = 150
    };}
}

int main() {
    ir_payload test = panasonic_payload(29);
    printf("%llx %llx %llx %llx\n", test.code1, test.code2, test.code3, test.code4);

    ir_payload off_test = panasonic_off_payload();
    printf("Off Payload: %llx %llx %llx %11x\n", off_test.code1, off_test.code2, off_test.code3, off_test.code4);

    return 0;
}