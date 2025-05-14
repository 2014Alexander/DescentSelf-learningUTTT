#pragma once

#include <cstdint>
#include <cstring>
#include "bits/constants/bit_constants.h"
#include "big_board/big_board_get_set_do.h"
#include "boards/precalculated/precalculated_small_boards.h"

class BigBoard {
public:
    alignas(64) uint64_t boardsArray[bigBoardArrays::size];
    uint64_t &hashKey;
    uint64_t &bigState1;
    uint64_t &bigState2;

private:
    alignas(64) uint8_t movesArray[bigBoardArrays::movesSize + 1];

    inline void updateHashKey() {
        uint64_t hash = 0xcbf29ce484222325;
        constexpr uint64_t prime = 0x100000001b3;

        for (int i = 0; i < bigBoardArrays::size - 1; ++i) {
            hash ^= boardsArray[i];
            hash *= prime;
        }

        hashKey = hash;
    }

    /**
     * @brief Przenosi stan małej planszy na globalne pole stanów małych plansz.
     *
     * @param board Mała plansza, której stan jest przenoszony.
     * @param boardIndex Indeks małej planszy na dużym polu (od 0 do 8).
     */
    inline void mergeBoardStateToGlobal(uint64_t board, int boardIndex) {
        constexpr int ongoingShift = bigState1::pos::ONGOING - stateCode::bitPos::ONGOING;
        constexpr int oWinsShift = bigState1::pos::O_part - stateCode::bitPos::O_WINS;
        constexpr int xWinsShift = bigState1::pos::X_part - stateCode::bitPos::X_WINS;
        constexpr uint64_t globalFieldFirstCellMask = 0b1000000001000000001;
        uint64_t boardStateVal = boardGet::state(board);
        uint64_t combinedShifted = ((boardStateVal & stateCode::ONGOING) << ongoingShift) |
                                   ((boardStateVal & stateCode::O_WINS) << oWinsShift) |
                                   ((boardStateVal & stateCode::X_WINS) << xWinsShift);
        bigState1 &= ~(globalFieldFirstCellMask << boardIndex); // czyszczenie miejsca dla bitów stanu planszy
        bigState1 |= (combinedShifted << boardIndex);
    }

    /**
     * @brief Określa globalny stan gry na podstawie globalnego pola stanów małych plansz.
     */
    inline void updateBigState() {
        constexpr uint64_t ongoingLayerMask = bigState1::mask::ONGOING << bigState1::pos::ONGOING;

        uint64_t newBigBoardState = boardGet::state(getBoardInfo(bigState1));
        if (newBigBoardState == stateCode::ONGOING) {
            uint64_t anyIsNotOngoing = !(bigState1 & ongoingLayerMask); // jeśli żaden bit nie jest ustawiony = 1
            newBigBoardState <<= anyIsNotOngoing; // zmiana stanu na remis
        }
        bigState2 = (bigState2 & ~bigState2::mask::STATE) | newBigBoardState; // aktualizacja stanu dużej planszy
    }

    /**
     * @brief Aktualizuje informacje o małych planszach zgodnie z układem figur na nich.
     * Aktualizuje informacje o dużej planszy.
     *
     * Aktualizuje informacje każdej małej planszy z `boardsInfoArray`,
     * przenosi ich stany na globalne pole,
     * oblicza globalny stan gry.
     */
    void updateAllBoardsInfo() {
        for (int boardIndex = 0; boardIndex < 9; ++boardIndex) {
            uint64_t &board = boardsArray[boardIndex];
            board = getBoardInfo(board); // aktualizacja informacji o planszy
            mergeBoardStateToGlobal(board, boardIndex);
        }
        updateBigState();
    }

    /**
     * @brief Aktualizuje informacje o jednej małej planszy.
     *
     * Jeśli stan małej planszy uległ zmianie:
     *     przenosi jej stan na globalne pole stanów małych plansz,
     *     aktualizuje stan dużej planszy.
     */
    inline void updateBoardInfo(int boardIndex) {
        uint64_t &board = boardsArray[boardIndex];
        uint64_t oldBoardState = boardGet::state(board);
        board = getBoardInfo(board); // aktualizacja informacji o planszy
        uint64_t updatedBoardState = boardGet::state(board);
        if (updatedBoardState != oldBoardState) {
            mergeBoardStateToGlobal(board, boardIndex);
            updateBigState();
        }
    }

