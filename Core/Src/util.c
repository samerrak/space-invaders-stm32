/* Constants */
#define ACCEL_THRESHOLD 10.0f  // Threshold for tilt detection
#define X_MAP_SIZE 10

#include "util.h"



extern UART_HandleTypeDef huart1; // Ensure this is declared

/* Detecting pitch */
float calculate_pitch(int16_t *accel_data) {
    // Pitch = atan2(accel_data[0], sqrt(accel_data[1]^2 + accel_data[2]^2))
    return atan2(accel_data[0], sqrt(accel_data[1] * accel_data[1] + accel_data[2] * accel_data[2])) * 180.0 / M_PI;
}

/* Detecting tilt */
int16_t tilt_detection(int16_t *filtered_acceleration, int16_t x_position) {
    float pitch;

    // Calculate pitch (left-right tilt)
    pitch = calculate_pitch(filtered_acceleration);
    int new_x = 0;

    // Update x position of the user
    if (pitch > ACCEL_THRESHOLD && x_position < X_MAP_SIZE-1) {
    	new_x = ++x_position;
        return new_x;
    } else if (pitch < -ACCEL_THRESHOLD && x_position > 0) {
    	new_x = --x_position;
        return new_x;
    }

    return x_position;
}

/* Print a variable number of arguments */
void printOne(char* string, ...) {
	char output[120];
	va_list args;
	va_start(args, string);
	vsnprintf(output, sizeof(output), string, args);
	uint16_t len = strlen(output);
	HAL_UART_Transmit(&huart1, (uint8_t *)output, len, 120);
}
