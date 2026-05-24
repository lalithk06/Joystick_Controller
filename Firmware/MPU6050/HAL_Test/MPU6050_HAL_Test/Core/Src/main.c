/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : MPU6050 Test with SWV ITM output
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
#define MPU6050_ADDR   (0x68 << 1)
#define WHO_AM_I_REG   0x75
#define PWR_MGMT_1     0x6B
#define ACCEL_XOUT_H   0x3B
#define GYRO_XOUT_H    0x43
/* USER CODE END PD */

I2C_HandleTypeDef hi2c1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN 0 */

// Redirect printf to SWO ITM
int __io_putchar(int ch) {
    ITM_SendChar(ch);
    return ch;
}

void MPU6050_Init(void) {
    uint8_t check, data;

    // Check WHO_AM_I
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 100);

    if (check == 0x68) {
        printf("MPU6050 Found! WHO_AM_I=0x%02X\r\n", check);

        // Wake up - clear sleep bit
        data = 0x00;
        HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, PWR_MGMT_1, 1, &data, 1, 100);
        printf("MPU6050 Initialized.\r\n\r\n");

        // LED solid ON = detected
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    } else {
        printf("MPU6050 NOT Found! Got=0x%02X\r\n", check);

        // LED fast blink = not found
        for(int i = 0; i < 6; i++) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            HAL_Delay(100);
        }
    }
}

void MPU6050_Read(void) {
    uint8_t buf[14];
    int16_t ax, ay, az, gx, gy, gz, temp_raw;
    float temp;

    // Read 14 bytes from ACCEL_XOUT_H
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        &hi2c1, MPU6050_ADDR, ACCEL_XOUT_H, 1, buf, 14, 100);

    if (status != HAL_OK) {
        printf("I2C Read Error!\r\n");
        return;
    }

    // Combine high and low bytes
    ax       = (int16_t)(buf[0]  << 8 | buf[1]);
    ay       = (int16_t)(buf[2]  << 8 | buf[3]);
    az       = (int16_t)(buf[4]  << 8 | buf[5]);
    temp_raw = (int16_t)(buf[6]  << 8 | buf[7]);
    gx       = (int16_t)(buf[8]  << 8 | buf[9]);
    gy       = (int16_t)(buf[10] << 8 | buf[11]);
    gz       = (int16_t)(buf[12] << 8 | buf[13]);

    // Convert raw temp to Celsius
    temp = (temp_raw / 340.0f) + 36.53f;

    // Convert raw accel to g (±2g range, sensitivity=16384 LSB/g)
    float ax_g = ax / 16384.0f;
    float ay_g = ay / 16384.0f;
    float az_g = az / 16384.0f;

    // Convert raw gyro to °/s (±250°/s range, sensitivity=131 LSB/°/s)
    float gx_ds = gx / 131.0f;
    float gy_ds = gy / 131.0f;
    float gz_ds = gz / 131.0f;

    printf("--- MPU6050 Readings ---\r\n");
    printf("Accel (raw): X=%6d  Y=%6d  Z=%6d\r\n", ax, ay, az);
    printf("Accel (g)  : X=%.3f  Y=%.3f  Z=%.3f\r\n", ax_g, ay_g, az_g);
    printf("Gyro  (raw): X=%6d  Y=%6d  Z=%6d\r\n", gx, gy, gz);
    printf("Gyro (d/s) : X=%.2f  Y=%.2f  Z=%.2f\r\n", gx_ds, gy_ds, gz_ds);
    printf("Temperature: %.2f C\r\n\r\n", temp);
}
/* USER CODE END 0 */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

    /* USER CODE BEGIN 2 */
    // Enable ITM/SWO
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    ITM->LAR  = 0xC5ACCE55;
    ITM->TCR |= ITM_TCR_ITMENA_Msk;
    ITM->TER |= 1UL;

    HAL_Delay(500); // Wait for ITM to stabilize
    printf("=== MPU6050 HAL Test ===\r\n");

    MPU6050_Init();
    /* USER CODE END 2 */

    while (1)
    {
        /* USER CODE BEGIN WHILE */
        MPU6050_Read();
        HAL_Delay(500);
        /* USER CODE END WHILE */
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // LED off initially

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif
