# DescentSelf-learningUTTT

Self-play AI for **Ultimate Tic-Tac-Toe** based on **Descent (UBFM) + Neural Network**.  
C++ bitboard engine + Python training pipeline, expert-based self-play, unified tournament harness and **Bayes Elo** evaluation.

---

## Why this matters

- **Alternative to AlphaZero-style MCTS:** Descent + NN shows competitive strength in UTTT while using a different search paradigm.  
- **Full-stack research-to-engineering path:** from bit-level game state and fast move generation to batch NN evaluation, self-play data loops, and reproducible rankings.  
- **Honest benchmarking:** large test pool, fixed time-per-move, optimal pairing graph for Bayes Elo → robust comparisons.

---

## Key results (Evaluation)
- **Evaluation pool:** 29,527 games, **0.5s/move** hard limit for each player.  
- **Final Bayes Elo (mean ± CI):**  
  - **Descent (with experts): 764 ± 15**  
  - **SaltZero (AlphaZero-style baseline): 776 ± 7**  
  - Descent (no experts): 534 ± 11  
  - Minimax (Negamax α–β, iterative-deepening): 567 ± 14

> Trend shows the expert-based training variant catching the SaltZero baseline and clearly surpassing Minimax.

---

## Architecture (high-level)

**C++ core**
- **Bitboard engine (`BigBoard`)** with precomputed 3×3 boards for fast legality and terminal checks.  
- **Descent search** with **`BatchEvaluator`** for batched NN evaluation of non-terminal states.  
- **Self-play loop** writing experience into a shared buffer (`ReplayBuffer`), **experience replay** via `SampleTrainer`.  
- **SharedMemory (pybind11)** bridge exposes zero-copy NumPy views for channels/values and calls Python `Evaluate()` / `Learn()`.

**Python side**
- **ModelWrapper / ModelManager** (TensorFlow/Keras): checkpointing, switching between **main** and **expert** networks.  
- **Shared-memory script** initializes NumPy views over C++ buffers and runs training/inference on batches.  
- **Expert learning regime:** periodically load top past checkpoints as “experts” to stabilize and speed up training.

> Outcome: fast move generation in C++, batched GPU NN in Python, tight coupling without redundant copies.

---

## Players compared (summary)

| Player                       | Approach                     | Bayes Elo |
|-----------------------------|------------------------------|-----------|
| **Descent (with experts)**  | UBFM + NN, self-play w/ experts | **764 ± 15** |
| **SaltZero**                | AlphaZero-style (MCTS + NN)  | **776 ± 7** |
| Descent (no experts)        | UBFM + NN                    | 534 ± 11  |
| Minimax (Negamax α–β)       | Classic search               | 567 ± 14  |

Notes: same time control (**0.5s/move**), common tournament harness and Bayes Elo computation.

---

## Repository structure (as-is)

