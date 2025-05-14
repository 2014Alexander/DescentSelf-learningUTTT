# config.py

# ------------------- ПАРАМЕТРЫ БАТЧИРОВАНИЯ И ОБУЧЕНИЯ ------------------- #
BATCH_NUM = 20
LEARNING_RATE = 0.0003

# ------------------- ПУТИ К ФАЙЛАМ ------------------- #
CHECKPOINT_PATH = "model_checkpoint.weights.h5"
LOG_FILE = "training_log.txt"

# ------------------- ЛОГИРОВАНИЕ TENSORBOARD ------------------- #
TENSORBOARD_LOG_DIR = "tensor_logs"
TENSORBOARD_LOGS = False
TENSORBOARD_WRITE_GRAPH = False
LOG_MODEL_ARCHITECTURE = False

# ------------------- ПАРАМЕТРЫ АРХИТЕКТУРЫ ------------------- #
# (A) Локальная ветвь (каналы X, O, WinX, WinO, Active, Valid)
LOCAL_FILTERS = 128
LOCAL_L2 = 1e-5
LOCAL_BLOCK_COUNT = 5
LOCAL_DROPOUT = 0.04
LOCAL_USE_SE = True
LOCAL_SE_REDUCTION = 16

# (B) Макроветвь (3×3×2)
MACRO_FILTERS = 32
MACRO_L2 = 1e-4
MACRO_RES_BLOCK_COUNT = 1
MACRO_DROPOUT = 0.05
MACRO_USE_SE = True
MACRO_SE_REDUCTION = 16

# (C) Attention (на 9 мини‑досок, после локальной свёртки)
ATTN_EMBED = 384
ATTN_HEADS = 3
ATTN_NUM_BLOCKS = 2
ATTN_MLP_RATIO = 2
ATTN_DROPOUT = 0.01
ATTN_L2 = 1e-5

# (D) Финальные Dense‑слои
DENSE_1_UNITS = 512
DENSE_2_UNITS = 256
DENSE_L2 = 5e-5

# (E) Опциональный третий Dense-слой
THIRD_DENSE_SIZE = 0
THIRD_DENSE_L2 = 1e-5
THIRD_DENSE_DROPOUT = 0.0
