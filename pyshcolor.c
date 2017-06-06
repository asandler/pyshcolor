#include <fcntl.h>
#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int max_y, max_x;
char player_name[25];
char* field;

void quit() {
    if (field) {
        free(field);
    }
    endwin();
    exit(0);
}

void sighandler(int signo) {
    if (signo == SIGWINCH) {
        quit();
    }
}

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
    mvprintw(0, 0, "lives: ");//%d   ", lives);
    attron(COLOR_PAIR(1));
    for (int i = 0; i < 3; ++i) {
        if (i < lives) {
            addch(ACS_CKBOARD | A_ALTCHARSET);
        } else {
            addch(' ');
        }
    }
    attroff(COLOR_PAIR(1));
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

void add_enemies(int cur_y, int cur_x) {
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

void init_game(int* lives, int* score, int* cur_y, int* cur_x) {
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

void create_score_table() {
    struct stat stat_info;
    if (stat("score.bin", &stat_info) == -1) {
        int fd = open("score.bin", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        char buf[25];
        bzero(buf, 24);
        strncpy(buf, "player", 24);

        for (int i = 0; i < 20; ++i) {
            write(fd, buf, 24);
        }
        close(fd);
    }
}

void print_and_save_score_table(int score) {
    char names[20][20];
    int scores[20];
    unsigned char buf[25];
    int place = -1;

    bzero(scores, 20);
    create_score_table();

    int fd = open("score.bin", O_RDONLY);
    for (int i = 0; i < 20; ++i) {
        if (read(fd, buf, 24) == 24) {
            strncpy(names[i], buf, 16);
            for (int j = 0; j < 8; ++j) {
                scores[i] *= 256;
                scores[i] += (int) buf[16 + j];
            }
        } else {
            close(fd);
            return;
        }
    }
    close(fd);

    for (int i = 0; i < 20; ++i) {
        if (score > scores[i]) {
            for (int j = 19; j >= i + 1; --j) {
                scores[j] = scores[j - 1];
                strcpy(names[j], names[j - 1]);
            }
            scores[i] = score;
            strncpy(names[i], player_name, 16);
            place = i;
            break;
        }
    }

    for (int i = 0; i < 20; ++i) {
        if (i == place) {
            attron(COLOR_PAIR(3));
            mvprintw(3 + i, 0, "* %-16s...  %-8d", names[i], scores[i]);
        } else {
            if (i == 0) {
                attron(COLOR_PAIR(1));
            } else if (i == 1 || i == 2) {
                attron(COLOR_PAIR(2));
            } else {
                attron(COLOR_PAIR(4));
            }
            mvprintw(3 + i, 0, "  %-16s...  %-8d", names[i], scores[i]);
        }
    }
    attron(COLOR_PAIR(4));

    fd = open("score.bin", O_WRONLY);
    for (int i = 0; i < 20; ++i) {
        write(fd, names[i], 16);

        int scr = scores[i];
        for (int j = 7; j >= 0; --j) {
            buf[j] = (unsigned char) (scr % 256);
            scr /= 256;
        }
        write(fd, buf, 8);
    }
    close(fd);
}

int main(int argc, char** argv) {
    initscr();
    keypad(stdscr, TRUE);
    curs_set(0);
    getmaxyx(stdscr, max_y, max_x);
    srand(time(NULL));

    signal(SIGWINCH, sighandler);

    start_color();
    use_default_colors();
    attron(A_BOLD);
    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_YELLOW, -1);
    init_pair(3, COLOR_GREEN, -1);
    init_pair(4, COLOR_WHITE, -1);

    mvprintw(max_y / 2, max_x / 2 - 6, "Your name: ");
    bzero(player_name, strlen(player_name));
    getnstr(player_name, 16);
    for (int i = 0; i < 25; ++i) {
        if (player_name[i] < ' ' || player_name[i] > 'z') {
            player_name[i] = ' ';
        }
        if (i > 14) {
            player_name[i] = 0;
        }
    }

    nodelay(stdscr, true);
    noecho();

    field = (char*) malloc((max_y + 1) * max_x * sizeof(char));
    int lives, score, cur_y, cur_x;

    while (true) {
        init_game(&lives, &score, &cur_y, &cur_x);

        int ch = 0, old_ch = 0, dir_length = 0;
        bool started = false;

        while (lives > 0) {
            ch = getch();

            switch (ch) {
                case 'q':
                    mvprintw(max_y / 2, max_x / 2 - 10, "  Really quit? (y/n)   ");
                    while (true) {
                        ch = getch();
                        if (ch == 'y') {
                            quit();
                        } else if (ch == 'n') {
                            lives = 0;
                            break;
                        }
                    }
                    break;

                case KEY_UP:
                case KEY_DOWN:
                case KEY_LEFT:
                case KEY_RIGHT:
                    if (ch != old_ch) {
                        score += calc_bonus(dir_length, ch, old_ch);
                        dir_length = 0;
                    }

                    old_ch = ch;
                    started = true;

                    //if (ch != ERR && ch != old_ch && ch != 0 && old_ch != 0) {

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

            if (ch != 0 && started) {
                print_score(++score);
                add_enemies(cur_y, cur_x);
            }

            usleep(20000);
        }

        score += calc_bonus(dir_length, ch, old_ch);
        print_score(score);

        print_and_save_score_table(score);

        mvprintw(max_y / 2, max_x / 2 - 10, "  Play again? (y/n)   ");
        while (true) {
            ch = getch();
            if (ch == 'n') {
                quit();
            } else if (ch == 'y') {
                break;
            }
        }
    }
}
