#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>

int max_y, max_x;

void put_symbol(int y, int x, char c) {
    mvaddch(y, x, c);
}

void move_coordinates(int* y, int* x, int dir) {
    switch (dir) {
        case KEY_DOWN:
            *y = 2 + (*y + max_y - 3) % (max_y - 2);
            return;

        case KEY_UP:
            *y = 2 + (*y + max_y - 5) % (max_y - 2);
            return;

        case KEY_RIGHT:
            *x = (*x + 1) % max_x;
            return;

        case KEY_LEFT:
            *x = (*x + max_x - 1) % max_x;
            return;
    }
}

void print_lives(int lives) {
    mvprintw(0, 0, "lives: %d   ", lives);
}

void print_score(int score) {
    mvprintw(0, max_x / 2 - 5, "score: %d     ", score);
}

int calc_bonus(int l, int new, int old) {
    bool new_up_down = ((new == KEY_UP) || (new == KEY_DOWN));
    bool old_up_down = ((old == KEY_UP) || (old == KEY_DOWN));

    if (new != 0 && old != 0 && new_up_down != old_up_down) {
        return l;
    } else {
        return 0;
    }
}

void add_enemies(char* field, int cur_y, int cur_x) {
    int count = 2;
    while (count > 0) {
        int x = rand() % max_x;
        int y = 2 + rand() % (max_y - 2);
        if (x != cur_x || y != cur_y) {
            count--;
            field[y * max_x + x] = '*';
            put_symbol(y, x, '*');
        }
    }
}

void init_game(char* field, int* lives, int* score, int* cur_y, int* cur_x) {
    for (size_t y = 2; y <= max_y; ++y) {
        for (size_t x = 0; x <= max_x; ++x) {
            field[y * max_x + x] = ' ';
            put_symbol(y, x, field[y * max_x + x]);
        }
    }

    *lives = 3;
    *score = 0;

    print_lives(*lives);
    print_score(*score);

    *cur_x = max_x / 2;
    *cur_y = max_y / 2;

    field[*cur_y * max_y + *cur_x] = ' ';
    put_symbol(*cur_y, *cur_x, '@');

    for (size_t x = 0; x < max_x; ++x) {
        put_symbol(1, x, '_');
    }
}

int main(int argc, char** argv) {
    initscr();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, true);
    getmaxyx(stdscr, max_y, max_x);
    srand(time(NULL));

    char* field = (char*) malloc((max_y + 1) * max_x * sizeof(char));
    int lives, score, cur_y, cur_x;

    while (true) {
        init_game(field, &lives, &score, &cur_y, &cur_x);

        int ch = 0, old_ch = 0, dir_length = 0;

        while (lives > 0) {
            ch = getch();

            if (ch != ERR && ch != old_ch && ch != 0 && old_ch != 0) {
                score += calc_bonus(dir_length, ch, old_ch);
                dir_length = 0;
            }

            switch (ch) {
                case 'q':
                    mvprintw(max_y / 2, max_x / 2 - 8, "Really quit? (y/n)");
                    while (true) {
                        ch = getch();
                        if (ch == 'y') {
                            goto quit;
                        } else if (ch == 'n') {
                            break;
                        }
                    }
                    break;

                case KEY_UP:
                case KEY_DOWN:
                case KEY_LEFT:
                case KEY_RIGHT:
                    old_ch = ch;
                    break;

                default:
                    ch = old_ch;
                    break;
            }

            put_symbol(cur_y, cur_x, ' ');
            move_coordinates(&cur_y, &cur_x, ch);
            put_symbol(cur_y, cur_x, '@');
            ++dir_length;

            if (field[cur_y * max_x + cur_x] == '*') {
                field[cur_y * max_x + cur_x] = ' ';
                print_lives(--lives);
            }

            if (ch != 0) {
                print_score(++score);
                add_enemies(field, cur_y, cur_x);
            }

            usleep(20000);
        }

        score += calc_bonus(dir_length, ch, old_ch);
        print_score(score);

        mvprintw(max_y / 2, max_x / 2 - 8, "Play again? (y/n)");
        while (true) {
            ch = getch();
            if (ch == 'n') {
                goto quit;
            } else if (ch == 'y') {
                break;
            }
        }
    }

quit:
    free(field);
    endwin();
    return 0;
}
