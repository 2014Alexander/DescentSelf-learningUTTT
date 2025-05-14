// SampleTrainer.h
#pragma once

#include "structures/ReplayBuffer.h"
#include "shared_memory/SharedMemory.h"
#include "state_to_nn_representation/state_to_channels.h"

/**
 * @brief Класс, который берёт семпл (примеры состояний) из ReplayBuffer
 *        и передаёт их в нейронную сеть для обучения через SharedMemory.
 */
class SampleTrainer {
public:
    /**
     * @brief Конструктор
     * @param replayBuffer - ссылка на уже существующий ReplayBuffer,
     *                       откуда будем брать сэмплы.
     * @param sharedMem    - ссылка на SharedMemory для связи с Python
     *                       (где лежит нейросеть).
     */
    SampleTrainer(ReplayBuffer &replayBuffer, SharedMemory &sharedMem)
        : replayBuffer_(replayBuffer),
          sharedMem_(sharedMem) {
    }

    /**
     * @brief Забирает из replayBuffer случайную выборку (board, value),
     *        конвертирует каждое состояние в каналы (в sampleMainChannels и sampleMacroChannels),
     *        копирует value в sampleValues (при необходимости инвертируя для O).
     *        И в конце вызывает sharedMem.Learn().
     */
    void trainSample() const {
        // 1) Получаем семпл (вплоть до params::SAMPLE_SIZE)
        size_t sampleSize;
        auto sampleData = replayBuffer_.getSample(sampleSize);

        // 2) Заполняем sharedMem_.intVars[0] числом образцов
        sharedMem_.intVars[0] = static_cast<int>(sampleSize);

        // 3) Для каждого i-го элемента семпла
        //    - конвертируем board -> 9×9×6 каналы + 3×3×2 macro
        //    - пишем value (при player == O -> val = -val)
        uint8_t *dstMain = sharedMem_.sampleMainChannels;
        uint8_t *dstMacro = sharedMem_.sampleMacroChannels;
        float *dstVals = sharedMem_.sampleValues;

        for (size_t i = 0; i < sampleSize; i++) {
            const BigBoard *board = sampleData[i].board;
            float val = sampleData[i].value;

            // Конвертируем board в каналы
            stateToChannels::convert(board, dstMain, dstMacro);

            // Если ход у O, переворачиваем знак оценки
            if (board->getCurrentPlayer() == cell::O) {
                val = -val;
            }
            dstVals[i] = val;

            // Сдвигаемся на следующий блок
            dstMain += (9 * 9 * 6);
            dstMacro += (3 * 3 * 2);
        }

        // 4) Вызываем Python-метод Learn(), чтобы обучить нейросеть
        sharedMem_.Learn();
    }

private:
    ReplayBuffer &replayBuffer_;
    SharedMemory &sharedMem_;
};
