import numpy as np
import tensorflow as tf
import model_wrapper
from config import BATCH_NUM, LOG_FILE, TENSORBOARD_LOGS, TENSORBOARD_LOG_DIR, TENSORBOARD_WRITE_GRAPH
from metrics_logger_tf import (
    compute_metrics_tf,
    log_chunk_metrics_tf,
    log_final_summary_tf,
    LayerStatsLogger
)
from datetime import datetime

def augment_data(batch_main, batch_macro, batch_values):
    """
    Генерирует 8 вариантов для каждого элемента батча:
      - 4 поворота (k=0..3) на 90°,
      - для каждого поворота flip=False/True.
    Возвращает:
      aug_main:   (8*batch_size, 9,9,6)
      aug_macro:  (8*batch_size, 3,3,2)
      aug_values: (8*batch_size,)
    """
    aug_main_list = []
    aug_macro_list = []

    for flip in [False, True]:
        for k in range(4):
            # Поворот (axes=(1,2)) => rotate 2D-plane
            rotated_main = np.rot90(batch_main, k=k, axes=(1, 2))
            rotated_macro = np.rot90(batch_macro, k=k, axes=(1, 2))
            if flip:
                rotated_main = np.flip(rotated_main, axis=2)
                rotated_macro = np.flip(rotated_macro, axis=2)

            aug_main_list.append(rotated_main)
            aug_macro_list.append(rotated_macro)

    aug_main = np.concatenate(aug_main_list, axis=0)
    aug_macro = np.concatenate(aug_macro_list, axis=0)
    aug_values = np.tile(batch_values, 8)
    return aug_main, aug_macro, aug_values

def train_on_sample(arr_main, arr_macro, arr_values, sample_size):
    """
    Цикл обучения. arr_main: (sample_size,9,9,6), arr_macro: (sample_size,3,3,2).
    Делим на BATCH_NUM чанков, в каждом чанке делаем аугментацию.
    """
    # now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    # with open(LOG_FILE, "a") as f:
    #     f.write(f"\n[{now_str}] --- New training sample ---\n")

    callbacks = []
    global TENSORBOARD_WRITE_GRAPH
    if TENSORBOARD_LOGS:
        callbacks.append(tf.keras.callbacks.TensorBoard(
            log_dir=TENSORBOARD_LOG_DIR,
            write_graph=TENSORBOARD_WRITE_GRAPH
        ))
        TENSORBOARD_WRITE_GRAPH = False
        callbacks.append(LayerStatsLogger(TENSORBOARD_LOG_DIR))

    chunk_size = sample_size // BATCH_NUM
    chunk_metrics_list = []

    for i in range(BATCH_NUM):
        start_idx = i * chunk_size
        end_idx = sample_size if (i == BATCH_NUM - 1) else (start_idx + chunk_size)

        batch_main = arr_main[start_idx:end_idx]
        batch_macro = arr_macro[start_idx:end_idx]
        batch_vals = arr_values[start_idx:end_idx]

        # Аугментация x8
        aug_main, aug_macro, aug_vals = augment_data(batch_main, batch_macro, batch_vals)

        # Обучение (1 epoch)
        history = model_wrapper.model.fit(
            x=(aug_main, aug_macro),
            y=aug_vals,
            epochs=1,
            batch_size=64,
            verbose=0,
            shuffle=True,
            callbacks=callbacks
        )

    #     # Предсказания
    #     preds = model_wrapper.model.predict((aug_main, aug_macro), batch_size=1024, verbose=0)
    #     preds = preds.reshape(-1)

    #     # Вычисление метрик
    #     metrics = compute_metrics_tf(preds, aug_vals)

    #     # Запись метрик
    #     log_chunk_metrics_tf(
    #         chunk_idx=(i + 1),
    #         total_chunks=BATCH_NUM,
    #         chunk_size=len(aug_main),
    #         metrics=metrics
    #     )
    #     chunk_metrics_list.append(metrics)

    # # Итоговая сводка
    # log_final_summary_tf(sample_size, chunk_metrics_list)

    # Сохранение чекпоинта
    model_wrapper.save_checkpoint()
