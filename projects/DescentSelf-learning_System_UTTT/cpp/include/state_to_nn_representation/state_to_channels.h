// state_to_channels.h
#pragma once

#include <cstdint>
#include <cstring>

#include "big_board/BigBoard.h"
#include "boards/fields_functions/small_board_access.h" // для boardGet::*
#include "bits/constants/bit_constants.h"               // для rights::_9_BITS, etc.

namespace stateToChannels {

    // Количество каналов main и macro
    constexpr int main_channelsSize = 6;

    // Именованные индексы Main каналов
    constexpr int chanMain_1_CellsX = 0; // Фишки первого игрока (X)
    constexpr int chanMain_2_CellsO = 1; // Фишки второго игрока (O)
    constexpr int chanMain_3_WinBoardsX = 2; // Выигранные малые доски первым игроком (X)
    constexpr int chanMain_4_WinBoardsO = 3; // Выигранные малые доски вторым игроком (O)
    constexpr int chanMain_5_ValidBoards = 4; // Активные малые доски для следующего хода
    constexpr int chanMain_6_ValidMoveCells = 5; // Валидные позиции

    /**
     * @brief Заполняет channelsStates[channel][i] (где i=0..8) нужными 9 битами для каждой малой доски.
     *
     */
    inline void fillChannelsStatesForPlayerX(uint64_t channelsStates[6][9], const uint64_t *boardsArray, uint64_t bigState1, uint64_t bigState2) {

        // === Канал 1: X ===
        // Биты 0..8 показывают, где стоит X.
        // === Канал 2: O ===
        // Биты 9..17 показывают, где стоит O.
        for (int i = 0; i < 9; i++) {
            uint64_t board = boardsArray[i];
            channelsStates[chanMain_1_CellsX][i] = boardGet::Xpart(board);
            channelsStates[chanMain_2_CellsO][i] = boardGet::Opart(board);
        }

        // === Канал 3: X-wins (если доска выиграна X, то 0x1FF, иначе 0) ===
        uint64_t winBoardsX = BigBoardGet::layerWinsX(bigState1);
        for (int i = 0; i < 9; i++) {
            int winBitX = (winBoardsX >> i) & rights::_1_BIT;
            channelsStates[chanMain_3_WinBoardsX][i] = rights::_9_BITS * winBitX;
        }
        // === Канал 4: O-wins (если доска выиграна O, то 0x1FF, иначе 0) ===
        uint64_t winBoardsO = BigBoardGet::layerWinsO(bigState1);
        for (int i = 0; i < 9; i++) {
            int winBitO = (winBoardsO >> i) & rights::_1_BIT;
            channelsStates[chanMain_4_WinBoardsO][i] = rights::_9_BITS * winBitO;
        }
        // === Канал 5: Valid Boards (если доска доступна для хода, то 0x1FF, иначе 0) ===
        // === Канал 6: Valid (валидные позиции) ===
        std::memset(channelsStates[chanMain_5_ValidBoards], 0, 2 * sizeof(channelsStates[0]));
        uint64_t validBoardsCount = BigBoardGet::validBoardsCount(bigState2);
        uint64_t validBoardsEncoded = BigBoardGet::validBoards(bigState2);

        for (int i = 0; i < validBoardsCount; ++i) {
            int boardIndex = (validBoardsEncoded >> (i << 2)) & move::mask::boardIndex;
            channelsStates[chanMain_5_ValidBoards][boardIndex] = rights::_9_BITS;
            uint64_t board = boardsArray[boardIndex];
            uint64_t emptyCellsMask = ~(boardGet::Opart(board) | boardGet::Xpart(board)) & rights::_9_BITS;
            channelsStates[chanMain_6_ValidMoveCells][boardIndex] = emptyCellsMask;
        }

    }

