// Counter.h
#pragma once
class Counter {
private:
    int count;
    int counterLimit;

public:
    // Конструктор
    Counter(int limit) : count(0), counterLimit(limit) {
    }

    // Метод проверки превышения лимита
    bool isCountExceeded() {
        if (count++ < counterLimit) {
            return false;
        }
        count = 0;
        return true;
    }
};
