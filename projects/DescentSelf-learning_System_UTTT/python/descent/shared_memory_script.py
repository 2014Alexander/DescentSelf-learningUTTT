import numpy as np
import model_wrapper
from trainer import train_on_sample
from model_copy_manager import ModelCopyManager

sample_main_channels_np = None
sample_macro_channels_np = None
sample_values_np = None
int_vars_np = None
float_vars_np = None

# Глобальные объекты
copy_manager = None
learn_call_count = 0


def init_arrays(shm):
    global sample_main_channels_np
    global sample_macro_channels_np
    global sample_values_np
    global int_vars_np
    global float_vars_np
    global copy_manager

    sample_main_channels_np = shm.get_sample_main_channels()
    sample_macro_channels_np = shm.get_sample_macro_chennels()
    sample_values_np = shm.get_sample_values()
    int_vars_np = shm.get_int_vars()
    float_vars_np = shm.get_float_vars()

    # Загружаем основную (актуальную) модель
    model_wrapper.init_model_if_needed()

    # Создаём менеджер, передавая ему основную модель
    copy_manager = ModelCopyManager()
    copy_manager.setMainModel(model_wrapper.model)

    print("[shared_memory_script] ModelCopyManager initialized with mainModel.")


def Do():
    cmd = int_vars_np[0]
    if cmd == 200:
        # Загрузка конкретного чекпоинта в main_model (если нужно)
        load_specific_checkpoint_command()
    else:
        print(f"[shared_memory_script] Do(): Unknown command={cmd}.")


def load_specific_checkpoint_command():
    print("[shared_memory_script] Do(): Load model command")
    path_list = []
    idx = 1
    while True:
        c = int_vars_np[idx]
        if c == 0:
            break
        path_list.append(chr(c))
        idx += 1

    model_path = "".join(path_list).strip()
    print("[shared_memory_script] model path: ", model_path, flush=True)
    if not model_path:
        print("[shared_memory_script] Empty model path!", flush=True)
        return

    try:
        model_wrapper.init_model_if_needed()
        model_wrapper.model.load_weights(model_path)
        print(f"[shared_memory_script] Successfully loaded model from {model_path}", flush=True)
    except OSError:
        print(f"[shared_memory_script] OSError: failed to load weights from {model_path}", flush=True)
    except Exception as e:
        print(f"[shared_memory_script] Exception: {e}", flush=True)


def Evaluate():
    batch_size = int_vars_np[0]
    if batch_size <= 0:
        print("[shared_memory_script] Evaluate() called with batch_size <= 0.")
        return

    arr_main = sample_main_channels_np[:batch_size].astype(np.float32)
    arr_macro = sample_macro_channels_np[:batch_size].astype(np.float32)

    preds = copy_manager.evaluate_states(arr_main, arr_macro)
    sample_values_np[:batch_size] = preds


def Learn():
    global learn_call_count

    sample_size = int_vars_np[0]
    if sample_size <= 0:
        print("[shared_memory_script] Learn() called with sample_size <= 0.")
        return

    arr_main = sample_main_channels_np[:sample_size].astype(np.float32)
    arr_macro = sample_macro_channels_np[:sample_size].astype(np.float32)
    arr_values = sample_values_np[:sample_size].astype(np.float32)

    # Обучаем основную модель
    train_on_sample(arr_main, arr_macro, arr_values, sample_size)

    learn_call_count += 1

    # Пример логики: каждые 3 раза => используем main,
    # в остальные => переключаемся на эксперта
    if learn_call_count % 5 == 0:
        print("[shared_memory_script] loadExpertCheckpoint() + use_expert()")
        copy_manager.loadExpertCheckpoint()
        copy_manager.use_expert()
    else:
        print("[shared_memory_script] use_main()")
        copy_manager.use_main()
