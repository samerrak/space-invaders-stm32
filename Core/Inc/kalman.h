// kalman.h 
#include "arm_math.h" // Include necessary headers

typedef struct {
    float32_t q;  // Process noise covariance
    float32_t r;  // Measurement noise covariance
    float32_t x;  // Filtered value
    float32_t p;  // Estimate error covariance
    float32_t k;  // Kalman gain
} kalman_state;

float kalman_filter_CMSIS(kalman_state *kstate, float measurement); // Function prototype

