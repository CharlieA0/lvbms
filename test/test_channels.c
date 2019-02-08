#include "unity.h"
#include "channels.h"
#include "tim.h"
#include "i2c.h"

#include "mocks/mock_i2c.h"

#include <stdio.h>

// Expose some private functions and variables for testing 

Error_Type get_error(Channel const * const channel);
bool check_timeout_channel(Channel const * const channel_name);

bool update_cmd(Channel * const channel);
bool update_error(Channel * const channel);

void write_cmd(Channel const * const channel, Channel_Cmd const * const);

void log_error(Channel const * const channel);

// Private variables (made extern for testing purposes)
extern Channel_Error const error_definitions[NUM_ERRORS];
extern Error_Response const response_matrix[NUM_CHANNELS][NUM_ERRORS];


void test_should_always_pass(void) {}

void test_channels_linked(void) {
  uint16_t first_channel = CHANNEL_ADDR[0];
}

void test_single_channel_initialization(void) {
  
  // Create testing environment
  TIM_HandleTypeDef phony_timer;

  SYSTEM_TICKS = 1001; // Set system clock

  Channel channel;

  init_channel(&channel, SHUTDOWN_CHAN, 7889, &phony_timer, 34, 113, 33, 344, 44);
  
  
  TEST_ASSERT_EQUAL_INT_MESSAGE(NO_ERROR, channel.err, "Initialized with error");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(1001, channel.err_timestamp, "Initialzed with wrong timestamp");
  TEST_ASSERT_EQUAL_INT_MESSAGE(CHANNEL_ON, channel.cmd.type, "Initialized without CHANNEL_ON commmand");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(0, channel.cmd.pwm_val, "Initialized with nonzero pwm value");
  TEST_ASSERT_EQUAL_INT16_MESSAGE(channel.addr, 7889, "Initialized with wrong I2C address");
  TEST_ASSERT_EQUAL_PTR_MESSAGE(&phony_timer, channel.htim, "Initialized with wrong timer pointer");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(34, channel.tim_channel, "Initialized with wrong timer channel");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(113, channel.volt_max, "Initialized with wrong voltage max");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(33, channel.volt_min, "Initialized with wrong voltage min");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(344, channel.curr_max, "Initialized with wrong current max");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(44, channel.curr_min, "Initialized with wrong current min");
}

void test_channel_initialization_like_irl(void) {
  
  // Create testing environment
  SYSTEM_TICKS = 890122033;

  Channel phony_channels[NUM_CHANNELS];
  
  for (int i = 0; i < NUM_CHANNELS; i++) {
    init_channel(&phony_channels[i], VCU_CHAN, CHANNEL_ADDR[i], &htim4, i+1, DEFAULT_VMAX, DEFAULT_VMIN, DEFAULT_CMAX, DEFAULT_CMIN);
    SYSTEM_TICKS++;
  }

  for (int i = 0; i < NUM_CHANNELS; i++) {
    Channel channel = phony_channels[i];
    TEST_ASSERT_EQUAL_INT(NO_ERROR, channel.err);
    TEST_ASSERT_EQUAL_INT32(SYSTEM_TICKS - NUM_CHANNELS + i, channel.err_timestamp);
    TEST_ASSERT_EQUAL_INT(CHANNEL_ON, channel.cmd.type);
    TEST_ASSERT_EQUAL_INT32(0, channel.cmd.pwm_val);
    TEST_ASSERT_EQUAL_INT16(CHANNEL_ADDR[i], channel.addr);
    TEST_ASSERT_EQUAL_PTR(&htim4, channel.htim);
    TEST_ASSERT_EQUAL_INT32(DEFAULT_VMAX, channel.volt_max);
    TEST_ASSERT_EQUAL_INT32(DEFAULT_VMIN, channel.volt_min);
    TEST_ASSERT_EQUAL_INT32(DEFAULT_CMAX, channel.curr_min);
    TEST_ASSERT_EQUAL_INT32(DEFAULT_CMIN, channel.curr_min);
  }
}

