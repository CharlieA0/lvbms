#ifndef LVBMS_CHANNELS_H
#define LVBMS_CHANNELS_H

#include "stm32f4xx_hal.h"

#include <stdbool.h>
#include <stdint.h>

// Constants

#define DEFAULT_VMAX 42   // Units: mV
#define DEFAULT_VMIN 42   //        mV
#define DEFAULT_CMIN 42   //        mA
#define DEFAULT_CMAX 42   //        mA

#define PWM_ON  0xFFFFFFFFU
#define PWM_OFF 0x00000000U

// Type definitions

// Be sure to update these with their I2C Addresses -preserving the matching order- in channels.c as well
typedef enum {
  VCU_CHAN,
  SHUTDOWN_CHAN,
  PUMPS_CHAN,
  FANS_CHAN,
  AERO_CHAN,
  REGEN_CHAN,
  NUM_CHANNELS 
} Channel_Name;

typedef enum {
  IGNORE,
  WARN,
  SHUTOFF,
  NUM_ERROR_RESPONSES
} Error_Response;

typedef enum {
  OVER_VOLTAGE_ERROR,
  UNDER_VOLTAGE_ERROR,
  OVER_CURRENT_ERROR,
  UNDER_CURRENT_ERROR,
  NO_ERROR,
  NUM_ERRORS
} Error_Type;


typedef struct {
  bool      has_timeout;
  uint32_t  timeout_period;
} Channel_Error;

typedef enum {
  CHANNEL_ON,
  CHANNEL_OFF,
  PWM_VALUE,
  NO_COMMAND,
  NUM_CMD_TYPES
} Cmd_Type;


typedef struct {
  Cmd_Type type;
  uint32_t pwm_val;
} Channel_Cmd;


typedef struct Channel {
  Channel_Name name;

  Error_Type   err;
  uint32_t     err_timestamp;

  Channel_Cmd cmd;

  uint16_t    addr;
  
  TIM_HandleTypeDef   *htim;
  uint32_t            tim_channel;

  uint16_t    volt_min;
  uint16_t    volt_max;
  uint16_t    curr_min;
  uint16_t    curr_max;

} Channel;

extern Channel channels[NUM_CHANNELS];
extern uint16_t CHANNEL_ADDR[NUM_CHANNELS];

// Public Interface

void init_channel(Channel * const channel, Channel_Name name, uint16_t addr, TIM_HandleTypeDef *htim, uint32_t tim_channel, uint16_t vmax, uint16_t vmin, uint16_t curr_max, uint16_t curr_min);

bool update_channel(Channel * const channel_name);
void write_channel(Channel const * const channel_name);

#endif
