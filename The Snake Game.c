#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

#define BOARD_WIDTH 40
#define BOARD_HEIGHT 20
#define MAX_SNAKE_LENGTH (BOARD_WIDTH * BOARD_HEIGHT)

typedef struct {
	int x;
	int y;
} Point;

static Point snake[MAX_SNAKE_LENGTH];
static Point food;
static int snake_length;
static int score;
static int game_over;

#ifdef _WIN32
static void sleep_ms(int milliseconds) {
	Sleep((DWORD)milliseconds);
}

static void terminal_start(void) {
	printf("\x1b[?25l");
}

static void terminal_end(void) {
	printf("\x1b[?25h\x1b[0m\n");
}

static int read_key(void) {
	if (!_kbhit()) {
		return 0;
	}

	int key = _getch();
	if (key == 0 || key == 224) {
		key = _getch();
		if (key == 72) return 'w';
		if (key == 80) return 's';
		if (key == 75) return 'a';
		if (key == 77) return 'd';
	}
	return key;
}
#else
static struct termios original_terminal;

static void sleep_ms(int milliseconds) {
	usleep((useconds_t)milliseconds * 1000);
}

static void terminal_end(void) {
	tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal);
	printf("\x1b[?25h\x1b[0m\n");
}

static void terminal_start(void) {
	struct termios raw_terminal;
	tcgetattr(STDIN_FILENO, &original_terminal);
	raw_terminal = original_terminal;
	raw_terminal.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &raw_terminal);
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	atexit(terminal_end);
	printf("\x1b[?25l");
}

static int read_key(void) {
	unsigned char key;
	if (read(STDIN_FILENO, &key, 1) != 1) {
		return 0;
	}
	return key;
}
#endif

static int is_snake_at(int x, int y) {
	int index;
	for (index = 0; index < snake_length; index++) {
		if (snake[index].x == x && snake[index].y == y) {
			return 1;
		}
	}
	return 0;
}

static void place_food(void) {
	do {
		food.x = rand() % BOARD_WIDTH;
		food.y = rand() % BOARD_HEIGHT;
	} while (is_snake_at(food.x, food.y));
}

static void reset_game(void) {
	int index;
	snake_length = 4;
	score = 0;
	game_over = 0;

	for (index = 0; index < snake_length; index++) {
		snake[index].x = BOARD_WIDTH / 2 - index;
		snake[index].y = BOARD_HEIGHT / 2;
	}
	place_food();
}

static void draw_game(void) {
	int x;
	int y;
	int index;

	printf("\x1b[H\x1b[2J");
	printf("SNAKE  |  Score: %-4d |  Move: arrows/WASD  Quit: Q\n", score);
	printf("+");
	for (x = 0; x < BOARD_WIDTH; x++) printf("--");
	printf("+\n");

	for (y = 0; y < BOARD_HEIGHT; y++) {
		printf("|");
		for (x = 0; x < BOARD_WIDTH; x++) {
			char tile = ' ';
			if (food.x == x && food.y == y) {
				tile = '*';
			}
			for (index = 0; index < snake_length; index++) {
				if (snake[index].x == x && snake[index].y == y) {
					tile = index == 0 ? 'O' : 'o';
					break;
				}
			}
			printf("%c ", tile);
		}
		printf("|\n");
	}

	printf("+");
	for (x = 0; x < BOARD_WIDTH; x++) printf("--");
	printf("+\n");
	fflush(stdout);
}

static void update_game(int direction_x, int direction_y) {
	Point next = { snake[0].x + direction_x, snake[0].y + direction_y };
	int index;
	int ate_food = next.x == food.x && next.y == food.y;
	int collision_length = ate_food ? snake_length : snake_length - 1;

	if (next.x < 0 || next.x >= BOARD_WIDTH || next.y < 0 || next.y >= BOARD_HEIGHT) {
		game_over = 1;
		return;
	}
	for (index = 0; index < collision_length; index++) {
		if (snake[index].x == next.x && snake[index].y == next.y) {
			game_over = 1;
			return;
		}
	}

	if (ate_food && snake_length < MAX_SNAKE_LENGTH) {
		snake_length++;
		score += 10;
	}
	for (index = snake_length - 1; index > 0; index--) {
		snake[index] = snake[index - 1];
	}
	snake[0] = next;
	if (ate_food) {
		place_food();
	}
}

int main(void) {
	int direction_x = 1;
	int direction_y = 0;
	int key;

	srand((unsigned int)time(NULL));
	terminal_start();
	reset_game();

	while (!game_over) {
		key = read_key();
		if ((key == 'q') || (key == 'Q')) {
			break;
		}
		if ((key == 'w' || key == 'W') && direction_y == 0) {
			direction_x = 0; direction_y = -1;
		} else if ((key == 's' || key == 'S') && direction_y == 0) {
			direction_x = 0; direction_y = 1;
		} else if ((key == 'a' || key == 'A') && direction_x == 0) {
			direction_x = -1; direction_y = 0;
		} else if ((key == 'd' || key == 'D') && direction_x == 0) {
			direction_x = 1; direction_y = 0;
		}

		update_game(direction_x, direction_y);
		draw_game();
		sleep_ms(120);
	}

	draw_game();
	terminal_end();
	if (game_over) {
		printf("Game over! Final score: %d\n", score);
	} else {
		printf("Thanks for playing. Final score: %d\n", score);
	}
	return 0;
}