void test_errors(void) {

  Channel c;
  TIM_HandleTypeDef phony_timer;
  
  SYSTEM_TICKS = 2390;

  uint8_t tx_buf[2];
  tx_queue_start = &tx_buf[0];
  tx_queue_end = &tx_buf[2];

  uint8_t rx_buf[4] = { 0x00, 0x96, 0x0f, 0x11 };
  rx_queue_start = &rx_buf[0];
  rx_queue_end = &rx_buf[4];  

  init_channel(&c, REGEN_CHAN, 339, &phony_timer, 133, 0x0095, 0x0000, 0xffff, 0x0000); 
  
  // Test has over voltage

  TEST_ASSERT_TRUE_MESSAGE(get_error(&c) == OVER_VOLTAGE_ERROR, "get_error() didn't return OVER_VOLTAGE_ERROR");
  
  tx_queue_start = &tx_buf[0];
  rx_queue_start = &rx_buf[0];
  SYSTEM_TICKS = 293;

  TEST_ASSERT_TRUE_MESSAGE(update_error(&c), "update_error() didn't return true for OVER_VOLTAGE_ERROR");
  TEST_ASSERT_TRUE_MESSAGE(c.err == OVER_VOLTAGE_ERROR, "channel wasn't set to OVER_VOLTAGE_ERROR");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(SYSTEM_TICKS, c.err_timestamp, "channel was given wrong timestamp");

  // Test has under voltage error

  tx_queue_start = &tx_buf[0];
  rx_queue_start = &rx_buf[0];
  init_channel(&c, VCU_CHAN, 339, &phony_timer, 133, 0x0099, 0x0098, 0x0f13, 0x0003); 

  TEST_ASSERT_TRUE_MESSAGE(get_error(&c) == UNDER_VOLTAGE_ERROR, "get_error() didn't return UNDER_VOLTAGE_ERROR");
   
  tx_queue_start = &tx_buf[0];
  rx_queue_start = &rx_buf[0];
  SYSTEM_TICKS = 290;
  
  TEST_ASSERT_TRUE_MESSAGE(update_error(&c), "update_error() didn't return true for UNDER_VOLTAGE_ERROR");
  TEST_ASSERT_TRUE_MESSAGE(c.err == UNDER_VOLTAGE_ERROR, "channel wasn't set to UNDER_VOLTAGE_ERROR");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(SYSTEM_TICKS, c.err_timestamp, "channel was given wrong timestamp");

  // Test has over current error

  tx_queue_start = &tx_buf[0];
  rx_queue_start = &rx_buf[0];
  init_channel(&c, REGEN_CHAN, 339, &phony_timer, 133, 0x0099, 0x0094, 0x0013, 0x0003); 

  TEST_ASSERT_TRUE_MESSAGE(get_error(&c) == OVER_CURRENT_ERROR, "get_error() didn't return OVER_CURRENT_ERROR");
  
  tx_queue_start = &tx_buf[0];
  rx_queue_start = &rx_buf[0];
  SYSTEM_TICKS = 198;
  
  TEST_ASSERT_TRUE_MESSAGE(update_error(&c), "update_error() didn't return true for OVER_CURRENT_ERROR");
  TEST_ASSERT_TRUE_MESSAGE(c.err == OVER_CURRENT_ERROR, "channel wasn't set to OVER_CURRENT_ERROR");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(SYSTEM_TICKS, c.err_timestamp, "channel was given wrong timestamp");

  // Test has under current error

  tx_queue_start = &tx_buf[0];
  rx_queue_start = &rx_buf[0];
  init_channel(&c, VCU_CHAN, 339, &phony_timer, 133, 0x0099, 0x0094, 0xfff3, 0xff43); 

  TEST_ASSERT_TRUE_MESSAGE(get_error(&c) == UNDER_CURRENT_ERROR, "get_error() didn't return UNDER_CURRENT_ERROR");
  
  tx_queue_start = &tx_buf[0];
  rx_queue_start = &rx_buf[0];
  SYSTEM_TICKS = 119;
  
  TEST_ASSERT_TRUE_MESSAGE(update_error(&c), "update_error() didn't return true for UNDER_CURRENT_ERROR");
  TEST_ASSERT_TRUE_MESSAGE(c.err == UNDER_CURRENT_ERROR, "channel wasn't set to UNDER_CURRENT_ERROR");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(SYSTEM_TICKS, c.err_timestamp, "channel was given wrong timestamp");
 
  // How should we handle multiple errors? 

  // Test doesn't have error

  tx_queue_start = &tx_buf[0];
  rx_queue_start = &rx_buf[0];
  init_channel(&c, REGEN_CHAN, 339, &phony_timer, 133, 0x0097, 0x0003, 0x0f13, 0x0003); 

  TEST_ASSERT_TRUE_MESSAGE(get_error(&c) == NO_ERROR, "get_error() didn't return NO_ERROR");
  
  tx_queue_start = &tx_buf[0];
  rx_queue_start = &rx_buf[0];
  SYSTEM_TICKS = 193;
  c.err_timestamp = 399;

  TEST_ASSERT_FALSE_MESSAGE(update_error(&c), "update_error() didn't return false for NO_ERROR");
  TEST_ASSERT_TRUE_MESSAGE(c.err == NO_ERROR, "channel wasn't set to NO_ERROR");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(399, c.err_timestamp, "channel's timestamp was changed");

}

