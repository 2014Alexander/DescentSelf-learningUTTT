// Set_S.h
#pragma once

#include "structures/robin_lib/robin_set.h"
#include "big_board/BigBoard.h"

class Set_S {
private:
    size_t capacity; // Выделенная ёмкость
    tsl::robin_set<uint64_t> setHashKeys; // Robin-set для быстрого поиска уникальных указателей
    BigBoard **arrayStates; // массив состояний
public:
    size_t size; // Текущее количество элементов

    Set_S(size_t initial_capacity = 1024)
        : arrayStates(new BigBoard *[initial_capacity]),
          size(0),
          capacity(initial_capacity) {
        setHashKeys.reserve(initial_capacity); // Резервируем начальную ёмкость для robin_set
    }

    // Запрещаем конструктор копирования
    Set_S(const Set_S &) = delete;

    // Запрещаем оператор присваивания
    Set_S &operator=(const Set_S &) = delete;

    // Деструктор
    ~Set_S() {
        delete[] arrayStates;
    }

    // Добавление указателя
    void add(BigBoard *bigBoard) {
        uint64_t hashKey = bigBoard->hashKey;
        if (setHashKeys.contains(hashKey)) [[unlikely]] {
            return;
        }

        if (size == capacity) [[unlikely]] {
            grow();
        }
        setHashKeys.insert(hashKey); // Добавляем в robin_set
        arrayStates[size++] = bigBoard->clone(); // Добавляем в массив
    }

    // Очистка без освобождения памяти
    void clear() {
        size = 0; // Сбрасываем размер массива
        setHashKeys.clear(); // Очищаем robin_set
    }

    // Проверка наличия элемента
    inline bool contains(BigBoard *bigBoard) const {
        return setHashKeys.contains(bigBoard->hashKey);
    }

    inline BigBoard **getAllStates(size_t &outSize) {
        outSize = size;
        return arrayStates;
    }

private:
    // Увеличение ёмкости массива
    inline void grow() {
        size_t new_capacity = capacity * 2;
        BigBoard **new_data = new BigBoard *[new_capacity];
        std::memcpy(new_data, arrayStates, size * sizeof(BigBoard *)); // Копируем указатели побайтно
        delete[] arrayStates;
        arrayStates = new_data;
        capacity = new_capacity;

        setHashKeys.reserve(new_capacity); // Резервируем ёмкость для robin_set
    }
};
