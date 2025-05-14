// BigBoardsEvaluator.cpp
// ─────────────────────────────────────────── BigBoardsEvaluator.cpp
#include "selfplay/evaluate/BigBoardsEvaluator.h"

#include "bits/constants/bit_constants.h"
#include "selfplay/evaluate/SmallBoardsEvaluator.h"

int BigBoardsEvaluator::evaluate(const BigBoard& bb)
{
    /* ---------- 0. терминальный макро-результат ---------- */
    uint64_t gState = bb.getGameState();
    if (gState == stateCode::X_WINS) return  WIN_VAL_BIG;
    if (gState == stateCode::O_WINS) return -WIN_VAL_BIG;
    if (gState == stateCode::DRAW  ) return  0;

    /* ---------- 1. макро-доска как «малая» (18 бит) ------ */
    // в bigState1: биты 0-8 — X-слой, 9-17 — O-слой
    uint32_t macroCode = static_cast<uint32_t>(
        bb.bigState1 & bigState1::mask::OX_part);   // уже нужный формат

    int macroVal = SmallBoardsEvaluator::getBoardEvaluation(macroCode);

    /* ---------- 2. взвешенная сумма 9 малых досок -------- */
    int smallSum = 0;
    for (int i = 0; i < 9; ++i) {
        int ev = SmallBoardsEvaluator::getBoardEvaluation(bb.boardsArray[i]);
        smallSum += W_SMALL[i] * ev;
    }

    /* ---------- 3. итоговая оценка ----------------------- */
    return W_MACRO * macroVal + smallSum;
}
