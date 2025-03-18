/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "rtc.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include "bmp280.h"
#include "ssd1306_oled.h"
#include "gfx_oled.h"

#include "sun.h"
#include "fonts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C_ADDR 0x76
#define OLED_TESTING 1

#define SCAN_I2C_DEVICES
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

#ifndef __UUID_H
#define __UUID_H
//#define STM32_UUID ((uint32_t *)0x1FF0F420)
#define STM32_UUID ((uint32_t *)UID_BASE)
#endif //__UUID_H

#define MAX_UART_LEN    128

// USART FRAME PARSER
#define FRAME_NONE     0
#define FRAME_START    1
#define FRAME_TIME      2
#define FRAME_DATE      3
#define FRAME_END      4

#define SET_TIME_CMD "SET_TIME"
#define SET_DATE_CMD "SET_DATE"

#define TEST_OLED
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//CLOCK
RTC_TimeTypeDef rtcTime;
RTC_DateTypeDef rtcDate;
uint8_t compareSecods;
char timeInfoMess[128];
uint8_t timeSetMess[128];
uint8_t timeInfoLen;

// BMP Data
BMP280_t bmp280;
volatile float temperature;
volatile float pressure;

// OLED Data
char message[128];

uint32_t softTimer;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
uint8_t main_TimeChange();
uint8_t main_ScanI2CDevs();
uint8_t main_TestOled();
uint8_t main_SendInstructions();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_LPUART1_UART_Init();
  MX_I2C1_Init();
  MX_RTC_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
#ifdef SCAN_I2C_DEVICES
  main_ScanI2CDevs();
#endif
  // Temp and pressure sensor init:
  BMP280_Init(&bmp280, &hi2c1, I2C_ADDR);

  // Oled display init:
  SSD1306_Init(&hi2c1);
  SSD1306_Clear(BLACK);
  SSD1306_Display();
  
  // set font of the chars fisplay
  GFX_SetFont(font_8x5);
  GFX_SetFontSize(1);
  // Display each figure at the beginning
#ifdef TEST_OLED
  main_TestOled();
#endif

  // enable command receiving
  HAL_UARTEx_ReceiveToIdle_DMA(&hlpuart1, timeSetMess, MAX_UART_LEN);

  // Display command instructions on uart:
  main_SendInstructions();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    HAL_RTC_GetTime(&hrtc, &rtcTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &rtcDate, RTC_FORMAT_BIN);
    if(rtcTime.Seconds != compareSecods) {
      // if second has changed - called every second
      SSD1306_Clear(BLACK);
      timeInfoLen = sprintf(timeInfoMess, "Actual Date and Time: %02d-%02d-20%02d %02d:%02d:%02d\r\n", 
                                                                    rtcDate.Date,
                                                                    rtcDate.Month,
                                                                    rtcDate.Year,
                                                                    rtcTime.Hours,
                                                                    rtcTime.Minutes,
                                                                    rtcTime.Seconds
                                                                    );
      HAL_UART_Transmit_DMA(&hlpuart1,timeInfoMess ,timeInfoLen);
      if (bmp280.bmpI2c->State == HAL_I2C_STATE_READY)
      temperature = BMP28_ReadTemperature(&bmp280);
      pressure = BMP28_ReadPressure(&bmp280);
      sprintf(message, "Press: %.2f hpa", pressure);
      GFX_DrawString(0, 0, message, WHITE, BLACK);
      sprintf(message, "Temp: %.2f C", temperature);
      GFX_DrawString(0, 10, message, WHITE, BLACK);
      sprintf(message, "Date: %02d/%02d/20%02d", rtcDate.Date, rtcDate.Month, rtcDate.Year);
      GFX_DrawString(0, 20,  message, WHITE, BLACK);
      sprintf(message, "Time: %02d:%02d:%02d", rtcTime.Hours, rtcTime.Minutes, rtcTime.Seconds);
      GFX_DrawString(0, 30,  message, WHITE, BLACK);
      SSD1306_Display();
      compareSecods = rtcTime.Seconds;
    }

    // Check if Uart message has been received
    if (rcvMesFlag) {
      main_TimeChange();
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* I2C1_EV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* LPUART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(LPUART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(LPUART1_IRQn);
}

/* USER CODE BEGIN 4 */
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
 PUTCHAR_PROTOTYPE
 {
   /* Place your implementation of fputc here */
   /* e.g. write a character to the USART2 and Loop until the end of transmission */
   HAL_UART_Transmit(&hlpuart1, (uint8_t *)&ch, 1, 0xFFFF);
 
   return ch;
 }
 int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&hlpuart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

uint8_t main_TestOled() {

  for (uint8_t i = 0; i < 9; i++) {
    switch(i){
      case 1:
      // ROUND RECTANGLE
        GFX_DrawRoundRect(50, 10, 40, 40, 5, WHITE);
        break;
      case 2:
        GFX_DrawRect(20, 10, 50, 20, WHITE);
        break;
      case 3:
        GFX_DrawCircle(60, 30, 15, WHITE);
        break;
      case 4:
        GFX_FillCircle(60, 30, 15, WHITE);
        break;
      case 5:
        GFX_WriteFillRect(20, 10, 50, 20, WHITE);
        break;
      case 6:
        GFX_FillRoundRect(50, 10, 40, 40, 5, WHITE);
        break;
      case 7:
        GFX_DrawTriangle(5, 5, 60, 5, 30, 30, WHITE);
        break;  
      case 8:
        GFX_FillTriangle(5, 5, 60, 5, 30, 30, WHITE);
        break; 
    }
    SSD1306_Display();
    HAL_Delay(1000);
    SSD1306_Clear(BLACK);
  }
    
  return 0;
}

uint8_t main_SendInstructions() {
  printf(" * to set time send: $SET_TIMEHH/MM/SS#\r\n");
  printf(" * to set date send: $SET_DATEDD/MM/YYYY#\r\n");
  return 0;
}

uint8_t main_ScanI2CDevs() {
  char lpuart1Data[24] = "Connected to LPUART One\r\n";
  HAL_UART_Transmit(&hlpuart1, (uint8_t *)&lpuart1Data,sizeof(lpuart1Data), 0xFFFF);

 	printf("\r\n");

 	printf("Scanning I2C bus:\r\n");
	HAL_StatusTypeDef result;
 	uint8_t i, devCnt = 0;
 	for (i=1; i<128; i++)
 	{
 	  /*
 	   * the HAL wants a left aligned i2c address
 	   * &hi2c1 is the handle
 	   * (uint16_t)(i<<1) is the i2c address left aligned
 	   * retries 2
 	   * timeout 2
 	   */
 	  result = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i<<1), 2, 2);
 	  if (result != HAL_OK) // HAL_ERROR or HAL_BUSY or HAL_TIMEOUT
 	  {
 		  printf("."); // No ACK received at that address
 	  }
 	  if (result == HAL_OK)
 	  {
 		  printf("0x%X", i); // Received an ACK at that address
       devCnt++;
 	  }
 	}
 	printf("\r\n");
  printf("Found available devices: %d\r\n", devCnt);
  return (devCnt > 0) ? 0 : 1;
}

