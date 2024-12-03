// util.h
#include <stdint.h>
#include "arm_math.h"
#include <stdio.h>
#include "main.h"
#include "stm32l4s5i_iot01_psensor.h"
#include <stdarg.h>

// Constants
#define ACCEL_THRESHOLD 10.0f  // Threshold for tilt detection
#define NUM_MAPS 5
#define X_MAP_SIZE 58

// Function declarations
float calculate_pitch(int16_t *accel_data);
int16_t tilt_detection(int16_t *filtered_acceleration, int16_t x_position);
void printOne(char* string, ...);
