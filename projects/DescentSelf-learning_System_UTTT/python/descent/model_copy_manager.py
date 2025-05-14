import os
import tensorflow as tf
from tensorflow.keras.models import clone_model
from checkpoint_manager import CheckpointManager


class ModelCopyManager:
    """
    Хранит две модели:
      1) main_model (ссылка на актуальную обучаемую модель)
      2) expert_model (клонированная архитектура, в которую будут грузиться чекпоинты)
    Переключение, какая модель участвует в Evaluate(), делается методом use_expert() или use_main().
    """

    def __init__(self):
        self.checkpoint_mgr = CheckpointManager()
        self.main_model = None  # будет указывать на model_wrapper.model
        self.expert_model = None  # отдельная копия
        self.current_idx = 0
        self.use_expert_flag = False  # по умолчанию используем main

    def setMainModel(self, main_model):
        """
        Устанавливаем ссылку на основную (актуальную) модель.
        И создаём под неё копию для expert_model, но без весов.
        """
        self.main_model = main_model
        # Создаём копию архитектуры (без весов)
        self.expert_model = clone_model(main_model)
        # Компилируем так же, как main_model
        self.expert_model.compile(
            optimizer=main_model.optimizer,
            loss=main_model.loss,
            metrics=main_model.metrics
        )
        print("[ModelCopyManager] expert_model created as a clone of main_model (no weights loaded yet).")

    def loadExpertCheckpoint(self):
        """
        Загружает веса из списка чекпоинтов в self.expert_model.
        Не трогаем self.main_model.
        """
        if not self.checkpoint_mgr.checkpoint_files:
            print("[ModelCopyManager] No checkpoints found, can't load expert.")
            return

        ckpt_list = self.checkpoint_mgr.checkpoint_files
        ckpt_path = ckpt_list[self.current_idx]

        try:
            self.expert_model.load_weights(ckpt_path)
            print(f"[ModelCopyManager] Expert model loaded from: {ckpt_path}")
        except Exception as e:
            print(f"[ModelCopyManager] Error loading: {ckpt_path}: {e}")

        self.current_idx = (self.current_idx + 1) % len(ckpt_list)

    def use_expert(self):
        """
        После вызова этого метода Evaluate() будет происходить на expert_model.
        """
        self.use_expert_flag = True

    def use_main(self):
        """
        После вызова этого метода Evaluate() будет происходить на main_model.
        """
        self.use_expert_flag = False

    @tf.function(
        input_signature=[
            tf.TensorSpec(shape=(None, 9, 9, 6), dtype=tf.float32),
            tf.TensorSpec(shape=(None, 3, 3, 2), dtype=tf.float32)
        ]
    )
    def _predict_func_expert(self, all_main_6, all_macro):
        return tf.reshape(self.expert_model([all_main_6, all_macro], training=False), [-1])

    @tf.function(
        input_signature=[
            tf.TensorSpec(shape=(None, 9, 9, 6), dtype=tf.float32),
            tf.TensorSpec(shape=(None, 3, 3, 2), dtype=tf.float32)
        ]
    )
    def _predict_func_main(self, all_main_6, all_macro):
        return tf.reshape(self.main_model([all_main_6, all_macro], training=False), [-1])

    def evaluate_states(self, arr_main_6, arr_macro):
        """
        Вызывается из Evaluate() в shared_memory_script.py
        Возвращает предсказания либо expert_model, либо main_model,
        в зависимости от use_expert_flag.
        """
        if self.use_expert_flag:
            preds = self._predict_func_expert(arr_main_6, arr_macro)
        else:
            preds = self._predict_func_main(arr_main_6, arr_macro)
        return preds.numpy()