    /**
   * @brief Ustawia listę aktywnych małych plansz na podstawie indeksu ostatniej wykonanej komórki.
   *
   * Jeśli stan gry nie jest w trakcie (ONGOING), ustawia liczbę aktywnych plansz na 0.
   * W przeciwnym razie, sprawdza stan małej planszy wskazanej przez `lastMoveCellIndex`.
   * - Jeśli ta plansza jest w stanie ONGOING, ustawia jedną aktywną planszę na ten indeks.
   * - Jeśli nie, ustawia wszystkie plansze w stanie ONGOING jako aktywne.
   *
   * @param lastMoveCellIndex Indeks komórki ostatniego ruchu (0-8).
   */
    inline void settingValidBoards(uint64_t lastMoveCellIndex) {
        // Pobranie aktualnego stanu gry
        uint64_t gameState = BigBoardGet::state(bigState2);

        if (gameState != stateCode::ONGOING) {
            // Gra zakończona, brak aktywnych plansz
            BigBoardSet::validBoardsCount(bigState2, 0);
            BigBoardSet::validBoards(bigState2, 0);
            return;
        }

        // Pobranie maski plansz w stanie ONGOING
        uint64_t ongoing = BigBoardGet::layerOngoing(bigState1);

        // Sprawdzenie, czy plansza wskazana przez ostatni ruch jest w stanie ONGOING
        bool targetBoardOngoing = (ongoing & (rights::_1_BIT << lastMoveCellIndex)) != 0;

        if (targetBoardOngoing) {
            // Tylko jedna aktywna plansza: indeks ostatniego ruchu
            BigBoardSet::validBoardsCount(bigState2, 1);
            // Kodowanie indeksu validnej planszy w VALID_BOARDS (4 bity)
            BigBoardSet::validBoards(bigState2, lastMoveCellIndex);
        } else {
            // Znalezienie wszystkich plansz w stanie ONGOING
            int count = 0;
            uint64_t validBoards = 0;

            for (uint64_t i = 0; i < 9; ++i) {
                uint64_t bit = (ongoing >> i) & rights::_1_BIT;
                validBoards |= (i << (count << 2)) * bit;
                count += bit;
            }

            // Ustawienie liczby aktywnych plansz
            BigBoardSet::validBoardsCount(bigState2, count);

            // Ustawienie listy indeksów validnych plansz w VALID_BOARDS
            BigBoardSet::validBoards(bigState2, validBoards);
        }
    }

    /**
     * @brief Wypełnia tablicę dostępnych ruchów.
     *
     * Na pozycji 0 będzie liczba akcji (movesCount).
     */
    int fillMovesArray() {
        uint64_t validBoardsCount = BigBoardGet::validBoardsCount(bigState2);
        uint64_t validBoardsEncoded = BigBoardGet::validBoards(bigState2);

        uint8_t *bytePos = movesArray + 1; // na pozycji 0 będzie liczba akcji

        for (int i = 0; i < validBoardsCount; ++i) {
            int boardIndex = (validBoardsEncoded >> (i << 2)) & move::mask::boardIndex;

            uint64_t board = boardsArray[boardIndex];
            int freeCount = boardGet::freeCount(board);
            uint64_t freeCells = boardGet::freeCells(board);

            for (int j = 0; j < freeCount; ++j) {
                int cellIndex = (freeCells >> (j << 2)) & move::mask::cellIndex;
                uint8_t move = (boardIndex << move::pos::boardIndex) |
                               (cellIndex << move::pos::cellIndex);

                *(bytePos++) = move;
            }
        }
        int movesCount = (bytePos - (movesArray + 1));
        movesArray[0] = movesCount;
        return movesCount;
    }

    /**
     * @return Возвращает число свободных клеток на большой доске игры
     */
    inline int getAllFreeCells() {
        int allFreeCells = 0;
        for (int i = 0; i < 9; ++i) {
            uint64_t smallBoard = boardsArray[i];
            allFreeCells += zerosCountedArray[boardGet::OXpartsMeged(smallBoard)];
        }
        return allFreeCells;
    }

public:
    inline BigBoard()
        : bigState1(boardsArray[bigBoardArrays::bigState1Pos]),
          bigState2(boardsArray[bigBoardArrays::bigState2Pos]),
          hashKey(boardsArray[bigBoardArrays::hashKeyPos]) {
        stateInit();
    }

    inline BigBoard(const BigBoard &other)
        : bigState1(boardsArray[bigBoardArrays::bigState1Pos]),
          bigState2(boardsArray[bigBoardArrays::bigState2Pos]),
          hashKey(boardsArray[bigBoardArrays::hashKeyPos]) {
        std::memcpy(this->boardsArray, other.boardsArray, sizeof(this->boardsArray));
    }

    BigBoard &operator=(const BigBoard &) = delete;

    /**
     *    BigBoard original;
     *    BigBoard* clonedBoard = original.clone();
     */
    inline BigBoard *clone() const {
        return new BigBoard(*this);
    }


