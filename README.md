# DescentSelf-learningUTTT

Self-play AI for **Ultimate Tic-Tac-Toe** based on **Descent (UBFM) + Neural Network**.  
C++ bitboard engine + Python training pipeline, expert-based self-play, unified tournament harness, and **Bayes Elo** evaluation.

---

## Why this matters

**1) A competitive alternative to AlphaZero-style MCTS for UTTT.**  
We apply **Descent (UBFM + NN)** to Ultimate Tic-Tac-Toe — to our knowledge, its first rigorous use on this game — and show it can **match an AlphaZero-style baseline (SaltZero)** under the same time control. This is valuable for teams exploring search-and-learn hybrids beyond MCTS.

**2) End-to-end, production-like engineering—not just a model.**  
This repo includes a **high-performance C++ bitboard engine** (compact state in 12 integers, precomputed small boards), a **Python/TensorFlow trainer** embedded into the C++ process via **pybind11/shared memory**, and **batched GPU evaluation**. The result is **low-latency C++ control with ML throughput from Python**—a pattern directly transferable to real systems.

**3) Reproducible and honest benchmarking at scale.**  
We evaluate **≈29.5k games at 0.5s/move** across many checkpoints and players and compute **Bayes Elo** from a unified tournament harness. Crucially, we **don’t** rely on round-robin; instead, we auto-build a **pairing graph with maximal algebraic connectivity (Fiedler λ₂)** so that even with partial pairings you still get robust relative strength estimates. This reduces bias and makes results credible.

**4) “Expert-based” self-play that stabilizes and accelerates learning.**  
Beyond vanilla self-play, we periodically load **expert checkpoints** and control their contribution during training/inference. In practice this **speeds up early learning to expert level** and **smooths training**, producing a clear, monotonic improvement curve and stronger final agents. This regime also enables controlled ablation studies (fixed expert, same starting checkpoint).

**5) A generalizable framework for two-player perfect-information games.**  
The code cleanly separates **engine, training, and evaluation** with well-defined interfaces (clients over TCP/protocol, config-driven tournaments). That makes it straightforward to **swap the game**, **change network architectures**, or **plug in other search agents** (e.g., Minimax, MCTS) and evaluate them under identical conditions.

**6) Signals aligned with what hiring teams value.**  
This project demonstrates: (a) **systems thinking** (C++ perf + Python ML in one process), (b) **experiment discipline** (checkpoints, metrics, ablations, Elo with statistical grounding), and (c) **ability to ship** a **reproducible pipeline** rather than a one-off demo—skills that map directly to production ML/AI roles.


---

## Key results (evaluation)

- **Evaluation pool:** 29,527 games, **0.5s/move** hard limit per player.  
- **Final Bayes Elo (mean ± CI):**  
  - **Descent (with experts): 764 ± 15**  
  - **SaltZero (AlphaZero-style baseline): 776 ± 7**  
  - Descent (no experts): 534 ± 11  
  - Minimax (Negamax α–β, iterative-deepening): 567 ± 14

> The expert-based training variant approaches the SaltZero baseline and clearly surpasses Minimax.

---

## Learning curve (Elo vs. Epoch)

<p align="center">
  <img src="docs/elo_vs_epoch.png" alt="Elo rating vs. training epoch (Descent + NN on UTTT)" width="80%">
</p>

**Notes**
- The red curve shows the model’s Elo across training epochs (Bayes Elo, 0.5s/move, unified tournament system).
- Horizontal lines: **SaltZero ≈ 780** (blue) and **Minimax ≈ 565** (orange) as reference baselines.
- Rapid early gains followed by steady improvement; the model approaches SaltZero’s strength in later epochs.

---

## Architecture (high-level)

**C++ core**
- **Bitboard engine (`BigBoard`)** with precomputed 3×3 boards for fast legality and terminal checks.  
- **Descent search** with **`BatchEvaluator`** for batched NN evaluation of non-terminal states.  
- **Self-play loop** writing experience to a shared buffer (`ReplayBuffer`), **experience replay** via `SampleTrainer`.  
- **SharedMemory / pybind11 bridge** exposing zero-copy NumPy views for channels/values and calling Python `Evaluate()` / `Learn()`.

