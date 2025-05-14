import tensorflow as tf
from tensorflow.keras import layers, models, regularizers
from backup_checkpoints import checkpoint_backup
import config

###############################################################################
#  Если есть GPU, включаем set_memory_growth, чтобы не занимать всю видеопамять
###############################################################################
gpus = tf.config.list_physical_devices('GPU')
if gpus:
    try:
        for gpu in gpus:
            tf.config.experimental.set_memory_growth(gpu, True)
        logical_gpus = tf.config.experimental.list_logical_devices('GPU')
        print(len(gpus), "Physical GPUs,", len(logical_gpus), "Logical GPUs", flush=True)
    except RuntimeError as e:
        print(e, flush=True)

model = None  # глобальная модель (keras.Model)


###############################################################################
#  SQUEEZE‑AND‑EXCITATION (SE)
###############################################################################
def squeeze_excitation_block(x, reduction=16, block_name=""):
    c = x.shape[-1]
    gap = tf.reduce_mean(x, axis=[1, 2], keepdims=False, name=f"{block_name}_gap")  # (batch,c)
    hidden_units = c // reduction
    fc1 = layers.Dense(hidden_units, activation='relu',
                       kernel_initializer='he_normal',
                       name=f"{block_name}_se_fc1")(gap)
    fc2 = layers.Dense(c, activation='sigmoid',
                       kernel_initializer='he_normal',
                       name=f"{block_name}_se_fc2")(fc1)
    scale = tf.reshape(fc2, [-1, 1, 1, c], name=f"{block_name}_se_reshape")
    return tf.multiply(x, scale, name=f"{block_name}_se_scale")


###############################################################################
#  RESIDUAL BLOCK
###############################################################################
def res_block(x, out_filters,
              l2_coef=1e-5,
              dropout_rate=0.0,
              use_se=False,
              se_reduction=16,
              block_index=0,
              prefix="res"):
    in_c = x.shape[-1]
    sc = x
    if in_c != out_filters:
        sc = layers.Conv2D(
            out_filters, kernel_size=1, padding='same',
            kernel_regularizer=regularizers.l2(l2_coef),
            kernel_initializer='he_normal',
            name=f"{prefix}{block_index}_sc_conv"
        )(sc)
        sc = layers.BatchNormalization(name=f"{prefix}{block_index}_sc_bn")(sc)

    # Conv1
    y = layers.Conv2D(
        out_filters, 3, padding='same',
        kernel_regularizer=regularizers.l2(l2_coef),
        kernel_initializer='he_normal',
        name=f"{prefix}{block_index}_conv1"
    )(x)
    y = layers.BatchNormalization(name=f"{prefix}{block_index}_bn1")(y)
    y = layers.ReLU(name=f"{prefix}{block_index}_relu1")(y)

    # Conv2
    y = layers.Conv2D(
        out_filters, 3, padding='same',
        kernel_regularizer=regularizers.l2(l2_coef),
        kernel_initializer='he_normal',
        name=f"{prefix}{block_index}_conv2"
    )(y)
    y = layers.BatchNormalization(name=f"{prefix}{block_index}_bn2")(y)

    # SE (опционально)
    if use_se:
        y = squeeze_excitation_block(y, reduction=se_reduction,
                                     block_name=f"{prefix}{block_index}_se")

    y = layers.Add(name=f"{prefix}{block_index}_add")([sc, y])
    y = layers.ReLU(name=f"{prefix}{block_index}_relu2")(y)

    # Dropout
    if dropout_rate > 0:
        y = layers.Dropout(dropout_rate, name=f"{prefix}{block_index}_drop")(y)

    return y


###############################################################################
#  Вспомогательная функция для Attention
###############################################################################
def extract_9_tokens(x_loc):
    """
    Делим (9×9,filters) на 9 блоков (3×3) -> Flatten => (9,9*filters).
    """
    bsz = tf.shape(x_loc)[0]
    filters = x_loc.shape[-1]
    x_reshaped = tf.reshape(x_loc, [bsz, 3, 3, 3, 3, filters])
    x_reshaped = tf.transpose(x_reshaped, [0, 1, 3, 2, 4, 5])
    x_reshaped = tf.reshape(x_reshaped, [bsz, 9, 3, 3, filters])
    x_reshaped = tf.reshape(x_reshaped, [bsz, 9, 9 * filters])
    return x_reshaped


