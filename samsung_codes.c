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
const static uint64_t temperature_table[] = {
    0b10000000010010011111000000000000000010000000011100000000, // 16 C
    0b10000000010011011111000000000000000010000000000010000000, // 17 C
    0b10000000010011011111000000000000000010000000000010000000, // 18 C
    0b10000000010000011111000000000000000010000000111001000000, // 19 C
    0b10000000010000011111000000000000000010000000111001000000, // 20 C
    0b10000000010011101111000000000000000010000000111101000000, // 21 C
    0b10000000010001011111000000000000000010000000000011000000, // 22 C
    0b10000000010010011111000000000000000010000000100011000000, // 23 C
    0b10000000010000011111000000000000000010000000110011000000, // 24 C
    0b10000000010011101111000000000000000010000000111011000000, // 25 C
    0b10000000010000111111000000000000000001000000000000000000, // 26 C
    0b10000000010011011111000000000000000001000000100000000000, // 27 C
    0b10000000010011011111000000000000000001000000010000000000, // 28 C
    0b10000000010001011111000000000000000001000000110000000000, // 29 C
    0b10000000010011011111000000000000000001000000001000000000  // 30 C
};

static uint64_t map_temp(uint64_t temp) {
    if (temp < 16) temp = 16;
    else if (temp > 30) temp = 30;
    return temperature_table[temp - 16];
}

const static uint64_t checksum_table[] = {
    0x48F58E00,  // 16 C
    0x40F58E08,  // 17 C
    0x40F58E04,  // 18 C
    0x4F758E0C,  // 19 C
    0x40F58E02,  // 20 C
    0x4F758E0A,  // 21 C
    0x4F758E06,  // 22 C
    0x47758E0E,  // 23 C
    0x40F58E01,  // 24 C
    0x4F758E09,  // 25 C
    0x4F758E05,  // 26 C
    0x47758E0D,  // 27 C
    0x4F758E03,  // 28 C
    0x47758E0B,  // 29 C
    0x47758E07   // 30 C
};

static uint64_t map_checksum(uint64_t sum) {
    if (sum < 16) sum = 16;
    else if (sum > 30) sum = 30;
    return checksum_table[sum - 16];
}

static inline uint64_t gen_code1() { return 0x4041F80000000F; }
static inline uint64_t gen_code2() {
        return 0x00000000000000 | (uint64_t)map_temp(30) << 0;
}

static inline uint64_t gen_code3() { return 0x8000000000820F | (uint64_t)map_checksum(30) << 16;}

ir_payload samsung_off_payload() {
    return (ir_payload){
        .code1 = 0x4041F80000000F,
        .code2 = 0x8041F0000282C0,
        .code3 = 0x8047758E07820F,
        .len = 150
    };
}

ir_payload samsung_payload(uint8_t temp) {
    if (temp == 0) {
        return samsung_off_payload();}
    else {
        return (ir_payload){
        .code1 = gen_code1(),
        .code2 = gen_code2(),
        .code3 = gen_code3(),
        .len = 150
    };}
}

int main() {
    ir_payload test = samsung_payload(30);
    printf("%llx %llx %llx\n", test.code1, test.code2, test.code3);

    ir_payload off_test = samsung_off_payload();
    printf("Off Payload: %llx %llx %llx\n", off_test.code1, off_test.code2, off_test.code3);

    return 0;
}