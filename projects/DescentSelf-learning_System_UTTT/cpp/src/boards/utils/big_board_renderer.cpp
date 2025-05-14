// big_board_renderer.cpp
#include "boards/utils/big_board_renderer.h"
#include "big_board/BigBoard.h"
#include "boards/fields_functions/small_board_access.h"
#include <iostream>
#include <vector>
#include <iomanip> // Для форматирования вывода хэш-кода

void drawBigBoard( BigBoard &bigBoard) {
    uint64_t bigState1 = bigBoard.boardsArray[bigBoardArrays::bigState1Pos];
    uint64_t bigState2 = bigBoard.boardsArray[bigBoardArrays::bigState2Pos];
    uint8_t *mas = bigBoard.getValidMoves();
    int size = mas[0];
    for (int i = 1; i <= size; ++i) {
        int move = mas[i];
        int bIndex = (move & 0xF0) >> 4;
        int cIndex = (move & 0x0F);
        std::cout << "(" << bIndex << ", " << cIndex << ") ";
    }
    std::cout << std::endl;
    // Renderowanie głównej dużej planszy
    for (int bigRow = 0; bigRow < 3; ++bigRow) {
        for (int smallRow = 0; smallRow < 3; ++smallRow) {
            for (int bigCol = 0; bigCol < 3; ++bigCol) {
                int smallBoardIndex = bigRow * 3 + bigCol;
                uint64_t smallBoard = bigBoard.boardsArray[smallBoardIndex];
                uint64_t X_mask = boardGet::Xpart(smallBoard);
                uint64_t O_mask = boardGet::Opart(smallBoard);

                for (int smallCol = 0; smallCol < 3; ++smallCol) {
                    int cellIndex = smallRow * 3 + smallCol;
                    if (X_mask & (1 << cellIndex)) {
                        std::cout << "X";
                    } else if (O_mask & (1 << cellIndex)) {
                        std::cout << "O";
                    } else {
                        std::cout << ".";
                    }

                    if (smallCol < 2) {
                        std::cout << " ";
                    }
                }

                if (bigCol < 2) {
                    std::cout << " | ";
                }
            }
            std::cout << std::endl;
        }

        if (bigRow < 2) {
            std::cout << "---------------------" << std::endl;
        }
    }

    // Wyświetlanie statusu dużej planszy
    std::cout << "\nStatus dużej planszy:" << std::endl;
    for (int statusRow = 0; statusRow < 3; ++statusRow) {
        for (int statusCol = 0; statusCol < 3; ++statusCol) {
            int boardIndex = statusRow * 3 + statusCol;
            uint64_t smallBoard = bigBoard.boardsArray[boardIndex];
            uint64_t boardState = boardGet::state(smallBoard);

            char statusChar;
            switch (boardState) {
                case stateCode::X_WINS:
                    statusChar = 'X';
                    break;
                case stateCode::O_WINS:
                    statusChar = 'O';
                    break;
                case stateCode::DRAW:
                    statusChar = 'D';
                    break;
                default:
                    statusChar = '.'; // Gra trwa
                    break;
            }

            std::cout << statusChar << " ";
        }
        std::cout << std::endl;
    }
    // Mała plansza stanu trwających gier
    std::cout << "Ongoing Layer:" << std::endl;
    uint64_t ongoingLayer = BigBoardGet::layerOngoing(bigState1);
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int index = row * 3 + col;
            if (ongoingLayer & (1 << index)) {
                std::cout << index << " "; // Wyświetlanie indeksu trwającej gry
            } else {
                std::cout << ". "; // Gra zakończona
            }
        }
        std::cout << std::endl;
    }

    // Wyświetlanie innych danych
    int gameState = bigBoard.getGameState();
    switch (gameState) {
        case stateCode::X_WINS:
            std::cout << "X wygrał grę." << std::endl;
            break;
        case stateCode::O_WINS:
            std::cout << "O wygrał grę." << std::endl;
            break;
        case stateCode::DRAW:
            std::cout << "Gra zakończyła się remisem." << std::endl;
            break;
        case stateCode::ONGOING:
            std::cout << "Gra trwa." << std::endl;
            break;
        default:
            std::cout << "Nieznany status." << std::endl;
            break;
    }


    // Aktualny gracz
    int currentPlayer = bigBoard.getCurrentPlayer();
    std::cout << "Aktualny gracz: " << (currentPlayer == cell::X ? "X" : "O") << std::endl;

    // Ilość dostępnych plansz
    uint64_t validBoardsCount = BigBoardGet::validBoardsCount(bigState2);
    std::cout << "Ilość dostępnych plansz: " << validBoardsCount << std::endl;

    // Lista dostępnych plansz
    uint64_t validBoards = BigBoardGet::validBoards(bigState2);
    std::vector<int> validBoardsIndices;
    for (int i = 0; i < validBoardsCount; ++i) {
        int boardIndex = (validBoards >> (i * 4)) & move::mask::boardIndex;
        validBoardsIndices.push_back(boardIndex);
    }

    std::cout << "Dostępne plansze: ";
    for (size_t i = 0; i < validBoardsIndices.size(); ++i) {
        std::cout << validBoardsIndices[i];
        if (i < validBoardsIndices.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;
    std::cout << "Terminal score: " << bigBoard.getTerminalScore() << std::endl;
    // Kod hash
    std::cout << "Klucz: 0x" << std::hex << bigBoard.hashKey << std::dec << std::endl;

    //    std::cout << "----------------------\n";
    //    for (int smallBoardIndex = 0; smallBoardIndex < 9; ++smallBoardIndex) {
    //        uint64_t smallBoard = bigBoard.boardsArray[smallBoardIndex];
    //        uint64_t X_mask = boardGet::Xpart(smallBoard);
    //        std::cout << X_mask << std::endl;
    //    }
    std::cout << "=====================\n";
}