// Updating and logging really depends on can

void test_timeout_channel(void) {

  Channel c;
  TIM_HandleTypeDef phony_timer;
  init_channel(&c, AERO_CHAN, 78, &phony_timer, 12,38, 32, 88, 9);

  for (int i = 0; i < NUM_ERRORS; i++) {

    c.err = i;
    c.err_timestamp = 14;
    Channel_Error def = error_definitions[i]; 

    if (def.has_timeout) {

      // Try not timing out error
      
      for (int j = 0; j <= def.timeout_period + c.err_timestamp; j++) {
        SYSTEM_TICKS = j;

        TEST_ASSERT_FALSE_MESSAGE(check_timeout_channel(&c), "Error timed-out early");

      }

      for (int j = def.timeout_period + c.err_timestamp + 1; j <= UINT16_MAX; j++) {

        // Reset error
        c.err = i;

        // Try timing out error
        SYSTEM_TICKS = j;
        
        TEST_ASSERT_TRUE_MESSAGE(check_timeout_channel(&c), "Error didn't timeout");
      }
    }

  }

}

void test_update_errors(void) {

  Channel c;
  TIM_HandleTypeDef phony_timer;

  uint8_t rx_buf[4] = {0xbf, 0xff, 0x55, 0x44};
  rx_queue_start = &rx_buf[0];
  rx_queue_end = &rx_buf[4];

  uint8_t tx_buf[2];
  tx_queue_start = &tx_buf[0];
  tx_queue_end = &tx_buf[2];

  SYSTEM_TICKS = 1990;

  init_channel(&c, VCU_CHAN, 3, &phony_timer, 1, 0x00af, 0x0044, 0x0055, 0x0033); 
  SYSTEM_TICKS = 18;

  TEST_ASSERT_TRUE_MESSAGE(update_error(&c), "update_error() didn't return true, no error update was found"); 
  TEST_ASSERT_TRUE_MESSAGE(c.err == OVER_VOLTAGE_ERROR, "update_error() set channel to wrong error");

  TEST_ASSERT_EQUAL_INT32_MESSAGE(SYSTEM_TICKS, c.err_timestamp, "update_error() set channel's err_timestamp incorrectly");


  rx_queue_start = &rx_buf[0];
  tx_queue_start = &tx_buf[0];

  init_channel(&c, VCU_CHAN, 3, &phony_timer, 1, 0xffaf, 0x0044, 0xff66, 0x0033); 
  SYSTEM_TICKS = 1899;

  TEST_ASSERT_FALSE_MESSAGE(update_error(&c), "update_error() returned true, but there was no error update");
  TEST_ASSERT_TRUE_MESSAGE(c.err == NO_ERROR, "update_error() gave channel error when none was present");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(18, c.err_timestamp, "update_error() set channel's err_timestamp, but there was no error");


}