def transformer_encoder_block(x, block_index=0):
    dim = x.shape[-1]
    mha = layers.MultiHeadAttention(
        num_heads=config.ATTN_HEADS,
        key_dim=dim // config.ATTN_HEADS,
        dropout=config.ATTN_DROPOUT,
        kernel_regularizer=regularizers.l2(config.ATTN_L2),
        name=f"AttnBlock{block_index}_MHA"
    )
    ln1 = layers.LayerNormalization(name=f"AttnBlock{block_index}_LN1")
    ln2 = layers.LayerNormalization(name=f"AttnBlock{block_index}_LN2")

    mlp_hidden = int(dim * config.ATTN_MLP_RATIO)
    mlp = tf.keras.Sequential([
        layers.Dense(mlp_hidden, activation='relu',
                     kernel_regularizer=regularizers.l2(config.ATTN_L2),
                     name=f"AttnBlock{block_index}_MLP_dense1"),
        layers.Dropout(config.ATTN_DROPOUT),
        layers.Dense(dim, kernel_regularizer=regularizers.l2(config.ATTN_L2),
                     name=f"AttnBlock{block_index}_MLP_dense2"),
    ], name=f"AttnBlock{block_index}_MLP")

    # (A) Self-attn + residual
    attn_out = mha(x, x, x)
    x = x + attn_out
    x = ln1(x)

    # (B) MLP + residual
    mlp_out = mlp(x)
    x = x + mlp_out
    x = ln2(x)
    return x