**Python side**
- **ModelWrapper / ModelManager** (TensorFlow/Keras): checkpointing, switching between **main** and **expert** networks.  
- **Shared-memory script** to map C++ buffers as NumPy arrays and run batched training/inference (GPU).  
- **Expert regime:** periodically load top past checkpoints as “experts” to stabilize and accelerate learning.

> Outcome: fast move generation in C++, batched GPU NN in Python, tight coupling without redundant copies.

---

## Players compared (summary)

| Player                       | Approach                         | Bayes Elo |
|-----------------------------|----------------------------------|-----------|
| **Descent (with experts)**  | UBFM + NN, self-play w/ experts  | **764 ± 15** |
| **SaltZero**                | AlphaZero-style (MCTS + NN)      | **776 ± 7** |
| Descent (no experts)        | UBFM + NN                        | 534 ± 11  |
| Minimax (Negamax α–β)       | Classic search                   | 567 ± 14  |

Notes: same time control (**0.5s/move**), common tournament harness, and Bayes Elo computation.

---

## Repository structure

```
.
├─ docs/                                  # plots & diagrams (e.g., elo_vs_epoch.png)
├─ projects/
│  ├─ DescentSelf-learning_System_UTTT/   # core self-learning system (engine + training)
│  │  ├─ cpp/
│  │  │  ├─ include/
│  │  │  │  ├─ big_board/                 # BigBoard.h, bitboard getters/setters
│  │  │  │  ├─ bits/constants/            # bit-level constants
│  │  │  │  ├─ boards/
│  │  │  │  │  ├─ fields_functions/       # small-board access helpers
│  │  │  │  │  ├─ precalculated/          # precomputed small boards
│  │  │  │  │  └─ utils/                  # renderers/representation helpers
│  │  │  │  ├─ selfplay/                  # Descent, SelfPlayer, BatchEvaluator (+ utils/)
│  │  │  │  ├─ shared_memory/             # SharedMemory.h, python_config.h
│  │  │  │  ├─ state_to_nn_representation/# C++ → NN channels mapping
│  │  │  │  ├─ structures/                # ReplayBuffer, robin_map/set
│  │  │  │  └─ training/                  # SampleTrainer.h
│  │  │  └─ src/
│  │  │     ├─ boards/…                   # impl of renderers/precalculated
│  │  │     ├─ main/main.cpp              # demo/entry point
│  │  │     └─ shared_memory/SharedMemory.cpp
│  │  └─ python/
│  │     └─ descent/
│  │        ├─ trainer.py                 # self-play training loop
│  │        ├─ model_wrapper.py           # TF/Keras model wrapper
│  │        ├─ checkpoint_manager.py      # saving/rotation of checkpoints
│  │        ├─ model_copy_manager.py      # copy/swap models (main ↔ expert)
│  │        ├─ metrics_logger_tf.py       # training metrics
│  │        ├─ shared_memory_script.py    # NumPy views ↔ C++ buffers bridge
│  │        ├─ config.py                  # training/runtime config
│  │        └─ backup_checkpoints.py
│  │
│  ├─ GeneralTestingSystem/               # tournament & evaluation harness
│  │  ├─ include/
│  │  │  ├─ game_board/                   # minimal board API (for testers)
│  │  │  └─ tournament_system/
│  │  │     ├─ client/                    # ClientWrapper
│  │  │     ├─ communication/             # Communicator, ProcessLauncher, SocketListener
│  │  │     ├─ referee/                   # ResultsLogger.h
│  │  │     ├─ setup/                     # players_programs config (pairs/params/utils)
│  │  │     └─ tournament/                # Referee, TournamentRunner (headers)
│  │  └─ src/tournament_system/
│  │     ├─ referee/ResultsLogger.cpp
│  │     ├─ setup/players_programs/…      # parameters, players_pairs
│  │     └─ tournament/                   # Referee.cpp, TournamentRunner.cpp
│  │
│  ├─ PlayersBots/                        # concrete players used in tests
│  │  ├─ AlphaSalt/                       # Python client for SaltZero baseline
│  │  │  ├─ AlphaSaltAPI.py
│  │  │  └─ alpha_salt_client.py
│  │  ├─ DescentPlayer/                   # C++ Descent-based agent (standalone)
│  │  │  ├─ include/…                     # mirrors engine headers (BigBoard, selfplay, etc.)
│  │  │  ├─ src/
│  │  │  │  ├─ main/main.cpp              # agent entry point
│  │  │  │  └─ shared_memory/SharedMemory.cpp
│  │  │  └─ CMakeLists.txt
│  │  └─ MiniMaxPlayer/                   # C++ Negamax α–β baseline
│  │     ├─ include/
│  │     │  ├─ client/communication/      # Communicator, TcpConnector
│  │     │  └─ selfplay/evaluate/         # Small/Big board evaluators
│  │     └─ src/
│  │        ├─ main/main.cpp
│  │        ├─ boards/precalculated/…
│  │        └─ selfplay/evaluate/…
│  │
│  └─ others/
│     └─ MaximumConnectivityMatchPlanner.py # pairing graph planner (max Fiedler λ₂)
│
├─ LICENSE
└─ README.md
```

