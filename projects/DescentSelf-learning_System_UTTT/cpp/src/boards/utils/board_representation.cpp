// board_representation.cpp
#include "boards/utils/board_representation.h"
#include <bitset>
#include <iostream>
#include "bits/constants/bit_constants.h"
#include "boards/fields_functions/small_board_access.h"

std::string generate_output_string(uint64_t encoded_state) {
    uint64_t X_mask = boardGet::Xpart(encoded_state);
    uint64_t O_mask = boardGet::Opart(encoded_state);
    uint64_t boardState = boardGet::state(encoded_state);
    uint64_t free_count = boardGet::freeCount(encoded_state);
    uint64_t free_cells = boardGet::freeCells(encoded_state);

    std::string output = std::bitset<36>(free_cells).to_string() + " " +
                         std::bitset<4>(free_count).to_string() + " " +
                         std::bitset<4>(boardState).to_string() + " " +
                         std::bitset<9>(O_mask).to_string() + " " +
                         std::bitset<9>(X_mask).to_string();
    return output;
}

void display_board_info(uint64_t encoded_state) {
    std::string output = generate_output_string(encoded_state);
    std::cout << "---------------------" << std::endl;
    std::cout << (encoded_state & board::mask::OX_parts) << std::endl;
    std::cout << output << std::endl;

    // Декодирование информации из encoded_state
    uint64_t X_mask = boardGet::Xpart(encoded_state);
    uint64_t O_mask = boardGet::Opart(encoded_state);
    uint64_t boardState = boardGet::state(encoded_state);
    uint64_t free_count = boardGet::freeCount(encoded_state);
    uint64_t free_cells = boardGet::freeCells(encoded_state);

    // Отображение состояния доски
    std::cout << "Current board stateCode:" << std::endl;
    for (int i = 0; i < 9; ++i) {
        if (X_mask & (1 << i)) {
            std::cout << "X ";
        } else if (O_mask & (1 << i)) {
            std::cout << "O ";
        } else {
            std::cout << ". ";
        }
        if ((i + 1) % 3 == 0) {
            std::cout << std::endl;
        }
    }

    std::cout << "Game stateCode: ";
    switch (boardState) {
        case stateCode::X_WINS:
            std::cout << "X_Part wins" << std::endl;
            break;
        case stateCode::O_WINS:
            std::cout << "O_Part wins" << std::endl;
            break;
        case stateCode::ONGOING:
            std::cout << "Game continues" << std::endl;
            break;
        case stateCode::DRAW:
            std::cout << "Draw" << std::endl;
            break;
        default:
            std::cout << "Unknown stateCode" << std::endl;
            break;
    }

    std::cout << "Number of free moves: " << free_count << std::endl;
    std::cout << "Free positions: ";
    for (int i = 0; i < free_count; ++i) {
        uint64_t pos = (free_cells >> (i * 4)) & board::mask::FREE_CELL;
        std::cout << pos;
        if (i < free_count - 1) {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;
}