    /**
    *  Заполняет channelsStates[channel][i]                         <br>
    *  В оригинальном состоянии ход делает игрок O,                 <br>
    *  нужно заменить представление доски в позицию хода игроком X  <br>
    *  Заменяем местами:                                            <br>
    *  | 1  | 9×9 | Фишки первого игрока (X)                        <br>
    *  | 2  | 9×9 | Фишки второго игрока (O)                        <br>
    *  Заменяем местами:                                            <br>
    *  | 3  | 9×9 | Выигранные малые доски первым игроком (X)       <br>
    *  | 4  | 9×9 | Выигранные малые доски вторым игроком (O)       <br>
    *  Без изменения:                                               <br>
    *  | 5  | 9×9 | Активные малые доски для следующего хода        <br>
    *  | 6  | 9×9 | Валидные позиции                                <br>
    */
    inline void fillChannelsStatesForPlayerO(uint64_t channelsStates[6][9], const uint64_t *boardsArray, uint64_t bigState1, uint64_t bigState2) {

        // === Канал 3: Фишки игрока X меняем с фишками O
        for (int i = 0; i < 9; i++) {
            uint64_t board = boardsArray[i];
            channelsStates[chanMain_1_CellsX][i] = boardGet::Opart(board); //Фишки первого игрока (X)
            channelsStates[chanMain_2_CellsO][i] = boardGet::Xpart(board); //Фишки второго игрока (O)
        }

        // === Канал 3: Выигранные малые доски игроком X будут игроком O
        uint64_t winBoardsX = BigBoardGet::layerWinsO(bigState1);
        for (int i = 0; i < 9; i++) {
            int winBitX = (winBoardsX >> i) & rights::_1_BIT;
            channelsStates[chanMain_3_WinBoardsX][i] = rights::_9_BITS * winBitX;
        }
        // === Канал 4: Выигранные малые доски игроком O будут игроком X
        uint64_t winBoardsO = BigBoardGet::layerWinsX(bigState1);
        for (int i = 0; i < 9; i++) {
            int winBitO = (winBoardsO >> i) & rights::_1_BIT;
            channelsStates[chanMain_4_WinBoardsO][i] = rights::_9_BITS * winBitO;
        }
        // === Канал 5: Valid Boards (если доска доступна для хода, то 0x1FF, иначе 0) ===
        // === Канал 6: Valid (валидные позиции) ===
        std::memset(channelsStates[chanMain_5_ValidBoards], 0, 2 * sizeof(channelsStates[0]));
        uint64_t validBoardsCount = BigBoardGet::validBoardsCount(bigState2);
        uint64_t validBoardsEncoded = BigBoardGet::validBoards(bigState2);

        for (int i = 0; i < validBoardsCount; ++i) {
            int boardIndex = (validBoardsEncoded >> (i << 2)) & move::mask::boardIndex;
            channelsStates[chanMain_5_ValidBoards][boardIndex] = rights::_9_BITS;
            uint64_t board = boardsArray[boardIndex];
            uint64_t emptyCellsMask = ~(boardGet::Opart(board) | boardGet::Xpart(board)) & rights::_9_BITS;
            channelsStates[chanMain_6_ValidMoveCells][boardIndex] = emptyCellsMask;
        }

    }


    /**
     * @brief Конвертирует состояние BigBoard в (height=9, width=9, channels=6),
     *        записывая 0/1 в буфер \p address.
     *
     * Форма записи:  address[(h * 9 + w) * 6 + c].
     *
     * @param[in]  pBigBoard  Указатель на BigBoard (9 мини-досок в boardsArray[0..8]).
     * @param[out] addressMainChannels   Массив размером 9*9*6 = 486 байт, куда записываются каналы.
     */
    inline void convert(const BigBoard *pBigBoard, uint8_t *addressMainChannels, uint8_t *addressMacroChannels) {
        const uint64_t *boardsArray = pBigBoard->boardsArray;
        uint64_t bigState1 = pBigBoard->bigState1;
        uint64_t bigState2 = pBigBoard->bigState2;

        uint64_t mainChannelsStates[main_channelsSize][9];
        uint64_t macroWinsX;
        uint64_t macroWinsO;

        if (pBigBoard->getCurrentPlayer() == cell::X) {
            fillChannelsStatesForPlayerX(mainChannelsStates, boardsArray, bigState1, bigState2);
            macroWinsX = BigBoardGet::layerWinsX(bigState1);
            macroWinsO = BigBoardGet::layerWinsO(bigState1);
        } else {
            fillChannelsStatesForPlayerO(mainChannelsStates, boardsArray, bigState1, bigState2);
            macroWinsX = BigBoardGet::layerWinsO(bigState1);
            macroWinsO = BigBoardGet::layerWinsX(bigState1);
        }

        // Индексация:
        //  boardIndex = (h/3)*3 + (w/3)
        //  cellIndex  = (h%3)*3 + (w%3)
        //  bitValue = (channelsStates[c][boardIndex] >> cellIndex) & 1
        //
        //  addressIndex = (h * 9 + w) * main_channelsSize + c

        for (int h = 0; h < 9; h++) {
            const int baseBoardIndex = (h / 3) * 3;
            const int baseCellIndex = (h % 3) * 3;

            for (int w = 0; w < 9; w++) {
                const int boardIndex = baseBoardIndex + (w / 3);
                const int cellIndex = baseCellIndex + (w % 3);

                for (int c = 0; c < main_channelsSize; c++) {
                    const uint64_t maskForBoard = mainChannelsStates[c][boardIndex];
                    const uint64_t bitValue = (maskForBoard >> cellIndex) & 1ULL;
                    *(addressMainChannels++) = static_cast<uint8_t>(bitValue);
                }
            }
        }
        for (int boardIndex = 0; boardIndex < 9; ++boardIndex) {
            *(addressMacroChannels++) = (macroWinsX >> boardIndex) & 1ULL;
            *(addressMacroChannels++) = (macroWinsO >> boardIndex) & 1ULL;
        }
    }


} // namespace stateToChannels