### What lives where (quick map)

- **DescentSelf-learning_System_UTTT/** — core **engine + training**: C++ core (bitboard, Descent/BatchEvaluator, SharedMemory) and Python trainer (TF/Keras, checkpoints, metrics).  
- **GeneralTestingSystem/** — tournament harness and evaluation infrastructure: clients, process/socket communication, referee, tournament runner, results logging.  
- **PlayersBots/** — concrete agents used for comparisons: `DescentPlayer` (C++ agent), `MiniMaxPlayer` (Negamax α–β), `AlphaSalt` (Python client for SaltZero).  
- **others/** — match planner script for pairing graphs with maximum algebraic connectivity (for reliable Bayes Elo evaluation).  
- **docs/** — artifacts: plots and result tables (e.g., `elo_vs_epoch.png`).  


## Reproducing thesis results (outline)

1. **Self-play & training schedule.** Alternate epochs: self-play data collection → training on sampled batches (experience replay). Maintain **main** checkpoints and a small pool of **experts** (top past checkpoints).
2. **Expert-based training.** Early epochs use experts exclusively for state evaluation; later, interleave experts with the current main net on a fixed cadence to stabilize learning.
3. **Tournament set.** Build N checkpointed players (e.g., 100). Generate a **d-regular pairing graph** that **maximizes algebraic connectivity (Fiedler λ₂)** to ensure indirect comparability when not all pairs meet directly.
4. **Play & collect.** Fixed time-per-move (0.5s), fixed encounters per pair (even count to balance first-move advantage). Export results to **PGN**.
5. **Rating.** Compute **Bayes Elo** from the PGN set; plot Elo vs. epoch for learning curves and report the final table.

---

## License & acknowledgments
- **License:** GPL-3.0  
- **References:** Descent/UBFM, AlphaZero line of work, Bayes Elo methodology, algebraic connectivity (Fiedler) for pairing graphs.

---

## Krótko po polsku (PL)
Projekt systemu samouczącego się dla **Ultimate Tic-Tac-Toe**: silnik C++ (bitboard), sieć w Pythonie (TensorFlow/Keras), uczenie z **ekspertami**. W testach (**29 527 gier**, **0.5s/ruch**) system **Descent + NN** osiąga **764 ± 15 Bayes Elo**, blisko **SaltZero 776 ± 7** i powyżej **Minimax 567 ± 14**. Ranking oparty na **Bayes Elo** i doborze par o maksymalnej łączności algebraicznej (Fiedler λ₂).

## Short summary (EN)
Self-play AI for **Ultimate Tic-Tac-Toe** using **Descent (UBFM) + NN**. C++ bitboard engine, Python training, expert-based regime. In **29,527** games at **0.5s/move**, **Descent+experts** scores **764 ± 15 Bayes Elo**, near **SaltZero 776 ± 7**, above **Minimax 567 ± 14**. Robust evaluation via **Bayes Elo** with an algebraically connected pairing graph (Fiedler λ₂).
