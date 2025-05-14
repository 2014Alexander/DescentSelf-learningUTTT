// parameters.h
#pragma once
#include <bits/random.h>

namespace params {
    constexpr int SAMPLE_SIZE = 5000;
    int SEED = std::random_device{}();
    constexpr float ORDINAL_ACTION_RATIO = 1.f; //0.618f;
    constexpr float MOVE_TIME_LIMIT = 1.0f; //sec
    //----------------------
    constexpr int DESCENT_ITERATION_COUNT = 100; //сколько раз повторять descentIteration
}
