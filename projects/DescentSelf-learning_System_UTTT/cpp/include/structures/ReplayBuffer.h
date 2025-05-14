#pragma once

#include <random>
#include <algorithm>
#include <cstddef>

#include "Map_T.h"
#include "parameters.h"
#include "Set_S.h"
#include "big_board/BigBoard.h"

struct StateValuePair {
    BigBoard *board;
    float value;
};

class ReplayBuffer {
public:
    size_t newAddedCount; //отслеживает количество добавленных элементов перед выборкой
    /**
     * @param seed Сид для инициализации генератора случайных чисел (по умолчанию случайный).
     */
    explicit ReplayBuffer()
        : maxBufferSize(params::REPLAY_BUFFER_MAX_SIZE),
          sampleArraySize(0),
          bufferFull(false),
          insertPos(0),
          newAddedCount(0),
          generator(params::SEED) // Инициализация генератора
    {
        // Выделяем память под массив ReplayItem размером maxSize
        bufferArray = new StateValuePair[maxBufferSize];
        for (size_t i = 0; i < maxBufferSize; ++i) {
            bufferArray[i].board = nullptr;
            bufferArray[i].value = 0.0f;
        }
        sampleArray = nullptr;
        indices = new size_t[maxBufferSize];
    }

    /**
     * Деструктор:
     * освобождаем все BigBoard*, которые ещё хранятся в буфере,
     * и сам массив m_buffer.
     */
    ~ReplayBuffer() {
        for (size_t i = 0; i < maxBufferSize; ++i) {
            delete bufferArray[i].board;
        }
        delete[] bufferArray;
        delete[] sampleArray;
        delete[] indices;
    }

    bool isEnoughNewData() const {
        return newAddedCount >= params::SAMPLE_SIZE;
    }

    /**
     * Добавить пару (board, value) в буфер.
     */
    void add(BigBoard *s, float v) {
        if (bufferFull) [[likely]] {
            delete bufferArray[insertPos].board;
        }
        bufferArray[insertPos].board = s;
        bufferArray[insertPos].value = v;

        ++insertPos;
        if (insertPos == maxBufferSize) {
            insertPos = 0;
            bufferFull = true;
        }
        ++newAddedCount;
    }

    /**
     *   Забирает все указатели на состояния (BigBoard * board),
     *   Для каждой board берет значение (value) из Map_T T,
     */
    void moveAll(Set_S &S, Map_T &V) {
        size_t S_size;
        BigBoard **states = S.getAllStates(S_size);
        for (int i = 0; i < S_size; ++i) {
            BigBoard *board = states[i];
            add(board, V(board));
        }
        S.clear();
        V.clear();
    }

    /**
     * Текущее число элементов в буфере.
     */
    inline size_t bufferSize() const {
        return bufferFull ? maxBufferSize : insertPos;
    }

    /**
     * Получить случайную выборку элементов из буфера.
     */
    StateValuePair *getSample(size_t &outSampleSize) {
        size_t sampleSize = params::SAMPLE_SIZE;
        sampleArrayCheckGrow(sampleSize);
        const size_t bufferActualSize = bufferSize();
        //------shuffle----------
        for (size_t i = 0; i < bufferActualSize; ++i) {
            indices[i] = i;
        }
        std::shuffle(indices, indices + bufferActualSize, generator);
        //-----------------------
        if (bufferActualSize <= sampleSize) [[unlikely]] {
            for (size_t i = 0; i < bufferActualSize; ++i) {
                sampleArray[i] = bufferArray[indices[i]];
            }
            outSampleSize = bufferActualSize;
        } else {
            for (size_t i = 0; i < sampleSize; ++i) {
                sampleArray[i] = bufferArray[indices[i]];
            }
            outSampleSize = sampleSize;
        }
        newAddedCount = 0;
        return sampleArray;
    }

private:
    size_t maxBufferSize; // Максимальное число элементов
    size_t sampleArraySize;
    StateValuePair *bufferArray;
    StateValuePair *sampleArray;
    size_t *indices; // массив индексов для шафла
    bool bufferFull; // Флаг, что буфер заполнен
    size_t insertPos; // Текущая позиция для записи
    std::mt19937 generator; // Генератор случайных чисел

    inline void sampleArrayCheckGrow(size_t sampleCheckSize) {
        if (sampleCheckSize > sampleArraySize) [[unlikely]] {
            delete[]sampleArray;
            sampleArray = new StateValuePair[sampleCheckSize];
            sampleArraySize = sampleCheckSize;
        }
    }
};
