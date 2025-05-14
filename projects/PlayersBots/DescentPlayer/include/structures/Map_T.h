// Map_T.h
#pragma once

#include "structures/robin_lib/robin_map.h" // Заголовок из библиотеки tsl::robin_map
#include "big_board/BigBoard.h"


class Map_T {
public:
    explicit Map_T(size_t reserve_size_states = 1024, size_t reserve_size_state_actions = 1024) {
        state_valueMap.reserve(reserve_size_states);
        state_action_valueMap.reserve(reserve_size_state_actions);
    }

    inline float &operator()(BigBoard *s) {
        return state_valueMap[s->hashKey]; // Позволяет присваивать и получать
    }

    inline float &operator()(BigBoard *s, uint8_t a) {
        uint64_t hashKey = combineStateHashWithMove(s->hashKey, a);
        return state_action_valueMap[hashKey]; // Позволяет присваивать и получать
    }

    void clear() {
        state_valueMap.clear(); //не сокращает capacity, а только удаляет элементы. То есть исходный bucket_count сохранится.
        state_action_valueMap.clear();
    }

    inline size_t sizeStates() const {
        return state_valueMap.size();
    }

    inline size_t sizeStateActions() const {
        return state_action_valueMap.size();
    }

    inline bool contains(const BigBoard *s) const {
        return state_valueMap.contains(s->hashKey);
    }

    inline bool containsStateAction(const BigBoard *s, uint8_t a) const {
        uint64_t hashKey = combineStateHashWithMove(s->hashKey, a);
        return state_action_valueMap.contains(hashKey);
    }

private:
    tsl::robin_map<uint64_t, float> state_valueMap;
    tsl::robin_map<uint64_t, float> state_action_valueMap;

    inline uint64_t combineStateHashWithMove(uint64_t stateHash, uint8_t move) const {
        stateHash ^= static_cast<uint64_t>(move) * 0x87c37b91114253d5ULL;
        stateHash = (stateHash << 31) | (stateHash >> 33);
        stateHash *= 0x4cf5ad432745937fULL;
        return stateHash;
    }
};
