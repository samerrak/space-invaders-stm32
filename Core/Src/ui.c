/*
 * ui.c
 *
 *  Created on: Nov 24, 2024
 *      Author: Roko, Tim, Samer, Ralph
 */

#include "ui.h"
extern struct Position {
	int8_t row;
	int8_t col;
};
extern UART_HandleTypeDef huart1; // Ensure this is declared
extern char display[25][60];
extern char ui_string[1760];
extern struct Position alien_positions[70];
extern struct Position bullet_positions[300];

extern int16_t x_position;


// HELPERS
int idx_to_pos(int row, int col) {
	return 69 + row*64 + col + 1;
}


// PUBLIC
int compute_new_UI_frame(int moveBullets, int moveAliens) {
	uint8_t gameOver = 0;

	// Loop for bullets
	if (moveBullets){
		for (int i = 0; i < 300; i++) {
			if (bullet_positions[i].row > 0 && bullet_positions[i].row < 24) {
				bullet_positions[i].row--;
			} else {
				bullet_positions[i].row = -1;
				bullet_positions[i].col = -1;
			}
		}
	}

	// Loop for aliens
	if (moveAliens){
		for (int i = 0; i < 70; i++) {
			if (alien_positions[i].row > 0 && alien_positions[i].row < 24) {
				alien_positions[i].row++;
				if (alien_positions[i].row == 58){
					gameOver = 1;
				}
			} else {
				alien_positions[i].row = -1;
				alien_positions[i].col = -1;
			}
		}
	}

	update_canvas();

	ui_string[1759] = 0;
	uint16_t len= strlen(ui_string);
	HAL_UART_Transmit(&huart1, (uint8_t*)ui_string, len, 100);

	return gameOver;
}


void start_wave() {
	for (int i = 0; i < 70; i++) {
		if (i < 20){
			alien_positions[i].row = 1;
			alien_positions[i].col = 3*i + 1;
		} else {
			alien_positions[i].row = -1;
			alien_positions[i].col = -1;
		}
	}

	for (int i = 0; i < 300; i++) {
		bullet_positions[i].row = -1;
		bullet_positions[i].col = -1;
	}
}

void update_canvas() {
	reset_canvas();

//	 Draw bullets
	for (int i = 0; i < 300; i++) {
		if (bullet_positions[i].row != -1) {
			ui_string[idx_to_pos(bullet_positions[i].row, bullet_positions[i].col)] = '!';
		}
	}
//
//	// Draw player
	ui_string[idx_to_pos(23, x_position-1)] = '/';
	ui_string[idx_to_pos(23, x_position)] = '|';
	ui_string[idx_to_pos(23, x_position+1)] = '\\';
//
//	// Draw aliens
	draw_aliens();
}


void draw_aliens() {
	for (int i = 0; i < 70; i++) {
		if (alien_positions[i].row == -1) continue;
		int8_t row = alien_positions[i].row;
		int8_t col = alien_positions[i].col;
		uint8_t hit = 0;
		// Handle collision with bullet
		if (ui_string[idx_to_pos(row, col-1)] == '!') {
			handle_collision(row, col-1);
			hit = 1;
		}
		if (ui_string[idx_to_pos(row, col)] == '!') {
			handle_collision(row, col);
			hit = 1;
		}
		if (ui_string[idx_to_pos(row, col+1)] == '!') {
			handle_collision(row, col+1);
			hit = 1;
		}
		if (ui_string[idx_to_pos(row+1, col-1)] == '!') {
			handle_collision(row+1, col-1);
			hit = 1;
		}
		if (ui_string[idx_to_pos(row+1, col)] == '!') {
			handle_collision(row+1, col);
			hit = 1;
		}
		if (ui_string[idx_to_pos(row+1, col+1)] == '!') {
			handle_collision(row+1, col+1);
			hit = 1;
		}

		if (hit) {
			ui_string[idx_to_pos(row, col-1)] = '*';
			ui_string[idx_to_pos(row, col)] = '*';
			ui_string[idx_to_pos(row, col+1)] = '*';
			ui_string[idx_to_pos(row+1, col-1)] = '*';
			ui_string[idx_to_pos(row+1, col)] = '*';
			ui_string[idx_to_pos(row+1, col+1)] = '*';

			alien_positions[i].row = -1;
			alien_positions[i].col = -1;
		} else {
			ui_string[idx_to_pos(row, col-1)] = '_';
			ui_string[idx_to_pos(row, col)] = '_';
			ui_string[idx_to_pos(row, col+1)] = '_';
			ui_string[idx_to_pos(row+1, col-1)] = '\\';
			ui_string[idx_to_pos(row+1, col)] = '|';
			ui_string[idx_to_pos(row+1, col+1)] = '/';
		}
	}
}

void handle_collision(row, col) {
	for (int i = 0; i < 300; i++) {
		if (bullet_positions[i].row == row && bullet_positions[i].col == col) {
			bullet_positions[i].row = -1;
			bullet_positions[i].col = -1;
		}
	}
}

void reset_canvas() {
	// 69 (5 + 62 + 2) bytes
	strcpy(ui_string, "\033[H\r"
			"##############################################################\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"#                                                            #\n\r"
			"##############################################################"
);
}


void shoot() {
	for (int i = 0; i < 300; i++) {
		if (bullet_positions[i].row == -1) {
			bullet_positions[i].row = 2;
			bullet_positions[i].col = x_position;
			break;
		}
	}
}
