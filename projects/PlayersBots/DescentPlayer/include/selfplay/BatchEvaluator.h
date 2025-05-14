// BatchEvaluator.h
#pragma once

#include <cstdint>
#include "big_board/BigBoard.h"       // BigBoard
#include "shared_memory/SharedMemory.h"
#include "state_to_nn_representation/state_to_channels.h"
#include "structures/Map_T.h"         // Map_T

/**
 * @brief Класс для пакетной оценки нетерминальных состояний одним вызовом нейросети.
 */
class BatchEvaluator {
public:
    static constexpr int MAX_SIZE = 81; ///< Максимальное число состояний в пакете

private:
    SharedMemory &sharedMem; ///< Ссылка на общий буфер (sampleMainChannels, sampleValues, и т.д.)
    Map_T &V; ///< Ссылка на карту v(s) и v'(s,a)
    int movesCount; ///< Текущее число записей в батче

    /**
     * Массив ходов (move), по которым будем потом
     * присваивать значения v'(parentState, move).
     * Индекс i соответствует i-му добавленному состоянию.
     */
    uint8_t moves[MAX_SIZE];

public:
    /**
     * @brief Конструктор
     * @param shm   - ссылка на уже созданный SharedMemory (глобальный на всё приложение)
     * @param mapV  - ссылка на Map_T (хранит v(s), v'(s,a))
     */
    BatchEvaluator(SharedMemory &shm, Map_T &mapV)
        : sharedMem(shm)
          , V(mapV)
          , movesCount(0) {
    }

    /**
     * @brief Инициализируем батч перед обработкой нового родительского состояния.
     *        Обнуляем счётчик, чтобы начать заново собирать child'ы.
     *        (Можно вызывать перед циклом, где раскрываем s.)
     */
    inline void beginBatch() {
        movesCount = 0;
    }

    /**
     * @brief Добавляем нетерминальное состояние (child) в общий буфер.
     */
    inline void addNonTerminalState(const BigBoard &childBoard, uint8_t move) {
        // (1) Найдём адрес, куда писать каналы для i-го child
        const int i = movesCount;
        uint8_t *dstMain = sharedMem.sampleMainChannels + (std::size_t) i * (9 * 9 * 6);
        uint8_t *dstMacro = sharedMem.sampleMacroChannels + (std::size_t) i * (3 * 3 * 2);

        // (2) Конвертируем состояние BigBoard -> каналы
        stateToChannels::convert(&childBoard, dstMain, dstMacro);

        // (3) Запоминаем ход
        moves[i] = move;

        // (4) Увеличиваем счётчик
        movesCount++;
    }

    /**
     * @brief Запустить нейросеть на всём батче добавленных состояний
     *        и записать результаты в V(parentState, move).
     */
    inline void evaluateAllNonTerminalChildStates(BigBoard *parentState) {
        if (movesCount == 0) [[unlikely]] {
            return; // Нечего оценивать
        }

        // (1) Сообщаем Python, сколько реально child-состояний
        sharedMem.intVars[0] = movesCount;

        // (2) Запуск Evaluate() (один вызов)
        sharedMem.Evaluate();

        int parentPl = parentState->getCurrentPlayer();

        if (parentPl == cell::O) {
            // Если реальный ход X, то каналы без свапа,
            // сеть вернула + = "хорошо X", => можно напрямую писать
            for (int i = 0; i < movesCount; i++) {
                float netVal = sharedMem.sampleValues[i];
                // v'(s, a) = netVal
                V(parentState, moves[i]) = netVal;
            }
        } else {
            // Если реальный ход O, то каналы со свапом,
            // сеть вернула "+ = хорошо для 'X'(свапнутого)", но это реально "хорошо для O".
            // => нужно инвертировать знак, чтобы оставалось + = X в глобальных координатах
            for (int i = 0; i < movesCount; i++) {
                float netVal = sharedMem.sampleValues[i];
                V(parentState, moves[i]) = -netVal;
            }
        }

        // (4) Сбрасываем movesCount
        movesCount = 0;
    }
};
