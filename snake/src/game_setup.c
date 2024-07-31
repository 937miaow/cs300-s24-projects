#include "game_setup.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "game.h"

// Some handy dandy macros for decompression
#define E_CAP_HEX 0x45
#define E_LOW_HEX 0x65
#define G_CAP_HEX 0x47
#define G_LOW_HEX 0x67
#define S_CAP_HEX 0x53
#define S_LOW_HEX 0x73
#define W_CAP_HEX 0x57
#define W_LOW_HEX 0x77
#define DIGIT_START 0x30
#define DIGIT_END 0x39

/** Initializes the board with walls around the edge of the board.
 *
 * Modifies values pointed to by cells_p, width_p, and height_p and initializes
 * cells array to reflect this default board.
 *
 * Returns INIT_SUCCESS to indicate that it was successful.
 *
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 */
enum board_init_status initialize_default_board(int** cells_p, size_t* width_p,
                                                size_t* height_p) {
    *width_p = 20;
    *height_p = 10;
    int* cells = malloc(20 * 10 * sizeof(int));
    *cells_p = cells;
    for (int i = 0; i < 20 * 10; i++) {
        cells[i] = PLAIN_CELL;
    }

    // Set edge cells!
    // Top and bottom edges:
    for (int i = 0; i < 20; ++i) {
        cells[i] = FLAG_WALL;
        cells[i + (20 * (10 - 1))] = FLAG_WALL;
    }
    // Left and right edges:
    for (int i = 0; i < 10; ++i) {
        cells[i * 20] = FLAG_WALL;
        cells[i * 20 + 20 - 1] = FLAG_WALL;
    }

    // Set grass cells!
    // Top and bottom edges:
    for (int i = 1; i < 19; ++i) {
        cells[i + 20] = FLAG_GRASS;
        cells[i + (20 * (9 - 1))] = FLAG_GRASS;
    }
    // Left and right edges:
    for (int i = 1; i < 9; ++i) {
        cells[i * 20 + 1] = FLAG_GRASS;
        cells[i * 20 + 19 - 1] = FLAG_GRASS;
    }

    // Add snake
    cells[20 * 2 + 2] = FLAG_SNAKE;

    return INIT_SUCCESS;
}

/** Initialize variables relevant to the game board.
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 *  - snake_p: a pointer to your snake struct (not used until part 3!)
 *  - board_rep: a string representing the initial board. May be NULL for
 * default board.
 */
enum board_init_status initialize_game(int** cells_p, size_t* width_p,
                                       size_t* height_p, snake_t* snake_p,
                                       char* board_rep) {
    // TODO: implement!
    g_game_over = 0;
    g_score = 0;
    enum board_init_status result;

    if (board_rep == NULL) {
        result = initialize_default_board(cells_p, width_p, height_p);
    } else {
        result = decompress_board_str(cells_p, width_p, height_p, snake_p,
                                      board_rep);
    }
    if (result == INIT_SUCCESS) place_food(*cells_p, *width_p, *height_p);

    return result;
}

/** Takes in a string `compressed` and initializes values pointed to by
 * cells_p, width_p, and height_p accordingly. Arguments:
 *      - cells_p: a pointer to the pointer representing the cells array
 *                 that we would like to initialize.
 *      - width_p: a pointer to the width variable we'd like to initialize.
 *      - height_p: a pointer to the height variable we'd like to initialize.
 *      - snake_p: a pointer to your snake struct (not used until part 3!)
 *      - compressed: a string that contains the representation of the board.
 * Note: We assume that the string will be of the following form:
 * B24x80|E5W2E73|E5W2S1E72... To read it, we scan the string row-by-row
 * (delineated by the `|` character), and read out a letter (E, S or W) a number
 * of times dictated by the number that follows the letter.
 */