    /**
     * @brief Вычисляет нормализованную оценку терминального состояния игры в диапазоне [-1, +1].
     *
     * Особенности расчёта:
     * 1. Победа X возможна не раньше 17-го хода. При самой быстрой победе у нас будет:
     *    E_Xmin = 81 - 17 = 64 (свободных клеток). Для нормализации значения до +1 используется формула.
     * 2. Победа O возможна не раньше 18-го хода. При самой быстрой победе:
     *    E_Omin = 81 - 18 = 63 (свободных клеток).
     *
     * Формула оценки:
     * Если победил X: score = (E(s) + C) / (64 + C).
     * Если победил O: score = -(E(s) + C) / (64 + C).
     * Если ничья: score = 0.
     *
     * @return Нормализованная оценка терминального состояния:
     *         +1 — для самой ранней победы X,
     *         -1 — для самой ранней победы O,
     *         0 — для ничьей,
     *         промежуточные значения в диапазоне (-1, +1) зависят от количества оставшихся свободных клеток.
     */
    inline float getTerminalScore() {
        // Получаем текущее состояние игры (завершена / не завершена и кто победил).
        int st = getGameState();

        // Подсчитываем, сколько клеток осталось пустыми в терминальном состоянии.
        int freeCells = getAllFreeCells();

        // Используем константу C для оптимальной нормализации.
        constexpr float lastWinScore = 0.3f;
        constexpr float maxFreeCells = 64.0f; // Максимальное число свободных клеток для победы X.
        constexpr float C = maxFreeCells * lastWinScore / (1 - lastWinScore); //При последней победе E(s)=0, score +- 0.20; При ранней: +1 / -0.9875
        constexpr float denom = maxFreeCells + C; // Знаменатель формулы.

        // Победа X
        if (st == stateCode::X_WINS) {
            return (static_cast<float>(freeCells) + C) / denom;
        }
        // Победа O
        else if (st == stateCode::O_WINS) {
            return -(static_cast<float>(freeCells) + C) / denom;
        }
        // Ничья (DRAW)
        else {
            return 0.0f;
        }
    }

    /**
     * @brief Inicjalizuje stan dużej planszy.
     *
     * Zeruje tablicę stanów małych plansz i aktualizuje ich informacje.
     */
    void stateInit() {
        std::memset(boardsArray, 0, sizeof(boardsArray));
        updateAllBoardsInfo();
        BigBoardSet::validBoardsCount(bigState2, 9);
        BigBoardSet::validBoards(bigState2, 0x876543210);
        updateHashKey();
    }

    /**
     * @brief Przetwarza wykonanie ruchu na dużej planszy.
     *
     * Aktualizuje stan małej i dużej planszy, ustawia aktywne plansze w zależności od ostatniego ruchu
     * i zmienia aktualnego gracza.
     *
     * @param move Zakodowany ruch: 4 bity na indeks komórki, 4 bita na indeks planszy.
     */
    inline void applyMove(uint8_t move) {
        int moveBoardIndex = (move >> move::pos::boardIndex) & move::mask::boardIndex;
        int moveCellIndex = (move >> move::pos::cellIndex) & move::mask::cellIndex;

        uint64_t &targetBoard = boardsArray[moveBoardIndex];
        uint64_t cellMask = rights::_1_BIT << (moveCellIndex + BigBoardGet::player(bigState2) * board::pos::O_part);
        targetBoard |= cellMask; // apply the move

        updateBoardInfo(moveBoardIndex);
        settingValidBoards(moveCellIndex);

        BigBoardDo::invertPlayer(bigState2);
        updateHashKey();
    }

    /**
     * @return 0 jeśli X, 1 - jeśli O
     */
    inline int getCurrentPlayer() const {
        return BigBoardGet::player(bigState2);
    }

    inline int getGameState() const {
        return BigBoardGet::state(bigState2);
    }

    inline int isGameOver() {
        int gameState = getGameState();
        return gameState != stateCode::ONGOING;
    }

    /**
     * @brief Zwraca dostępne ruchy w zakodowanym formacie.
     *
     * Metoda wypełnia tablicę `movesArray` dostępnymi ruchami.
     * Zwracany wskaźnik wskazuje na tablicę, gdzie:
     * - Na pozycji 0 znajduje się liczba dostępnych ruchów.
     * - Na kolejnych pozycjach znajdują się kody ruchów.
     *
     * Dane pozostają ważne do następnej zmiany stanu planszy.
     *
     * @return Wskaźnik na tablicę z dostępnymi ruchami.
     */
    inline uint8_t *getValidMoves() {
        fillMovesArray();
        return movesArray;
    }
};
