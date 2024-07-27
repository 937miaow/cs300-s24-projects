#ifndef GAME_SETUP_H
#define GAME_SETUP_H

#include <stddef.h>

#include "common.h"
#include "game.h"

/** Enum to communicate board initialization status.
 * Values include INIT_SUCCESS, INIT_ERR_INCORRECT_DIMENSIONS,
 * INIT_ERR_WRONG_SNAKE_NUM, and INIT_ERR_BAD_CHAR.
 */
enum board_init_status {
    INIT_SUCCESS,                   // no errors were thrown
    INIT_ERR_INCORRECT_DIMENSIONS,  // dimensions description was not formatted
                                    // correctly, or too many rows/columns are
                                    // specified anywhere in the string for the
                                    // given dimensions
                                    // 维度描述的格式不正确，或者在给定维度的字符串中的任何位置指定了太多的行/列
    INIT_ERR_WRONG_SNAKE_NUM,  // no snake or multiple snakes are on the board
    INIT_ERR_BAD_CHAR,  // any other part of the compressed string was formatted
                        // incorrectly
                        // 压缩字符串的任何其他部分的格式都不正确
    INIT_UNIMPLEMENTED  // only used in stencil, no need to handle this
                        // 仅用于模板 无需处理
};

enum board_init_status initialize_game(int** cells_p, size_t* width_p,
                                       size_t* height_p, snake_t* snake_p,
                                       char* board_rep);

enum board_init_status decompress_board_str(int** cells_p, size_t* width_p,
                                            size_t* height_p, snake_t* snake_p,
                                            char* compressed);
enum board_init_status initialize_default_board(int** cells_p, size_t* width_p,
                                                size_t* height_p);

#endif
