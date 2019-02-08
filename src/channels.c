
#include "channels.h"
#include "tim.h"
#include "i2c.h"

// Static definitions

Channel channels[NUM_CHANNELS]; 

// These are the I2C Addresses of each channel
uint16_t CHANNEL_ADDR[NUM_CHANNELS] = 
{
  0,  // VCU_CHAN,
  0,  // SHUTDOWN_CHAN,
  0,  // PUMPS_CHAN,
  0,  // FANS_CHAN,
  0,  // AERO_CHAN,
  0,  // REGEN_CHAN,
};

// Error definitions and responses

#ifdef TEST
Channel_Error const error_definitions[NUM_ERRORS] =
{
  {.has_timeout=true, .timeout_period=13},  // OVER_VOLTAGE_ERROR
  {.has_timeout=true, .timeout_period=32},  // UNDER_VOLTAGE_ERROR
  {.has_timeout=true, .timeout_period=0},   // OVER_CURRENT_ERROR
  {.has_timeout=false, .timeout_period=0},  // UNDER_CURRENT_ERROR
  {.has_timeout=false, .timeout_period=32}, // NO_ERROR
};

Error_Response const response_matrix[NUM_CHANNELS][NUM_ERRORS] = 
{ // VCU_CHAN SHUTDOWN_CHAN PUMPS_CHAN FANS_CHAN  AERO_CHAN REGEN_CHAN
      WARN,     IGNORE,      SHUTOFF,   IGNORE,    WARN,     SHUTOFF,   // OVER_VOLTAGE_ERROR
      SHUTOFF,  SHUTOFF,     IGNORE,     WARN,    SHUTOFF,   IGNORE,    // UNDER_VOLTAGE_ERROR
      IGNORE,    WARN,        WARN,     SHUTOFF,  IGNORE,     WARN,     // OVER_CURRENT_ERROR
      SHUTOFF,   WARN,       IGNORE,     WARN,    SHUTOFF,   IGNORE,    // UNDER_CURRENT_ERROR
      IGNORE,   SHUTOFF,      WARN,     IGNORE,   SHUTOFF,    WARN,     // NO_ERROR
};

#elif DEVELOPMENT_ERRORS
Channel_Error const error_definitions[NUM_ERRORS] =
{
  {.has_timeout=false, .timeout_period=0},  // OVER_VOLTAGE_ERROR
  {.has_timeout=false, .timeout_period=0},  // UNDER_VOLTAGE_ERROR
  {.has_timeout=false, .timeout_period=0},  // OVER_CURRENT_ERROR
  {.has_timeout=false, .timeout_period=0},  // UNDER_CURRENT_ERROR
  {.has_timeout=false, .timeout_period=0},  // NO_ERROR
};

Error_Response const response_matrix[NUM_CHANNELS][NUM_ERRORS] = 
{ // VCU_CHAN SHUTDOWN_CHAN PUMPS_CHAN FANS_CHAN  AERO_CHAN REGEN_CHAN
      WARN,     WARN,         WARN,       WARN,     WARN,     WARN,   // OVER_VOLTAGE_ERROR
      WARN,     WARN,         WARN,       WARN,     WARN,     WARN,   // UNDER_VOLTAGE_ERROR
      WARN,     WARN,         WARN,       WARN,     WARN,     WARN,   // OVER_CURRENT_ERROR
      WARN,     WARN,         WARN,       WARN,     WARN,     WARN,   // UNDER_CURRENT_ERROR
     IGNORE,   IGNORE,       IGNORE,     IGNORE,   IGNORE,   IGNORE,  // NO_ERROR
};


#else // COMPETITION ERRORS
Channel_Error const error_definitions[NUM_ERRORS] =
{
  {.has_timeout=false, .timeout_period=0},  // OVER_VOLTAGE_ERROR
  {.has_timeout=false, .timeout_period=0},  // UNDER_VOLTAGE_ERROR
  {.has_timeout=false, .timeout_period=0},  // OVER_CURRENT_ERROR
  {.has_timeout=false, .timeout_period=0},  // UNDER_CURRENT_ERROR
  {.has_timeout=false, .timeout_period=0},  // NO_ERROR
};

Error_Response const response_matrix[NUM_CHANNELS][NUM_ERRORS] = 
{ // VCU_CHAN SHUTDOWN_CHAN PUMPS_CHAN FANS_CHAN  AERO_CHAN REGEN_CHAN
      WARN,     WARN,         WARN,       WARN,     WARN,     WARN,   // OVER_VOLTAGE_ERROR
      WARN,     WARN,         WARN,       WARN,     WARN,     WARN,   // UNDER_VOLTAGE_ERROR
      WARN,     WARN,         WARN,       WARN,     WARN,     WARN,   // OVER_CURRENT_ERROR
      WARN,     WARN,         WARN,       WARN,     WARN,     WARN,   // UNDER_CURRENT_ERROR
     IGNORE,   IGNORE,       IGNORE,     IGNORE,   IGNORE,   IGNORE,  // NO_ERROR
};


#endif

Error_Type get_error(Channel const * const channel);
bool check_timeout_channel(Channel const * const channel_name);

bool update_cmd(Channel * const channel);
bool update_error(Channel * const channel);

void write_cmd(Channel const * const channel, Channel_Cmd const * const);

