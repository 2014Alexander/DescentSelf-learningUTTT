// src/game_board/boards/utils/big_board_renderer.cpp

#include "game_board/boards/utils/big_board_renderer.h"
#include "game_board/big_board/BigBoard.h"
#include "game_board/boards/fields_functions/small_board_access.h"
#include <iostream>
#include <iomanip> // Для форматирования вывода
#include "tournament_system/tournament/Referee.h"    // полный тип SimpleReferee
#include "tournament_system/referee/RefereeBoard.h"

void drawBigBoard(BigBoard &bigBoard, RefereeBoard &refereeBoard) {
    using namespace move; // pos::boardIndex, mask::boardIndex и т.д.

    // Индексы мини‑доски и ячейки из последнего хода
    int moveBoardIndex = (refereeBoard.lastMove >> pos::boardIndex) & mask::boardIndex;
    int moveCellIndex = (refereeBoard.lastMove >> pos::cellIndex) & mask::cellIndex;

    // Заголовок
    std::cout << "=====================\n";
    std::cout << " Move number: " << refereeBoard.moveNum << "\n";

    // Информация об игроках
    std::cout << " X Player: " << refereeBoard.ref.clientX->getName() << "\n";
    std::cout << " O Player: " << refereeBoard.ref.clientO->getName() << "\n";

    // Текущий игрок
    int currentPlayer = bigBoard.getCurrentPlayer();
    std::cout << " Current player: "
            << (currentPlayer == cell::X ? "X" : "O") << "\n";

    // Отрисовка 3×3 мини‑досок
    for (int bigRow = 0; bigRow < 3; ++bigRow) {
        for (int smallRow = 0; smallRow < 3; ++smallRow) {
            for (int bigCol = 0; bigCol < 3; ++bigCol) {
                int sbIndex = bigRow * 3 + bigCol;
                uint64_t sb = bigBoard.boardsArray[sbIndex];
                uint64_t X_mask = boardGet::Xpart(sb);
                uint64_t O_mask = boardGet::Opart(sb);

                for (int smallCol = 0; smallCol < 3; ++smallCol) {
                    int cellIndex = smallRow * 3 + smallCol;
                    char c = '.';
                    if (X_mask & (uint64_t(1) << cellIndex)) c = 'X';
                    else if (O_mask & (uint64_t(1) << cellIndex)) c = 'O';

                    // Выделяем последний ход
                    if (refereeBoard.lastMove != uint8_t(-1) &&
                        sbIndex == moveBoardIndex &&
                        cellIndex == moveCellIndex) {
                        std::cout << "[" << c << "]";
                    } else {
                        std::cout << " " << c << " ";
                    }
                }
                if (bigCol < 2) std::cout << "|";
            }
            std::cout << "\n";
        }
        if (bigRow < 2) std::cout << "----------------------------\n";
    }

    // Статус игры
    switch (bigBoard.getGameState()) {
        case stateCode::X_WINS: std::cout << "X won the game.\n";
            break;
        case stateCode::O_WINS: std::cout << "O won the game.\n";
            break;
        case stateCode::DRAW: std::cout << "The game ended in a draw.\n";
            break;
        case stateCode::ONGOING: std::cout << "The game is ongoing.\n";
            break;
        default: std::cout << "Unknown status.\n";
            break;
    }

    // Отрисовка статусов мини‑досок
    for (int sr = 0; sr < 3; ++sr) {
        for (int sc = 0; sc < 3; ++sc) {
            int idx = sr * 3 + sc;
            uint64_t sb = bigBoard.boardsArray[idx];
            uint64_t st = boardGet::state(sb);
            char ch = '.';
            if (st == stateCode::X_WINS) ch = 'X';
            else if (st == stateCode::O_WINS) ch = 'O';
            else if (st == stateCode::DRAW) ch = 'D';
            std::cout << ch << " ";
        }
        std::cout << "\n";
    }

    std::cout << "=====================\n";
    std::cout << std::flush;
}