void test_update_errors_timeout(void) {

  Channel c;
  TIM_HandleTypeDef phony_timer;

  uint8_t rx_buf[4] = {0x00, 0x46, 0x00, 0x44};
  rx_queue_start = &rx_buf[0];
  rx_queue_end = &rx_buf[4];

  uint8_t tx_buf[2];
  tx_queue_start = &tx_buf[0];
  tx_queue_end = &tx_buf[2];

  SYSTEM_TICKS = 1990;

  init_channel(&c, REGEN_CHAN, 2, &phony_timer, 1, 0x0123, 0x0023, 0xffff, 0x0000);

  c.err = OVER_VOLTAGE_ERROR;
  c.err_timestamp = 199;
  SYSTEM_TICKS = 199 + error_definitions[c.err].timeout_period + 1;
  
  TEST_ASSERT_TRUE_MESSAGE(update_error(&c), "update_error() didn't return true, no error update was found");
  TEST_ASSERT_TRUE_MESSAGE(c.err == NO_ERROR, "update_error() didn't set channel to NO_ERROR");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(SYSTEM_TICKS, c.err_timestamp, "update_error() didn't set channel's err_timestamp correctly");

  rx_queue_start = &rx_buf[0];
  tx_queue_start = &tx_buf[0];

  TEST_ASSERT_FALSE_MESSAGE(update_error(&c), "update_error() returned true for channel without errors or timeout");
  TEST_ASSERT_TRUE_MESSAGE(c.err == NO_ERROR, "update_error() set error incorrectly");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(SYSTEM_TICKS, c.err_timestamp, "update_error() set channel's err_timestamp incorrectly");

}

// Mock a couple of hal functions so that MX_TIM4_Init() works

HAL_StatusTypeDef HAL_TIM_PWM_Init(TIM_HandleTypeDef * htim) {

  return HAL_OK;

}

HAL_StatusTypeDef HAL_TIMEx_MasterConfigSynchronization(TIM_HandleTypeDef * htim, TIM_MasterConfigTypeDef * sMasterConfig) {

  return HAL_OK;

} 

HAL_StatusTypeDef HAL_TIM_PWM_ConfigChannel(TIM_HandleTypeDef * htim, TIM_OC_InitTypeDef * sConfig, uint32_t channel) {

  return HAL_OK;

}  

HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef * htim, uint32_t channel) {

  return HAL_OK;

}

void test_write_cmd(void) {

  Channel c;

  MX_TIM4_Init();
  init_channel(&c, AERO_CHAN, 2, &htim4, 0, 134, 4, 11, 4);
 
  Channel_Cmd cmd = { CHANNEL_ON, 0};

  for (int i = 0; i < UINT16_MAX; i++) {
    cmd.pwm_val = i;
    write_cmd(&c, &cmd);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UINT32_MAX, htim4.Instance->CCR1, "write_cmd() set the timer's CCR register incorrectly");
  }

  cmd =(Channel_Cmd) { CHANNEL_OFF, 0};

  for (int i = 0; i < UINT16_MAX; i++) {
    cmd.pwm_val = i;
    write_cmd(&c, &cmd);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, htim4.Instance->CCR1, "write_cmd() set the timer's CCR register incorrectly");
  }
 
  cmd = (Channel_Cmd) {PWM_VALUE, 0};

  for (int i = 0; i < UINT16_MAX; i++) {
    cmd.pwm_val = i;
    write_cmd(&c, &cmd);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(i, htim4.Instance->CCR1, "write_cmd() set the timer's CCR register incorrectly");
  }

  cmd = (Channel_Cmd) {NO_COMMAND, 88};
  htim4.Instance->CCR1 = 37494;

  for (int i = 0; i < UINT16_MAX; i++) {
    cmd.pwm_val = i;
    write_cmd(&c, &cmd);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(37494, htim4.Instance->CCR1, "write_cmd() set the timer's CCR register on NO_COMMAND");
  }
   

}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_should_always_pass);
  RUN_TEST(test_channels_linked);
  RUN_TEST(test_single_channel_initialization);
  RUN_TEST(test_channel_initialization_like_irl);
  RUN_TEST(test_errors);
  RUN_TEST(test_timeout_channel);
  RUN_TEST(test_update_errors); 
  RUN_TEST(test_update_errors_timeout);
  RUN_TEST(test_write_cmd);
  return UNITY_END();
}


void _Error_Handler(char *file, int line)
{

 TEST_FAIL_MESSAGE("Error was thrown"); 

}
