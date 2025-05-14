import os
import shutil
import config


def checkpoint_backup():
    step = 5
    # Получаем путь к файлу чекпоинта из конфига
    checkpoint_path = config.CHECKPOINT_PATH

    # Разбиваем путь на директорию и имя файла.
    file_dir, file_name = os.path.split(checkpoint_path)
    if not file_dir:
        # Если в CHECKPOINT_PATH указан только файл – используем текущую рабочую директорию
        file_dir = os.getcwd()

    # Определяем директорию для резервных копий: <file_dir>/checkpoints
    backups_dir = os.path.join(file_dir, "checkpoints")

    # Разбиваем имя файла на базовую часть и расширение
    base, ext = os.path.splitext(file_name)

    # Если директория для резервных копий не существует, создаём её и делаем первую копию с суффиксом _0000
    if not os.path.exists(backups_dir):
        os.makedirs(backups_dir)
        backup_name = f"{base}_0000{ext}"
        backup_path = os.path.join(backups_dir, backup_name)
        shutil.copy2(checkpoint_path, backup_path)
        print(f"Backup created: {backup_path}")
        return

    # Собираем информацию о существующих резервных копиях в backups_dir
    checkpoints_map = {}  # {номер: путь_к_файлу}
    checkpoints_nums_list = []  # список номеров резервных копий

    for f in os.listdir(backups_dir):
        # Ищем файлы, имя которых начинается с базового имени и заканчивается на то же расширение
        if f.startswith(f"{base}_") and f.endswith(ext):
            # Извлекаем часть имени после последнего символа '_'
            without_ext = os.path.splitext(f)[0]
            parts = without_ext.rsplit("_", 1)
            if len(parts) == 2:
                num_str = parts[1]
                try:
                    num = int(num_str)
                    checkpoints_map[num] = os.path.join(backups_dir, f)
                    checkpoints_nums_list.append(num)
                except ValueError:
                    # Если часть после '_' не является числом, пропускаем файл
                    continue

    # Определяем максимальный номер среди существующих резервных копий
    if checkpoints_nums_list:
        max_num = max(checkpoints_nums_list)
    else:
        max_num = -1  # Если резервных копий пока нет, следующий номер будет 0

    new_num = max_num + 1

    # new_num кратен step, удаляем промежуточные резервные копии (те, номера которых не кратны step)
    if new_num % step == 0:
        for num in checkpoints_nums_list:
            if num % step != 0:
                file_to_remove = checkpoints_map[num]
                try:
                    os.remove(file_to_remove)
                except Exception as e:
                    print(f"Failed to remove backup file {file_to_remove}: {e}")

    # Форматируем новый номер в строку фиксированной длины (4 разряда, например "0003")
    new_num_str = f"{new_num:04d}"
    new_backup_name = f"{base}_{new_num_str}{ext}"
    new_backup_path = os.path.join(backups_dir, new_backup_name)

    # Копируем текущий чекпоинт в директорию резервных копий с новым именем
    shutil.copy2(checkpoint_path, new_backup_path)
    print(f"Backup created: {new_backup_path}")
