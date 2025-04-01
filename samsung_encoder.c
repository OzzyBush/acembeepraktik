#include "esp_check.h"
#include "ir_encoder.h"


static const char *TAG = "samsung_encoder";

static rmt_encoder_handle_t samsung_encoder = NULL;
#define IR_RESOLUTION_HZ 1000000 // 1MHz resolution, 1 tick = 1us


typedef struct {
    rmt_encoder_t base;           // the base "class", declares the standard encoder interface
    rmt_encoder_t *copy_encoder;  // use the copy_encoder to encode the leading and ending pulse
    rmt_encoder_t *bytes_encoder; // use the bytes_encoder to encode the address and command data
    rmt_symbol_word_t nec_leading_symbol; // NEC leading code with RMT representation
    rmt_symbol_word_t nec_ending_symbol;  // NEC ending code with RMT representation
    rmt_symbol_word_t nec_mid_symbol;     // NEC mid symbol with RMT representation
    int state;
} rmt_ir_samsung_encoder_t;



static size_t rmt_encode_ir_nec(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
    rmt_ir_samsung_encoder_t *samsung_encoder = __containerof(encoder, rmt_ir_samsung_encoder_t, base);
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;
    const ir_scan_code_t *scan_code = (const ir_scan_code_t *)primary_data;
    rmt_encoder_handle_t copy_encoder = samsung_encoder->copy_encoder;
    rmt_encoder_handle_t bytes_encoder = samsung_encoder->bytes_encoder;

    switch (samsung_encoder->state) {
    case 0: // Send leading code
        encoded_symbols += copy_encoder->encode(copy_encoder, channel, &samsung_encoder->nec_leading_symbol,
                                                sizeof(rmt_symbol_word_t), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            samsung_encoder->state = 1; // Proceed to the next state
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
    // fall-through
    case 1: // Send address
        encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, &scan_code->code1, sizeof(scan_code->code1), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            samsung_encoder->state = 2;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
    // fall-through
    case 2: // Send command
                encoded_symbols += copy_encoder->encode(copy_encoder, channel, &samsung_encoder->nec_mid_symbol,
                                                sizeof(rmt_symbol_word_t), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            samsung_encoder->state = 3; // Reset for the next session
            state |= RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
    // fall-through
    case 3: // Send ending code
       encoded_symbols += copy_encoder->encode(copy_encoder, channel, &samsung_encoder->nec_leading_symbol,
                                                sizeof(rmt_symbol_word_t), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            samsung_encoder->state = 4; // Proceed to the next state
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
    // fall-through
    case 4:
               encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, &scan_code->code1, sizeof(scan_code->code1), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            samsung_encoder->state = 5;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
    // fall-through
        case 5: // Send command
                encoded_symbols += copy_encoder->encode(copy_encoder, channel, &samsung_encoder->nec_mid_symbol,
                                                sizeof(rmt_symbol_word_t), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            samsung_encoder->state = 6; // Reset for the next session
            state |= RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
    // fall-through
    case 6: // Send ending code
       encoded_symbols += copy_encoder->encode(copy_encoder, channel, &samsung_encoder->nec_leading_symbol,
                                                sizeof(rmt_symbol_word_t), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            samsung_encoder->state = 7; // Proceed to the next state
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
    // fall-through
        case 7:
               encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, &scan_code->code2, sizeof(scan_code->code2), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            samsung_encoder->state = 8;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
    // fall-through
    case 8: encoded_symbols += copy_encoder->encode(copy_encoder, channel, &samsung_encoder->nec_ending_symbol,
                                                sizeof(rmt_symbol_word_t), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            samsung_encoder->state = RMT_ENCODING_RESET; // Proceed to the next state
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
    }

    
out:
    *ret_state = state;
    return encoded_symbols;
}


esp_err_t rmt_del_ir_samsung_encoder(rmt_encoder_t *encoder)
{
    rmt_ir_samsung_encoder_t *samsung_encoder = __containerof(encoder, rmt_ir_samsung_encoder_t, base);
    rmt_del_encoder(samsung_encoder->copy_encoder);
    rmt_del_encoder(samsung_encoder->bytes_encoder);
    free(samsung_encoder);
    return ESP_OK;
}

static esp_err_t rmt_ir_samsung_encoder_reset(rmt_encoder_t *encoder)
{
    rmt_ir_samsung_encoder_t *samsung_encoder = __containerof(encoder, rmt_ir_samsung_encoder_t, base);
    rmt_encoder_reset(samsung_encoder->copy_encoder);
    rmt_encoder_reset(samsung_encoder->bytes_encoder);
    samsung_encoder->state = RMT_ENCODING_RESET;
    return ESP_OK;
}

esp_err_t rmt_new_ir_samsung_encoder(const ir_samsung_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder)
{
    esp_err_t ret = ESP_OK;
    rmt_ir_samsung_encoder_t *samsung_encoder = NULL;
    ESP_GOTO_ON_FALSE(config && ret_encoder, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    samsung_encoder = (rmt_ir_samsung_encoder_t *)malloc(sizeof(rmt_ir_samsung_encoder_t));
    ESP_GOTO_ON_FALSE(samsung_encoder, ESP_ERR_NO_MEM, err, TAG, "no mem for ir nec encoder");
    samsung_encoder->base.encode = rmt_encode_ir_nec;
    samsung_encoder->base.del = rmt_del_ir_samsung_encoder;
    samsung_encoder->base.reset = rmt_ir_samsung_encoder_reset;

    rmt_copy_encoder_config_t copy_encoder_config = {};
    ESP_GOTO_ON_ERROR(rmt_new_copy_encoder(&copy_encoder_config, &samsung_encoder->copy_encoder), err, TAG, "create copy encoder failed");

    // construct the leading code and ending code with RMT symbol format
    samsung_encoder->nec_leading_symbol = (rmt_symbol_word_t) {
        .level0 = 1,
        .duration0 = 3100ULL * config->resolution / 1000000,
        .level1 = 0,
        .duration1 = 8800ULL * config->resolution / 1000000,
    };

    samsung_encoder->nec_mid_symbol = (rmt_symbol_word_t) {
        .level0 = 1,
        .duration0 = 650 * config->resolution / 1000000,
        .level1 = 0,
        .duration1 = 2000ULL * config->resolution / 1000000,
    };

    samsung_encoder->nec_ending_symbol = (rmt_symbol_word_t) {
        .level0 = 1,
        .duration0 = 500 * config->resolution / 1000000,
        .level1 = 0,
        .duration1 = 0x7FFF,
    };



    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = 500 * config->resolution / 1000000, // T0H=560us
            .level1 = 0,
            .duration1 = 500 * config->resolution / 1000000, // T0L=560us
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = 500 * config->resolution / 1000000,  // T1H=560us
            .level1 = 0,
            .duration1 = 1500 * config->resolution / 1000000, // T1L=1690us
        },
        .flags = {
            .msb_first = true,
        },

    };
    ESP_GOTO_ON_ERROR(rmt_new_bytes_encoder(&bytes_encoder_config, &samsung_encoder->bytes_encoder), err, TAG, "create bytes encoder failed");

    *ret_encoder = &samsung_encoder->base;
    return ESP_OK;
err:
    if (samsung_encoder) {
        if (samsung_encoder->bytes_encoder) {
            rmt_del_encoder(samsung_encoder->bytes_encoder);
        }
        if (samsung_encoder->copy_encoder) {
            rmt_del_encoder(samsung_encoder->copy_encoder);
        }
        free(samsung_encoder);
    }
    return ret;
}

void samsung_encoder_init()
{

    ESP_LOGD(TAG, "install IR NEC encoder");
    ir_samsung_encoder_config_t samsung_encoder_cfg = {
        .resolution = IR_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_ir_samsung_encoder(&samsung_encoder_cfg, &samsung_encoder));

    rmt_encoder_reset(samsung_encoder);
}

// void sendo_encoder_reset()
// {
//     ESP_LOGI(TAG, "Resetting RMT channel");
//     if (samsung_encoder!=NULL){
//     ESP_ERROR_CHECK(rmt_encoder_reset(samsung_encoder));
//     }
// }

void samsung_encoder_delete()
{
    ESP_LOGD(TAG, "Deleting RMT channel");
    if (samsung_encoder!=NULL){
    ESP_ERROR_CHECK(rmt_del_ir_samsung_encoder(samsung_encoder));
    }
}

rmt_encoder_handle_t get_samsung_encoder()
{
    return samsung_encoder;
}