enum board_init_status decompress_board_str(int** cells_p, size_t* width_p,
                                            size_t* height_p, snake_t* snake_p,
                                            char* compressed) {
    // TODO: implement!
    // 用于`strtok_r`的指针
    char* token;
    char* rest = compressed;

    // 动态数组用于存储子字符串指针
    char** tokens = NULL;
    size_t cnt = 0;

    // 分割字符串
    while ((token = strtok_r(rest, "|", &rest))) {
        // 重新分配空间
        tokens = realloc(tokens, sizeof(char*) * (cnt + 1));
        if (tokens == NULL) exit(EXIT_FAILURE);

        // 分配空间并复制子字符串
        tokens[cnt] = malloc(strlen(token) + 1);
        if (tokens[cnt] == NULL) exit(EXIT_FAILURE);
        strcpy(tokens[cnt], token);

        // 计数器加一
        cnt++;

        // 记得free！！！！
    }

    // 初始化board
    char* left = tokens[0];
    char* right = tokens[0];
    int lentoken0 = strlen(tokens[0]);
    // 获取宽高 第一个字符串正确格式是类似于B24x80
    for (int i = 0; i < lentoken0; ++i) {
        char c = tokens[0][i];
        if (c != 'B' && c != 'x' && (c < '0' || c > '9')) {
            for (size_t l = 0; l < cnt; l++) {
                free(tokens[l]);
            }
            free(tokens);
            return INIT_ERR_BAD_CHAR;
        }

        if (c == 'B') {
            left = &tokens[0][i + 1];
        } else if (c == 'x') {
            right = &tokens[0][i];
            break;
        }
    }
    int height_len = right - left;
    char* height_temp = strndup(left, height_len);
    char* width_temp = strdup(right + 1);
    *height_p = atoi(height_temp);
    *width_p = atoi(width_temp);

    int height = *height_p;
    int width = *width_p;
    int* cells = malloc(height * width * sizeof(int));

    free(height_temp);
    free(width_temp);

    *cells_p = cells;
    for (int i = 0; i < height * width; i++) {
        cells[i] = PLAIN_CELL;
    }

    // 读取每一段字符串
    left = tokens[0];
    right = tokens[0];

    int height_cnt = 0;
    int snake_num = 0;  // 记录蛇的数量

    for (size_t i = 1; i < cnt; i++) {
        int substr_len = strlen(tokens[i]);
        height_cnt++;
        if (height_cnt > height) {
            for (size_t l = 0; l < cnt; l++) {
                free(tokens[l]);
            }
            free(tokens);
            return INIT_ERR_INCORRECT_DIMENSIONS;
        }
        int width_cnt = 0;

        for (int j = 0; j < substr_len; j++) {
            // 读取字符
            char c = tokens[i][j];
            switch (c) {
                case 'E': {
                    // 读取数字
                    left = tokens[i] + j + 1;
                    j++;
                    while (tokens[i][j] >= DIGIT_START &&
                           tokens[i][j] <= DIGIT_END) {
                        j++;
                    }
                    right = tokens[i] + j - 1;

                    // 分配内存
                    char* num = strndup(left, right - left + 1);
                    int num_int = atoi(num);
                    width_cnt += num_int;
                    if (width_cnt > width) {
                        free(num);
                        for (size_t l = 0; l < cnt; l++) {
                            free(tokens[l]);
                        }
                        free(tokens);
                        return INIT_ERR_INCORRECT_DIMENSIONS;
                    }
                    for (int k = 0; k < num_int; ++k) {
                        *(cells + (height_cnt - 1) * width + width_cnt -
                          num_int + k) = PLAIN_CELL;
                    }
                    free(num);

                    // 将j移动至右指针,以便下一次读取到正确的字母，否则会直接跳到default
                    j = right - tokens[i];

                    break;
                }

                case 'S': {
                    left = tokens[i] + j + 1;
                    j++;
                    while (tokens[i][j] >= DIGIT_START &&
                           tokens[i][j] <= DIGIT_END) {
                        j++;
                    }
                    right = tokens[i] + j - 1;

                    char* num = strndup(left, right - left + 1);
                    int num_int = atoi(num);
                    width_cnt += num_int;
                    snake_num += num_int;
                    if (width_cnt > width) {
                        free(num);
                        for (size_t l = 0; l < cnt; l++) {
                            free(tokens[l]);
                        }
                        free(tokens);
                        return INIT_ERR_INCORRECT_DIMENSIONS;
                    }
                    for (int k = 0; k < num_int; ++k) {
                        *(cells + (height_cnt - 1) * width + width_cnt -
                          num_int + k) = FLAG_SNAKE;
                    }
                    free(num);

                    j = right - tokens[i];

                    break;
                }

                case 'W': {
                    left = tokens[i] + j + 1;
                    j++;
                    while (tokens[i][j] >= DIGIT_START &&
                           tokens[i][j] <= DIGIT_END) {
                        j++;
                    }
                    right = tokens[i] + j - 1;

                    char* num = strndup(left, right - left + 1);
                    int num_int = atoi(num);
                    width_cnt += num_int;
                    if (width_cnt > width) {
                        free(num);
                        for (size_t l = 0; l < cnt; l++) {
                            free(tokens[l]);
                        }
                        free(tokens);
                        return INIT_ERR_INCORRECT_DIMENSIONS;
                    }
                    for (int k = 0; k < num_int; ++k) {
                        *(cells + (height_cnt - 1) * width + width_cnt -
                          num_int + k) = FLAG_WALL;
                    }
                    free(num);

                    j = right - tokens[i];

                    break;
                }

                case 'G': {
                    left = tokens[i] + j + 1;
                    j++;
                    while (tokens[i][j] >= DIGIT_START &&
                           tokens[i][j] <= DIGIT_END) {
                        j++;
                    }
                    right = tokens[i] + j - 1;

                    char* num = strndup(left, right - left + 1);
                    int num_int = atoi(num);
                    width_cnt += num_int;
                    if (width_cnt > width) {
                        free(num);
                        for (size_t l = 0; l < cnt; l++) {
                            free(tokens[l]);
                        }
                        free(tokens);
                        return INIT_ERR_INCORRECT_DIMENSIONS;
                    }
                    for (int k = 0; k < num_int; ++k) {
                        *(cells + (height_cnt - 1) * width + width_cnt -
                          num_int + k) = FLAG_GRASS;
                    }
                    free(num);

                    j = right - tokens[i];

                    break;
                }
                default:
                    for (size_t l = 0; l < cnt; l++) {
                        free(tokens[l]);
                    }
                    free(tokens);
                    return INIT_ERR_BAD_CHAR;
            }
        }

        if (width_cnt != width) {
            for (size_t l = 0; l < cnt; l++) {
                free(tokens[l]);
            }
            free(tokens);
            return INIT_ERR_INCORRECT_DIMENSIONS;
        }
    }
    if (height_cnt != height) {
        for (size_t l = 0; l < cnt; l++) {
            free(tokens[l]);
        }
        free(tokens);
        return INIT_ERR_INCORRECT_DIMENSIONS;
    }
    if (snake_num != 1) {
        for (size_t l = 0; l < cnt; l++) {
            free(tokens[l]);
        }
        free(tokens);
        return INIT_ERR_WRONG_SNAKE_NUM;
    }

    // 释放内存
    for (size_t i = 0; i < cnt; i++) {
        free(tokens[i]);
    }
    free(tokens);

    return INIT_SUCCESS;
}