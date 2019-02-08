#include "unity.h"
#include "i2c.h"

#include <stdio.h>

#include "mocks/i2c-mock.h"

void test_should_always_pass(void) {}


void test_register_read(void) {
  
  // Setup 
  uint8_t rx_buf[2] = {0xa3, 0xf3}; // Loads value in register
  rx_queue_start = &rx_buf[0];
  rx_queue_end = &rx_buf[2];

  uint8_t tx_buf[2];
  tx_queue_start = &tx_buf[0];
  tx_queue_end = &tx_buf[2];

  uint16_t rsp;
  read_reg(13, 31, &rsp);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(31 << 1, transmit_address, "Wrote wrong i2c address during Read");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(31 << 1, receive_address, "Read from wrong i2c address");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(13, tx_buf[0], "Set register pointer to wrong address");
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0xa3f3, rsp, "Read wrong value from register");
}

void test_register_many_read(void) {
  
  uint8_t rx_buf[18] = {0x13, 0xb3, 0xbe, 0x33, 0x11, 0x33, 0x3b, 0x4b, 0x21, 0xb3, 0x22, 0x33, 0x23, 0x24, 0xff, 0xff, 0x00, 0x00 };
  rx_queue_start = &rx_buf[0];
  rx_queue_end = &rx_buf[18];

  uint8_t tx_buf[2];
  tx_queue_end = &tx_buf[2];

  uint8_t registers[9]   = { 32, 44, 55, 12, 234, 39, 13, 11, 0};
  uint8_t i2c_channel[9] = { 99, 134, 24, 11, 1, 33, 23, 44, 9};

  uint16_t rsp;

  for (int i = 0; i < 9; i++) { 

    // Reset transmit queue
    tx_queue_start = &tx_buf[0];

    read_reg(registers[i], i2c_channel[i], &rsp);

    TEST_ASSERT_EQUAL_UINT16_MESSAGE(i2c_channel[i] << 1, transmit_address, "Wrote wrong i2c address during read");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(i2c_channel[i] << 1, receive_address, "Read from wrong i2c address");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(registers[i], tx_buf[0], "Set register pointer to wrong address");
    TEST_ASSERT_EQUAL_HEX16_MESSAGE((rx_buf[2*i] << 8) | rx_buf[2*i + 1], rsp, "Read wrong value from register");
  
  }
}

void test_read_voltage(void) {

  uint8_t tx_buf[1];
  tx_queue_start = &tx_buf[0];
  tx_queue_end = &tx_buf[1];

  uint8_t rx_buf[2] = {0xae, 0x31};
  rx_queue_start = &rx_buf[0];
  rx_queue_end = &rx_buf[2];

  uint16_t voltage;
  read_voltage(39, &voltage);
  
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(INA226_REG_BUS_VOLT, tx_buf[0], "Didn't set voltage register for read");
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0xae31, voltage, "Read wrong voltage");
}
 
void test_read_current(void) {

  uint8_t tx_buf[1];
  tx_queue_start = &tx_buf[0];
  tx_queue_end = &tx_buf[1];

  uint8_t rx_buf[2] = {0xfd, 0x3d};
  rx_queue_start = &rx_buf[0];
  rx_queue_end = &rx_buf[2];

  uint16_t current;
  read_current(39, &current);
  
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(INA226_REG_CURRENT, tx_buf[0], "Didn't set current register for read");
  TEST_ASSERT_EQUAL_HEX16_MESSAGE(0xfd3d, current, "Read wrong current");
}

void test_register_write(void) {
  uint8_t tx_buf[3];
  tx_queue_start = &tx_buf[0];
  tx_queue_end = &tx_buf[3];

  write_reg(34, 2, 83);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(2 << 1, transmit_address, "Transmitted to wrong address");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(34, tx_buf[0], "First byte transmitted is wrong");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(83, tx_buf[1], "Second byte transmitted is wrong"); 
}

void test_register_many_write(void) {
  uint8_t tx_buf[3];
  
  uint8_t registers[9] = { 32, 44, 23, 92, 43, 43, 99, 90, 34};
  uint8_t i2c_channel[9] = { 34, 45, 24, 31, 44, 24, 33, 211, 234};
  uint16_t value[9]       = { 0, 43, 183, 3539, 39, 363, 239, 22, 31 };

  for (int i = 0; i < 9; i++) {
    tx_queue_start = &tx_buf[0];
    tx_queue_end = &tx_buf[3];
    
    write_reg(registers[i], i2c_channel[i], value[i]);
    
    TEST_ASSERT_EQUAL_HEX16_MESSAGE(i2c_channel[i] << 1, transmit_address, "Transmitted to wrong address");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(registers[i], tx_buf[0], "Wrote to wrong register");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t) value[i], tx_buf[1], "Wrote wrong first byte of value"); 
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(((uint8_t*) &value)[1], tx_buf[2], "Wrote wrong second byte of value");
  }
}  


int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_should_always_pass);
  RUN_TEST(test_register_write);
  RUN_TEST(test_register_many_write);
  RUN_TEST(test_register_read);
  RUN_TEST(test_register_many_read);
  RUN_TEST(test_read_current);
  RUN_TEST(test_read_voltage);
  return UNITY_END();
}


void _Error_Handler(char *file, int line)
{

  char error_message[10000];
  sprintf(&error_message[0], "Error was thrown at line %d in %s", line, file);
  TEST_FAIL_MESSAGE(error_message); 

}
