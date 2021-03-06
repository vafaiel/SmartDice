/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_hal.h"
#include "lis3dh_driver.h"

/* Defines -------------------------------------------------------------------*/
#define DATA GPIO_PIN_4
#define CLOCK GPIO_PIN_5
#define LATCH GPIO_PIN_6

//Macro converting binaries to actual binaries
#define LongToBin(n) \
(\
((n >> 21) & 0x80) | \
((n >> 18) & 0x40) | \
((n >> 15) & 0x20) | \
((n >> 12) & 0x10) | \
((n >>  9) & 0x08) | \
((n >>  6) & 0x04) | \
((n >>  3) & 0x02) | \
((n      ) & 0x01)   \
)

#define Bin(n) LongToBin(0x##n##l)


/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
uint8_t response;
uint8_t buffer[26]; 
AxesRaw_t data;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
void LED_reset(void);
void LED_write(uint8_t serial);

/* Private function prototypes -----------------------------------------------*/
uint16_t Register_out_x= 0x28;

int main(void)
{
  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  
  //Inizialize MEMS Sensor (from LIS3DH driver example)
  //set ODR (turn ON device)
  response = LIS3DH_SetODR(LIS3DH_ODR_100Hz);
  if(response == MEMS_SUCCESS){
        sprintf((char*)buffer,"\n\rSET_ODR_OK    \n\r\0");
  }
  //set PowerMode 
  response = LIS3DH_SetMode(LIS3DH_NORMAL);
  if(response == MEMS_SUCCESS){
        sprintf((char*)buffer,"SET_MODE_OK     \n\r\0");
  }
  //set Fullscale
  response = LIS3DH_SetFullScale(LIS3DH_FULLSCALE_2);
  if(response == MEMS_SUCCESS){
        sprintf((char*)buffer,"SET_FULLSCALE_OK\n\r\0");
  }
  //set axis Enable
  response = LIS3DH_SetAxis(LIS3DH_X_ENABLE | LIS3DH_Y_ENABLE | LIS3DH_Z_ENABLE);
  if(response == MEMS_SUCCESS){
        sprintf((char*)buffer,"SET_AXIS_OK     \n\r\0");
  }
  //Turn on 1 LED
	LED_write(Bin(10010000));

  /* Infinite loop */
  while (1)
  {
    //get Acceleration Raw data  
    response = LIS3DH_GetAccAxesRaw(&data);
		if( data.AXIS_Z>16000)
		{
			LED_write(Bin(10000000));
		}
		if( data.AXIS_Z>13000 && data.AXIS_Z<16000)
		{
			LED_write(Bin(00010000));
		}
		if( data.AXIS_Z<13000)
		{
			LED_write(Bin(00000010));
		}
    if(response == MEMS_SUCCESS){
      //print data values
      sprintf((char*)buffer, "X=%6d Y=%6d Z=%6d \r\n", data.AXIS_X, data.AXIS_Y, data.AXIS_Z);
    }
  }
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

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
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Analogue filter 
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Digital filter 
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Pinout Configuration
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA4 PA5 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void LED_reset(void)
{
	HAL_GPIO_WritePin(GPIOA,DATA,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,LATCH,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,LATCH,GPIO_PIN_RESET);
}

void LED_write(uint8_t serial)
{
	for(int i=0; i<8; i++)
	{
		if ( (serial/128)>=1)
		{
			HAL_GPIO_WritePin(GPIOA,DATA,GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(GPIOA,DATA,GPIO_PIN_RESET);
		}
		HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA,CLOCK,GPIO_PIN_RESET);
		serial=serial<<1;
	}
	HAL_GPIO_WritePin(GPIOA,LATCH,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,LATCH,GPIO_PIN_RESET);

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
