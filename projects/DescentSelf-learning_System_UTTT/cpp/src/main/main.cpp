#include "boards/precalculated/precalculated_small_boards.h"
#include <cstdlib>

#include "selfplay/SelfPlayer.h"
#include "structures/ReplayBuffer.h"
#include "training/SampleTrainer.h"


int main() {
    srand(params::SEED);
    precalculateSmallBoardsArray();
    SharedMemory sharedMemory(params::SAMPLE_SIZE);
    ReplayBuffer replayBuffer;
    SelfPlayer selfPlayer(replayBuffer, sharedMemory);
    SampleTrainer trainer(replayBuffer, sharedMemory);
    while (true) {
        selfPlayer.runSelfPlay();
        trainer.trainSample();
    }
    return 0;
}
