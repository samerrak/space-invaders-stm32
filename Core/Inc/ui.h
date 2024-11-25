/*
 * ui.h
 *
 *  Created on: Nov 24, 2024
 *      Author: rokob
 */

#ifndef INC_UI_H_
#define INC_UI_H_

#include "arm_math.h"
#include <stdio.h>
#include "main.h"

void reset_display();
int compute_new_UI_frame(int moveBullets, int moveAliens);
void start_wave();

#endif /* INC_UI_H_ */
