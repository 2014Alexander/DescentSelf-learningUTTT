import numpy as np
from config import LOG_FILE, LOG_MODEL_ARCHITECTURE
import tensorflow as tf


def compute_metrics_tf(preds: np.ndarray, targets: np.ndarray):
    """
    Вычисляет следующие метрики:
      - MSE (loss)
      - MAE
      - Точность знака (sign_acc)
      - R^2
      - max_err
      - Корреляция Пирсона (corr)
    Аргументы:
      preds   : (N,) np.float32 или np.float64
      targets : (N,) np.float32 или np.float64
    Возвращает словарь с ключами:
      'loss', 'mae', 'sign_acc', 'r2', 'max_err', 'corr'
    """
    # MSE
    loss_val = np.mean((preds - targets) ** 2)
    # MAE
    mae_val = np.mean(np.abs(preds - targets))
    # Точность знака
    correct_sign = np.logical_or(
        np.logical_and(preds >= 0, targets >= 0),
        np.logical_and(preds < 0, targets < 0)
    )
    sign_acc_val = correct_sign.mean()
    # R^2
    ss_res = np.sum((preds - targets) ** 2)
    mean_tg = np.mean(targets)
    ss_tot = np.sum((targets - mean_tg) ** 2)
    r2_val = 1 - ss_res / (ss_tot + 1e-8)
    # Максимальная ошибка
    maxerr_val = np.max(np.abs(preds - targets))
    # Корреляция Пирсона
    corr_matrix = np.corrcoef(preds, targets)
    corr_val = corr_matrix[0, 1]

    return {
        "loss": float(loss_val),
        "mae": float(mae_val),
        "sign_acc": float(sign_acc_val),
        "r2": float(r2_val),
        "max_err": float(maxerr_val),
        "corr": float(corr_val)
    }


def log_chunk_metrics_tf(chunk_idx, total_chunks, chunk_size, metrics):
    """
    Записывает в лог-файл метрики для одного чанка (batch) обучения.
    Формат строки:
      [trainer_tf] chunk=<i>/<total_chunks>, size=<chunk_size>, loss=<...>, mae=<...>,
      sign_acc=<...>, r2=<...>, max_err=<...>, corr=<...>
    """
    line_str = (
        f"[trainer_tf] chunk={chunk_idx}/{total_chunks}, size={chunk_size}, "
        f"loss={metrics['loss']:.4f}, mae={metrics['mae']:.4f}, "
        f"sign_acc={metrics['sign_acc']:.4f}, r2={metrics['r2']:.4f}, "
        f"max_err={metrics['max_err']:.4f}, corr={metrics['corr']:.4f}\n"
    )
    with open(LOG_FILE, "a") as f:
        f.write(line_str)


def log_final_summary_tf(sample_size, chunk_metrics_list):
    """
    Усредняет метрики по всем чанкам и записывает итоговую сводку в лог-файл.
    """
    if not chunk_metrics_list:
        return

    n = len(chunk_metrics_list)
    avg_loss = sum(m['loss'] for m in chunk_metrics_list) / n
    avg_mae = sum(m['mae'] for m in chunk_metrics_list) / n
    avg_sacc = sum(m['sign_acc'] for m in chunk_metrics_list) / n
    avg_r2 = sum(m['r2'] for m in chunk_metrics_list) / n
    avg_merr = sum(m['max_err'] for m in chunk_metrics_list) / n
    avg_corr = sum(m['corr'] for m in chunk_metrics_list) / n

    with open(LOG_FILE, "a") as f:
        f.write(f"sample_size={sample_size}, CHUNKS={n}\n")
        f.write(
            f"  avg_loss={avg_loss:.4f}, avg_mae={avg_mae:.4f}, "
            f"avg_sign_acc={avg_sacc:.4f}, avg_r2={avg_r2:.4f}, "
            f"avg_max_err={avg_merr:.4f}, avg_corr={avg_corr:.4f}\n\n"
        )


# Callback для логирования статистики слоёв через TensorBoard
class LayerStatsLogger(tf.keras.callbacks.Callback):
    def __init__(self, log_dir):
        super(LayerStatsLogger, self).__init__()
        self.log_dir = log_dir
        self.file_writer = tf.summary.create_file_writer(log_dir)

    def on_train_begin(self, logs=None):
        # Если флаг LOG_MODEL_ARCHITECTURE установлен, логируем архитектуру модели как текст
        if LOG_MODEL_ARCHITECTURE:
            model_summary = []
            self.model.summary(print_fn=lambda x: model_summary.append(x))
            summary_str = "\n".join(model_summary)
            with self.file_writer.as_default():
                tf.summary.text("Model Architecture", summary_str, step=0)
            self.file_writer.flush()

    def on_epoch_end(self, epoch, logs=None):
        # Перебираем все слои модели и логируем скалярную статистику и гистограммы для Conv2D и Dense слоёв
        for layer in self.model.layers:
            if isinstance(layer, (tf.keras.layers.Conv2D, tf.keras.layers.Dense)):
                weights = layer.get_weights()
                if weights:
                    w = weights[0]
                    mean = np.mean(w)
                    std = np.std(w)
                    min_val = np.min(w)
                    max_val = np.max(w)
                    with self.file_writer.as_default():
                        tf.summary.scalar(f"{layer.name}_w_mean", mean, step=epoch)
                        tf.summary.scalar(f"{layer.name}_w_std", std, step=epoch)
                        tf.summary.scalar(f"{layer.name}_w_min", min_val, step=epoch)
                        tf.summary.scalar(f"{layer.name}_w_max", max_val, step=epoch)
                        tf.summary.histogram(f"{layer.name}_w_hist", w, step=epoch)
        self.file_writer.flush()
