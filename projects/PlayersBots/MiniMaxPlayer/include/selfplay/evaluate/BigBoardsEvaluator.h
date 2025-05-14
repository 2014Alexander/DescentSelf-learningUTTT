// BigBoardsEvaluator.h
// ─────────────────────────────────────────── BigBoardsEvaluator.h
#pragma once
#include "big_board/BigBoard.h"

/**
 * Быстрая эвристика большой доски.
 * • Победа/ничья обрабатываются сразу.
 * • Оценка   =  W_MACRO  * eval(macroBoard)
 *            +  Σ  w[i]  * eval(smallBoard[i])
 *
 *  macroBoard – это те же 18 бит (X-слой | O-слой) из bigState1.
 *  w[i]       – вес малой доски:       центр > углы > рёбра.
 */
class BigBoardsEvaluator {
public:
    /// >0  – позиция лучше для X,  <0 – для O
    static int evaluate(const BigBoard &bb);

private:
    /* ------------------- константы ------------------- */
    static constexpr int WIN_VAL_BIG = 100000; // макро-победа /-поражение

    static constexpr int W_MACRO = 60; // вес макро-доски

    /* веса 9-ти малых досок (центр / углы / рёбра)      *
     * выбраны так, чтобы центр≈1.2, угол=1.0, ребро≈0.8 */
    static constexpr int W_SMALL[9] = {
        12, 9, 12,
        9, 15, 9,
        12, 9, 12
    };
};