void log_error(Channel const * const channel);


// Fills channel with initial values
void init_channel(Channel * const channel, Channel_Name name, uint16_t addr, TIM_HandleTypeDef *htim, uint32_t tim_channel, uint16_t vmax, uint16_t vmin, uint16_t curr_max, uint16_t curr_min) {

  channel->name           = name;

  channel->err            = NO_ERROR;
  channel->err_timestamp  = HAL_GetTick();

  channel->cmd  = (Channel_Cmd) {.type=CHANNEL_ON, .pwm_val=0};

  channel->addr         = addr;
  channel->htim         = htim;
  channel->tim_channel  = tim_channel;

  channel->volt_max     = vmax;
  channel->volt_min     = vmin;
  channel->curr_max     = curr_max;
  channel->curr_min     = curr_min;
}


// Tries to update channel with new command from can
// Returns true if there are updates, else false
bool update_cmd(Channel * const channel) {

  // TODO: Implement this
  
  _Error_Handler(__FILE__, __LINE__);

  return NO_COMMAND;

}

// Logs channel's error to CAN
void log_error(Channel const * const channel) {

  // TODO: Implement this
  
  _Error_Handler(__FILE__, __LINE__);

}

// Checks for and returns error on passed channel
Error_Type get_error(Channel const * const channel) {

  // Check for voltage errors
  uint16_t voltage;
  read_voltage(channel->addr, &voltage);
  
  if (voltage < channel->volt_min) {
    return UNDER_VOLTAGE_ERROR;
  }
  if (voltage > channel->volt_max) {
    return OVER_VOLTAGE_ERROR;
  }

  // Check for current errors
  uint16_t current;
  read_current(channel->addr, &current);

  if (current < channel->curr_min) {
    return UNDER_CURRENT_ERROR;
  }
  if (current > channel->curr_max) {
    return OVER_CURRENT_ERROR;  
  }

  // No more errors
  return NO_ERROR;

}

// Tries to update errors on this channel, 
// Returns true if there are updates, else false
bool update_error(Channel * const channel) {

  // Check for errors on channel
  Error_Type current_error = get_error(channel);

  // If there are errors, update channel
  if (current_error != NO_ERROR) {

    channel->err = current_error;
    channel->err_timestamp = HAL_GetTick();

    return true;

  }  

  // If there aren't current errors, try timing out old ones
  if (check_timeout_channel(channel)) {

    channel->err = NO_ERROR;
    channel->err_timestamp = HAL_GetTick();

    return true;

  }

  // There are no error updates
  return false;

}

// Returns true if a channel's error has timed out, else false
bool check_timeout_channel(Channel const * const channel) {

  // Get error's definition
  const Channel_Error error_def = error_definitions[channel->err];

  // Check if it is timed out
  if (error_def.has_timeout && HAL_GetTick() > channel->err_timestamp + error_def.timeout_period) {
      return true;
  }

  // Then the error isn't timed out
  else {
    return false;
  }
}

// Updates channel
// Returns true if there are updates, else false
bool update_channel(Channel * const channel) {
  
  // Get errors if present
  if (update_error(channel)) {

    // Log errors that aren't ignored
    if (response_matrix[channel->name][channel->err] > IGNORE) {
      log_error(channel);
    }
    
    return true;
  }

  if (update_cmd(channel)) {
    return true;
  } 

  return false;

}

// Write the passed command to passed channel
void write_cmd(Channel const * const channel, Channel_Cmd const * const cmd) {
  
  if (cmd->type == CHANNEL_ON) {
    pwm_write(channel->htim, channel->tim_channel, UINT32_MAX);  
  }
  
  else if (cmd->type == CHANNEL_OFF) {
    pwm_write(channel->htim, channel->tim_channel, 0U);
  }
  
  else if (cmd->type == PWM_VALUE) {
    pwm_write(channel->htim, channel->tim_channel, cmd->pwm_val);
  }
  
  else if (cmd->type == NO_COMMAND) {
    // Do nothing
  }
  
  else { // Default - We shouldn't fall through to this
    
    // Throw error if testing    
    #ifdef TEST
    _Error_Handler(__FILE__, __LINE__);
    #endif

    // Otherwise, do nothing

  }


}

// Write command and error responses described in passed channel
void write_channel(Channel const * const channel) {
  
  // If there is an error, respond to it
  if (channel->err != NO_ERROR) {
    
    switch(response_matrix[channel->name][channel->err]) {
      
      case SHUTOFF : 
      {            

        // Turn off the channel
        Channel_Cmd cmd = {.type=CHANNEL_OFF, .pwm_val=0}; // Note pwm_val is irrelevant for CHANNEL_OFF
        write_cmd(channel, &cmd);
        
        // Now return - we won't write the channel's command
        return;
      }
      break;

      case WARN :   // Error was logged already
      break;        // Do nothing here

      case IGNORE : // Do nothing
      break; 

      default:      // We shouldn't fall through
        
        // Throw error if testing    
        #ifdef TEST
        _Error_Handler(__FILE__, __LINE__);
        #endif

        // Otherwise, do nothing

      break;
    }

    // For errors that don't return, 
    // will now write the channel's command

  }

  // Write the channel's command
  write_cmd(channel, &channel->cmd);
}
