#include "game.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "linked_list.h"
#include "mbstrings.h"

/** Updates the game by a single step, and modifies the game information
 * accordingly. Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: width of the board.
 *  - height: height of the board.
 *  - snake_p: pointer to your snake struct (not used until part 3!)
 *  - input: the next input.
 *  - growing: 0 if the snake does not grow on eating, 1 if it does.
 */
void update(int* cells, size_t width, size_t height, snake_t* snake_p,
            enum input_key input, int growing) {
    // `update` should update the board, your snake's data, and global
    // variables representing game information to reflect new state. If in the
    // updated position, the snake runs into a wall or itself, it will not move
    // and global variable g_game_over will be 1. Otherwise, it will be moved
    // to the new position. If the snake eats food, the game score (`g_score`)
    // increases by 1. This function assumes that the board is surrounded by
    // walls, so it does not handle the case where a snake runs off the board.
    // “更新”应该更新棋盘、蛇的数据和代表游戏信息的全局变量，以反映新的状态。
    // 如果在更新的位置，蛇撞到墙上或自己，它将不会移动，全局变量g_game_over将为1。
    // 否则，它将被移动到新位置。如果蛇吃食物，游戏分数（“g_score”）增加1。
    // 此函数假设电路板被墙壁包围，因此它不处理蛇从电路板上跑下来的情况。

    // TODO: implement!
    // 在cells中找到蛇的头部
    int head_index = -1;
    for (int i = 0; i < (int)width * (int)height; i++) {
        if ((*(cells + i) & FLAG_SNAKE) == FLAG_SNAKE) {
            head_index = i;
            break;
        }
    }
    // 根据输入方向，计算蛇头的新位置
    // // 但现在默认往右移动一格
    // int new_head_index = head_index + 1;

    int new_head_index = -1;
    switch (input) {
        case INPUT_UP:
            new_head_index = head_index - (int)width;
            break;
        case INPUT_DOWN:
            new_head_index = head_index + (int)width;
            break;
        case INPUT_LEFT:
            new_head_index = head_index - 1;
            break;
        case INPUT_RIGHT:
            new_head_index = head_index + 1;
            break;
        case INPUT_NONE:
            new_head_index = head_index + 1;
            break;
    }

    // bool has_snake = cells_state & FLAG_SNAKE;

    // 更新蛇的位置
    // 蛇与草重叠
    if (cells[new_head_index] & FLAG_GRASS) {
        cells[new_head_index] = FLAG_SNAKE | FLAG_GRASS;
        cells[head_index] = cells[head_index] ^ FLAG_SNAKE;
    }
    // 蛇碰到墙
    else if (cells[new_head_index] & FLAG_WALL) {
        g_game_over = 1;
    }
    // 普通情况
    else if (cells[new_head_index] == PLAIN_CELL) {
        cells[new_head_index] = FLAG_SNAKE;
        cells[head_index] = cells[head_index] ^ FLAG_SNAKE;
    }
}

/** Sets a random space on the given board to food.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: the width of the board
 *  - height: the height of the board
 */
void place_food(int* cells, size_t width, size_t height) {
    /* DO NOT MODIFY THIS FUNCTION */
    unsigned food_index = generate_index(width * height);
    // check that the cell is empty or only contains grass
    if ((*(cells + food_index) == PLAIN_CELL) ||
        (*(cells + food_index) == FLAG_GRASS)) {
        *(cells + food_index) |= FLAG_FOOD;
    } else {
        place_food(cells, width, height);
    }
    /* DO NOT MODIFY THIS FUNCTION */
}

/** Prompts the user for their name and saves it in the given buffer.
 * Arguments:
 *  - `write_into`: a pointer to the buffer to be written into.
 */
void read_name(char* write_into) {
    // TODO: implement! (remove the call to strcpy once you begin your
    // implementation)
    strcpy(write_into, "placeholder");
}

/** Cleans up on game over — should free any allocated memory so that the
 * LeakSanitizer doesn't complain.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - snake_p: a pointer to your snake struct. (not needed until part 3)
 */
void teardown(int* cells, snake_t* snake_p) {
    // TODO: implement!
    free(cells);  // 1A implement
}
