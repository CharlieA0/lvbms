#include "main.h"
#include <stm32f4xx_hal.h>
#include "can.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

#include <string.h>
#include <stdio.h>

#include "uart.h"
#include "channels.h"

// Use testing and development errors and responses
// Comment out to use competition errors and responses
#define DEVELOPMENT_ERRORS 

void SystemClock_Config(void);

int main(void) {
  
  // HAL Initialization and setup

  HAL_Init();
  SystemClock_Config();

  UART_Init();            
  
  MX_GPIO_Init();
  //MX_CAN1_Init();       // Initializing CAN on lonely bus causes exception
  MX_I2C1_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();

  // Initialize channels
  for (int i = 0; i < NUM_CHANNELS; i++) {

    // Note: that the first four channels are on htim4
    if (i < 4) {
      init_channel(&channels[i], i, CHANNEL_ADDR[i], &htim4, i+1, DEFAULT_VMAX, DEFAULT_VMIN, DEFAULT_CMAX, DEFAULT_CMIN);
    }

    // The rest are on htim5
    else {
      init_channel(&channels[i], i, CHANNEL_ADDR[i], &htim5, i-3, DEFAULT_VMAX, DEFAULT_VMIN, DEFAULT_CMAX, DEFAULT_CMIN);
    }
  }

  // Main loop
  
  while (1)
  {

    // for each named channel
    for (int i = 0; i < NUM_CHANNELS; i++) {

      // Check for channel updates
      bool channel_dirty = update_channel(&channels[i]);

      // Write results
      if (channel_dirty) {
        write_channel(&channels[i]);
      }

    }    
  }

}

/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * 
  ** This notice applies to portions of this file after this notice
  *  that are not enclosed by comment pairs reading MIT MOTORSPORTS. 
  *  Other portions of this file remain under the copyright of their
  *  creators. 
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;


  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void _Error_Handler(char *file, int line)
{

  // MIT MOTORSPORTS
  
  print("\n\rA terrible, terrible error occured.\n\r");
  print("It's certainly not my fault so just check: ");
  print(file);
  print(" : ");
  print_int(line, 10);
  print("\n\r and we won't discuss it again.\n\r");

  // MIT MOTORSPORTS

  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line)
{

}
#endif
