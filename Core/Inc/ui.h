
/*
 * ui.h
 *
 *  Created on: Nov 24, 2024
 *      Author: Roko, Samer, Tim, Ralph
 */

#ifndef INC_UI_H_
#define INC_UI_H_

#include "arm_math.h"
#include <stdio.h>
#include "main.h"

int compute_new_UI_frame(int moveBullets, int moveAliens);
void start_wave();
void shoot();
void print_main_menu();
void print_game_over();
#endif /* INC_UI_H_ */