###############################################################################
#  init_model_if_needed
###############################################################################
def init_model_if_needed():
    global model
    if model is not None:
        return

    # main_input: (9,9,6) => (X, O, WinX, WinO, Active, Valid)
    main_input_6 = layers.Input(shape=(9, 9, 6), name="main_input_6")
    macro_input = layers.Input(shape=(3, 3, 2), name="macro_input")

    # --- (A) Локальная ветвь (ResNet) ---
    x_loc = layers.Conv2D(
        config.LOCAL_FILTERS, 3, padding='same',
        kernel_regularizer=regularizers.l2(config.LOCAL_L2),
        kernel_initializer='he_normal',
        name="loc_init_conv"
    )(main_input_6)
    x_loc = layers.BatchNormalization(name="loc_init_bn")(x_loc)
    x_loc = layers.ReLU(name="loc_init_relu")(x_loc)

    for i in range(config.LOCAL_BLOCK_COUNT):
        x_loc = res_block(
            x_loc,
            out_filters=config.LOCAL_FILTERS,
            l2_coef=config.LOCAL_L2,
            dropout_rate=config.LOCAL_DROPOUT,
            use_se=config.LOCAL_USE_SE,
            se_reduction=config.LOCAL_SE_REDUCTION,
            block_index=i,
            prefix="loc_res"
        )

    # --- Attention ---
    tokens_9 = layers.Lambda(extract_9_tokens, name="extract_9_tokens")(x_loc)
    tokens_9 = layers.Dense(
        config.ATTN_EMBED,
        kernel_regularizer=regularizers.l2(config.ATTN_L2),
        activation='relu',
        name="loc_tokens_project"
    )(tokens_9)

    x_attn = tokens_9
    for blk_idx in range(config.ATTN_NUM_BLOCKS):
        x_attn = transformer_encoder_block(x_attn, block_index=blk_idx)

    x_loc_attn = layers.GlobalAveragePooling1D(name="loc_attn_pooling")(x_attn)

    # --- (B) Макроветвь (3×3×2) ---
    x_mac = layers.Conv2D(
        config.MACRO_FILTERS, 3, padding='same',
        kernel_regularizer=regularizers.l2(config.MACRO_L2),
        kernel_initializer='he_normal',
        name="mac_init_conv"
    )(macro_input)
    x_mac = layers.BatchNormalization(name="mac_init_bn")(x_mac)
    x_mac = layers.ReLU(name="mac_init_relu")(x_mac)

    for i in range(config.MACRO_RES_BLOCK_COUNT):
        x_mac = res_block(
            x_mac,
            out_filters=config.MACRO_FILTERS,
            l2_coef=config.MACRO_L2,
            dropout_rate=config.MACRO_DROPOUT,
            use_se=config.MACRO_USE_SE,
            se_reduction=config.MACRO_SE_REDUCTION,
            block_index=i,
            prefix="mac_res"
        )

    x_mac = layers.Flatten(name="mac_flatten")(x_mac)

    # --- (C) Слияние ---
    merged = layers.Concatenate(name="concat_all")([
        x_loc_attn,
        x_mac
    ])

    # --- (D) Dense-часть ---
    z = layers.Dense(
        config.DENSE_1_UNITS,
        kernel_regularizer=regularizers.l2(config.DENSE_L2),
        kernel_initializer='he_normal',
        name="dense_1"
    )(merged)
    z = layers.BatchNormalization(name="dense_1_bn")(z)
    z = layers.ReLU(name="dense_1_relu")(z)

    z = layers.Dense(
        config.DENSE_2_UNITS,
        kernel_regularizer=regularizers.l2(config.DENSE_L2),
        kernel_initializer='he_normal',
        name="dense_2"
    )(z)
    z = layers.BatchNormalization(name="dense_2_bn")(z)
    z = layers.ReLU(name="dense_2_relu")(z)

    if config.THIRD_DENSE_SIZE > 0:
        z = layers.Dense(
            config.THIRD_DENSE_SIZE,
            kernel_regularizer=regularizers.l2(config.THIRD_DENSE_L2),
            kernel_initializer='he_normal',
            name="dense_3"
        )(z)
        z = layers.BatchNormalization(name="dense_3_bn")(z)
        z = layers.ReLU(name="dense_3_relu")(z)
        if config.THIRD_DENSE_DROPOUT > 0.0:
            z = layers.Dropout(config.THIRD_DENSE_DROPOUT, name="dense_3_drop")(z)

    out_val = layers.Dense(
        1,
        activation='tanh',
        kernel_regularizer=regularizers.l2(config.DENSE_L2),
        name="output"
    )(z)

    # Финальная модель
    model_local = models.Model(
        inputs=[main_input_6, macro_input],
        outputs=out_val,
        name="UTTT_Local6_Macro_Attn"
    )
    tf.config.optimizer.set_jit(True)
    model_local.compile(
        optimizer=tf.keras.optimizers.Adam(learning_rate=config.LEARNING_RATE),
        loss='mse',
        metrics=['mae']
    )

    # Загрузка чекпоинта
    try:
        model_local.load_weights(config.CHECKPOINT_PATH)
        print(f"[model_wrapper] Loaded weights from {config.CHECKPOINT_PATH}", flush=True)
    except OSError:
        print("[model_wrapper] No checkpoint found, starting with fresh weights.", flush=True)

    model = model_local


###############################################################################
#  _predict_func: (None,9,9,6) + (None,3,3,2)
###############################################################################
@tf.function(input_signature=[
    tf.TensorSpec(shape=(None, 9, 9, 6), dtype=tf.float32),
    tf.TensorSpec(shape=(None, 3, 3, 2), dtype=tf.float32)
])
def _predict_func(all_main_6, all_macro):
    # all_main_6 => (batch,9,9,6) [X,O,WinX,WinO,Active,Valid]
    out_vals = model([all_main_6, all_macro], training=False)
    return tf.reshape(out_vals, [-1])


def evaluate_states(arr_main_6, arr_macro):
    preds = _predict_func(arr_main_6, arr_macro)
    return preds.numpy()


def save_checkpoint():
    if model is None:
        print("[model_wrapper] Model not initialized, nothing to save.")
        return
    model.save_weights(config.CHECKPOINT_PATH)
    print(f"[model_wrapper] Checkpoint saved to {config.CHECKPOINT_PATH}")
    checkpoint_backup()