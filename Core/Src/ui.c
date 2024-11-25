/*
 * ui.c
 *
 *  Created on: Nov 24, 2024
 *      Author: rokob
 */

#include "ui.h"

extern UART_HandleTypeDef huart1; // Ensure this is declared
extern char display[10][40];
extern char ui_string[431];
extern int16_t x_position;

// HELPERS
int idx_to_pos(int i, int j) {
	return i*42 + j + 11;
}


// PUBLIC
int compute_new_UI_frame(int moveBullets, int moveAliens) {
	uint8_t gameOver = 0;
	strcpy(ui_string, "\n\n\n\n\n\n\n\n\n\r\n");
	// Move player
	for (int i = 0; i < 10; i++) {
		if (display[i][0] == '8') display[i][0] = ' ';
	}
	display[x_position][0] = '8';
	// Loop for bullets
	if (moveBullets){
		for (int i = 0; i < 10; i++){
			for (int j = 38; j > 0; j--){
				if (display[i][j] == '-') {
					display[i][j] = ' ';
					if (display[i][j+1] == '^') display[i][j+1] = ' ';
					else display[i][j+1] = '-';
				}
			}
			if (display[i][39] == '-') display[i][39] = ' ';
		}
	}

	// Loop for aliens
	if (moveAliens){
		for (int i = 0; i < 10; i++){
			for (int j = 1; j < 40; j++){
				if (display[i][j] == '^') {
					display[i][j] = ' ';
					if (display[i][j-1] == '-') display[i][j-1] = ' ';
					else display[i][j-1] = '^';
				}
			}
			if (display[i][0] == '^') gameOver = 1;
		}
	}


	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 40; j++) {
			uint16_t str_index = idx_to_pos(i, j);
			ui_string[str_index] = display[i][j];
		}
		ui_string[idx_to_pos(i+1, -2)] = '\r';
		ui_string[idx_to_pos(i+1, -1)] = '\n';
	}
	ui_string[429] = 0;
	uint16_t len= strlen(ui_string);
	HAL_UART_Transmit(&huart1, (uint8_t*)ui_string, len, 100);

	return gameOver;
}


void start_wave() {
	for (int i = 0; i < 10; i++) {
		display[i][39] = '^';
	}
}


void reset_display() {
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 40; j++) {
			display[i][j] = ' ';
		}
		if (i == x_position) {
			display[i][0] = '8';
		}
	}
}