uint8_t main_TimeChange() {
  /* Frame example
  * <Start><Command><Data><Data Size><END>
  * Start     - '$' = 0x24
  * Command   - available: 'SET_TIME' 'SET_DATE'
  * Data      - TIME: 'XX/XX:/XX', DATE: 'DD/MM/YYYY'
  * END       - '#'= 0x23
  * 
  * */

  RTC_TimeTypeDef Time;
  RTC_DateTypeDef Date;

  uint8_t command[24];
  uint8_t frameSeq = FRAME_NONE;
  uint8_t j = 0, cnt = 0;;
  uint8_t str[4] = {0, 0, 0, 0};
  
  for (uint8_t i = 0; i < MAX_UART_LEN; i++) {
    switch (frameSeq) {
      case FRAME_NONE:
        if (timeSetMess[i] == '$') {
          frameSeq = FRAME_START;
        } 
        break;

      case FRAME_START:
        command[j] = timeSetMess[i];
        j++;
        if (j == 8 && !strncmp(command, SET_DATE_CMD, 8)) {
          frameSeq = FRAME_DATE;
          j = 0;
        } 
        else if (j == 8 && !strncmp(command, SET_TIME_CMD, 8)) {
          frameSeq = FRAME_TIME;
          j = 0;
        }
        else if (j > 8){
          // cmd not recognized!
          j = 0;
          return - 1;
        }
      break;

      case FRAME_DATE:
        if (timeSetMess[i] >= '0' && timeSetMess[i] <= '9') {
          if (j > 4) { return -1; }

          str[j] = timeSetMess[i];
          j++;
        } 
        else if (timeSetMess[i] == '/' || timeSetMess[i] == '#') {
          switch (cnt) {
            case 0:
              Date.Date = atoi(str);
              memset(str, 0, sizeof(str));
              cnt++;
              j = 0;
              break;
            case 1:
              Date.Month = atoi(str);
              memset(str, 0, sizeof(str));
              cnt++;
              j = 0;
              break;
            case 2:
              Date.Year = atoi(str + 2);
              memset(str, 0, sizeof(str));
              cnt++;
              j = 0;
              rtcDate.Date = Date.Date;
              rtcDate.Month = Date.Month;
              rtcDate.Year = Date.Year;
              HAL_RTC_SetDate(&hrtc, &rtcDate, RTC_FORMAT_BIN);
              break;  
          }
        }
        else {
          // invalid value
          return -1;
        }
      break;

      case FRAME_TIME:
        if (timeSetMess[i] >= '0' && timeSetMess[i] <= '9') {
          if (j > 4) { return -1; }

          str[j] = timeSetMess[i];
          j++;
        } 
        else if (timeSetMess[i] == '/' || timeSetMess[i] == '#') {
          switch (cnt) {
            case 0:
              Time.Hours = atoi(str);
              memset(str, 0, sizeof(str));
              cnt++;
              j = 0;
              break;
            case 1:
              Time.Minutes = atoi(str);
              memset(str, 0, sizeof(str));
              cnt++;
              j = 0;
              break;
            case 2:
              Time.Seconds = atoi(str);
              memset(str, 0, sizeof(str));
              cnt++;
              j = 0;
              rtcTime.Hours = Time.Hours;
              rtcTime.Minutes = Time.Minutes;
              rtcTime.Seconds = Time.Seconds;
              HAL_RTC_SetTime(&hrtc, &rtcTime, RTC_FORMAT_BIN);
              break;  
          }
        }
        else {
          // invalid value
          return -1;
        }
      break;
    }
    if (cnt == 3) {
      memset(timeSetMess, 0, 128);
      return 0;
    }
    
  }
  return 0;
}
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

#ifdef  USE_FULL_ASSERT
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
