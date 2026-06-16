#include "I2C.h"
#include "LCD.h"
#include "Accel.h"

#include "esp_rom_sys.h"
#include "esp_log.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define WIN_LEN 32

#define DELAY 20000 /* us */

#define RAD_TO_DEG(rad) ((rad) * 180.0f / M_PI)

const char *TAG = "main";

typedef enum
{
    ALPHA = 0,
    BETA,
    GAMMA,
    ANGLE_COUNT
} angle_t;

typedef enum
{
    X = 0,
    Y,
    Z,
    ELEM_COUNT
} elem_t;

void app_main(void)
{
    I2C_Init();
    LCD_Init();
    Accel_Init();

    uint32_t cnt = 0;

    while(true)
    {
        cnt++;

        int16_t accel[ELEM_COUNT];
        Accel_ReadRaw(accel);

        if(cnt % 25 == 0) // Print the angles on the LCD
        {
            if(accel[X] == 0 && accel[Y] == 0 && accel[Z] == 0)
            {
                ESP_LOGW(TAG, "Can not compute atan2f because all accelerometer values are zero.");
                esp_rom_delay_us(DELAY);

                continue;
            }

            uint32_t squared[] =
            {
                accel[X] * accel[X],
                accel[Y] * accel[Y],
                accel[Z] * accel[Z]
            };

            float angle[] =                                                   // Every angle is in range of [-90, 90] degrees
            {
                RAD_TO_DEG(atan2f(accel[X], sqrtf(squared[Y] + squared[Z]))), // alpha, angle between the x axis and the horizontal plane
                RAD_TO_DEG(atan2f(accel[Y], sqrtf(squared[X] + squared[Z]))), // beta,  angle between the y axis and the horizontal plane
                RAD_TO_DEG(atan2f(accel[Z], sqrtf(squared[X] + squared[Y])))  // gamma, angle between the z axis and the horizontal plane
            };

            char str[17]; // 16 characters in a LCD row + the null terminator

            LCD_SetCursor(0, 0);
            snprintf(str, sizeof(str), "%s%.1f\xDF ", angle[ALPHA] < 0 ? "-" : " ", fabsf(angle[ALPHA]));
            LCD_Print(str);

            LCD_SetCursor(0, 10);
            snprintf(str, sizeof(str), "%s%.1f\xDF%s", angle[BETA] < 0 ? "-" : " ", fabsf(angle[BETA]), fabsf(angle[BETA]) < 10 ? " " : "");
            LCD_Print(str);

            LCD_SetCursor(1, 5);
            snprintf(str, sizeof(str), "%s%.1f\xDF ", angle[GAMMA] < 0 ? "-" : " ", fabsf(angle[GAMMA]));
            LCD_Print(str);
        }

        esp_rom_delay_us(DELAY);
    }
}