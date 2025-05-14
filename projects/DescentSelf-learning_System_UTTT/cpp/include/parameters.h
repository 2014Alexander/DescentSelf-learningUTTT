// parameters.h
#pragma once
namespace params {
    constexpr int REPLAY_BUFFER_MAX_SIZE = 800000; //(µ)
    // constexpr double SAMPLING_RATE = 0.1; //(σ)
    constexpr int SAMPLE_SIZE = 40960;
    int SEED = std::random_device{}();
    constexpr float ORDINAL_ACTION_RATIO = 0.7f; //0.618f;
    constexpr float MOVE_TIME_LIMIT = 1.0f; //sec
    //----------------------
    constexpr int DESCENT_ITERATION_COUNT = 100; //сколько раз повторять descentIteration
}
