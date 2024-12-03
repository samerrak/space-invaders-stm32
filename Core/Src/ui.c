/*
 * ui.c
 *
 *  Created on: Nov 24, 2024
 *      Author: rokob
 */

#include "ui.h"

extern UART_HandleTypeDef huart1; // Ensure this is declared
extern char display[25][60];
extern char ui_string[1760];
extern struct Position alien_positions[70];
extern struct Position bullet_positions[300];



extern int16_t x_position;


// Track current selection
static int current_selection = 0;

// Map names array
extern const char* map_names[NUM_MAPS];

char art[10][69] = {
        "                           _                    _               ",
        "                          (_)                  | |              ",
        " ___ _ __   __ _  ___ ___ _ _ ____   ____ _  __| | ___ _ __ ___ ",
        "/ __| '_ \\ / _` |/ __/ _ \\ | '_ \\ \\ / / _` |/ _` |/ _ \\ '__/ __|",
        "\\__ \\ |_) | (_| | (_|  __/ | | | \\ V / (_| | (_| |  __/ |  \\__ \\",
        "|___/ .__/ \\__,_|\\___\\___|_|_| |_|\\_/ \\__,_|\\__,_|\\___|_|  |___/",
        "    | |                                                         ",
        "    |_|                                                         ",
    };


// HELPERS
int idx_to_pos(int row, int col) {
	return 69 + row*64 + col + 1;
}

// MAIN MENU {
void print_main_menu() {
	HAL_UART_Transmit(&huart1, (uint8_t*)"\x1B[2J\r", strlen("\x1B[2J\r"), 100);
	// Dynamically allocate buffer
    char* buffer = (char*)malloc(512 * sizeof(char));
    if (buffer == NULL) {
        // Handle allocation failure
        return;
    }

    for (int i = 0; i < 8; i++) {
    	printOne("%s\n\r", art[i]);
    }



    // Clear buffer
    memset(buffer, 0, 256);

    // Add header
    strcpy(buffer, "\n"
    		"Available Maps:\r\n");

    // Add map names
    for(int i = 0; i < NUM_MAPS; i++) {
        strcat(buffer, map_names[i]);
        strcat(buffer, "\r\n");
    }

    // Transmit
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);

    // Free allocated memory
    free(buffer);

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
			if (alien_positions[i].row > 0 && alien_positions[i].row < 23) {
				if (alien_positions[i].row == 22){
					gameOver = 1;
				} else {
					alien_positions[i].row++;
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
	strcpy(ui_string, "\033[H \r"
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
