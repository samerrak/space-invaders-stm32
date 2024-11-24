#include "kalman.h"

float kalman_filter_CMSIS(kalman_state *kstate, float measurement) {
    float temp_p, temp_k, temp_x;

    // Predict
    arm_add_f32(&kstate->p, &kstate->q, &temp_p, 1);  // p = p + q

    // Update
    arm_add_f32(&temp_p, &kstate->r, &temp_k, 1);  // temp_k = p + r
    temp_k = temp_p / temp_k;  // k = p / (p + r)

    float temp_diff;
    arm_sub_f32(&measurement, &kstate->x, &temp_diff, 1);  // temp_diff = measurement - x
    arm_mult_f32(&temp_k, &temp_diff, &temp_x, 1);  // temp_x = k * (measurement - x)
    arm_add_f32(&kstate->x, &temp_x, &temp_x, 1);  // x = x + k * (measurement - x)

    // Update covariance
    float temp_p_update;
    float one_minus_k = 1.0f - temp_k;
    arm_mult_f32(&one_minus_k, &temp_p, &temp_p_update, 1);  // p = (1 - k) * p

    // Save updated values
    kstate->p = temp_p_update;
    kstate->x = temp_x;
    kstate->k = temp_k;

    return kstate->x;
}