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
    int new_head_index = -1;
    switch (input) {
        case INPUT_UP:
            new_head_index = head_index - (int)width;
            g_snake_lastdire = UP;
            break;
        case INPUT_DOWN:
            new_head_index = head_index + (int)width;
            g_snake_lastdire = DOWN;
            break;
        case INPUT_LEFT:
            new_head_index = head_index - 1;
            g_snake_lastdire = LEFT;
            break;
        case INPUT_RIGHT:
            new_head_index = head_index + 1;
            g_snake_lastdire = RIGHT;
            break;
        case INPUT_NONE:
            switch (g_snake_lastdire) {
                case UP:
                    new_head_index = head_index - (int)width;
                    break;
                case DOWN:
                    new_head_index = head_index + (int)width;
                    break;
                case LEFT:
                    new_head_index = head_index - 1;
                    break;
                case RIGHT:
                    new_head_index = head_index + 1;
                    break;
            }
            break;
    }

    // bool has_snake = cells_state & FLAG_SNAKE;

    // 更新蛇的位置
    // 之后如果要用head指针来写的话
    // 记得每个snake_t都要有一个head指针，指向下一个方向

    switch (cells[new_head_index]) {
        case FLAG_WALL:  // 撞墙
            g_game_over = 1;
            break;
        case FLAG_FOOD:  // 吃到食物
            g_score++;
            cells[new_head_index] = FLAG_SNAKE;
            cells[head_index] = cells[head_index] ^ FLAG_SNAKE;
            place_food(cells, width, height);
            break;
        case FLAG_GRASS:  // 草地
            cells[new_head_index] = FLAG_SNAKE | FLAG_GRASS;
            cells[head_index] = cells[head_index] ^ FLAG_SNAKE;
            break;
        case PLAIN_CELL:  // 空地
            cells[new_head_index] = FLAG_SNAKE;
            cells[head_index] = cells[head_index] ^ FLAG_SNAKE;
            break;
        case FLAG_FOOD | FLAG_GRASS:  // 在草里吃到食物
            g_score++;
            cells[new_head_index] = FLAG_SNAKE | FLAG_GRASS;
            cells[head_index] = cells[head_index] ^ FLAG_SNAKE;
            place_food(cells, width, height);
            break;
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
