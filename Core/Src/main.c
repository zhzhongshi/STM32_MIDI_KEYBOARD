/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usb_device.h"
#include "usbd_midi_if.h"
#include <string.h>
#include <stdint.h>
#include "midiMessage.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
PCD_HandleTypeDef hpcd_USB_FS;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static int firstNote = 53;

static uint64_t keypadState = 0;
static uint64_t previousKeypadState = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config();

uint64_t GetKeypadState();
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t GetRowState(GPIO_TypeDef *gpio, uint16_t pin)
{
	HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_SET);

	uint32_t oldResult = 0;
	uint32_t result = 0;
	uint8_t sameResultCount = 5;
	uint8_t allResultCount = 10;
	while (sameResultCount > 0 && allResultCount > 0)
	{
		result = GPIOB->IDR & (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8); // B4,B5,B6,B7,B8
		if (oldResult == result)
		{
			sameResultCount--;
		}
		else
		{
			sameResultCount = 5;
			oldResult = result;
		}

		allResultCount--;
	}

	HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_RESET);

	return (uint8_t)(result >> 4);
}

uint64_t GetColumnState(GPIO_TypeDef *gpio, uint16_t pin)
{
	HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_SET);

	uint64_t oldResult = 0;
	uint64_t result = 0;
	uint8_t sameResultCount = 5;
	uint8_t allResultCount = 10;
	uint32_t resultA;
	uint32_t resultB;
	while (sameResultCount > 0 && allResultCount > 0)
	{
		resultA = GPIOA->IDR & (GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);								 // A8,A9,A10
		resultB = GPIOB->IDR & (GPIO_PIN_9 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_14); // B9,B12,B13,B14,B15
		result = 0;
		if (resultB & GPIO_PIN_9)
		{
			result |= 1;
		}
		if (resultA & GPIO_PIN_10)
		{
			result |= 1 << 5;
		}
		if (resultA & GPIO_PIN_9)
		{
			result |= 1 << 10;
		}
		if (resultA & GPIO_PIN_8)
		{
			result |= 1 << 15;
		}
		if (resultB & GPIO_PIN_15)
		{
			result |= 1 << 20;
		}
		if (resultB & GPIO_PIN_14)
		{
			result |= 1 << 25;
		}
		if (resultB & GPIO_PIN_13)
		{
			result |= 1 << 30;
		}
		if (resultB & GPIO_PIN_12)
		{
			result |= (uint64_t)1 << 35;
		}

		if (oldResult == result)
		{
			sameResultCount--;
		}
		else
		{
			sameResultCount = 5;
			oldResult = result;
		}

		allResultCount--;
	}

	HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_RESET);

	return result;
}

uint64_t GetKeypadStateByRow()
{
	uint64_t result = 0;
	uint8_t rowState;

	rowState = GetRowState(GPIOB, GPIO_PIN_12);
	result |= rowState;
	result <<= 5;

	rowState = GetRowState(GPIOB, GPIO_PIN_13);
	result |= rowState;
	result <<= 5;

	rowState = GetRowState(GPIOB, GPIO_PIN_14);
	result |= rowState;
	result <<= 5;

	rowState = GetRowState(GPIOB, GPIO_PIN_15);
	result |= rowState;
	result <<= 5;

	rowState = GetRowState(GPIOA, GPIO_PIN_8);
	result |= rowState;
	result <<= 5;

	rowState = GetRowState(GPIOA, GPIO_PIN_9);
	result |= rowState;
	result <<= 5;

	rowState = GetRowState(GPIOA, GPIO_PIN_10);
	result |= rowState;
	result <<= 5;

	rowState = GetRowState(GPIOB, GPIO_PIN_9);
	result |= rowState;

	return result;
}

uint64_t GetKeypadStateByColumn()
{

	uint64_t result = 0;
	uint64_t columnState;

	columnState = GetColumnState(GPIOB, GPIO_PIN_4);
	result |= columnState;

	columnState = GetColumnState(GPIOB, GPIO_PIN_5);
	result |= columnState << 1;

	columnState = GetColumnState(GPIOB, GPIO_PIN_6);
	result |= columnState << 2;

	columnState = GetColumnState(GPIOB, GPIO_PIN_7);
	result |= columnState << 3;

	columnState = GetColumnState(GPIOB, GPIO_PIN_8);
	result |= columnState << 4;

	return result;
}

uint64_t GetKeypadState()
{
	uint64_t resultByRow = GetKeypadStateByRow();
	uint64_t resultByColumn = GetKeypadStateByColumn();

	return resultByRow | resultByColumn;
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */
	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USB_PCD_Init();
	/* USER CODE BEGIN 2 */

	HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	MX_USB_DEVICE_Init();

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		keypadState = GetKeypadState();
		if (keypadState != previousKeypadState)
		{
			for (int note = 0; note < 37; note++)
			{
				uint64_t noteBit = (uint64_t)1 << note;
				if ((keypadState & noteBit) != (previousKeypadState & noteBit))
				{
					if ((keypadState & noteBit) == 0)
					{
						// note released
						midiMessage(MIDI_NOTE_OFF, 0, firstNote + note, 0);
					}
					else
					{
						// note pressed
						midiMessage(MIDI_NOTE_ON, 0, firstNote + note, 100);
					}
				}
			}

			previousKeypadState = keypadState;
		}

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
