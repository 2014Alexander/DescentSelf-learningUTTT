import os
import config

class CheckpointManager:
    """
    Класс, который при инициализации ищет все файлы в той же директории,
    что и config.CHECKPOINT_PATH, которые заканчиваются на такое же расширение,
    за исключением самого файла config.CHECKPOINT_PATH.
    """

    def __init__(self):
        self.main_checkpoint_path = os.path.abspath(config.CHECKPOINT_PATH)
        self.dir_name, self.file_name = os.path.split(self.main_checkpoint_path)
        if not self.dir_name:
            # Если папка не указана, используем текущую директорию
            self.dir_name = os.getcwd()

        # Получаем расширение основного файла, например ".h5"
        _, self.ext = os.path.splitext(self.file_name)

        # Собираем список всех файлов в директории, которые заканчиваются на self.ext,
        # исключая сам основной чекпоинт.
        self.checkpoint_files = []
        try:
            for fname in os.listdir(self.dir_name):
                full_path = os.path.abspath(os.path.join(self.dir_name, fname))
                if os.path.isfile(full_path) and fname.endswith(self.ext):
                    if full_path != self.main_checkpoint_path:
                        self.checkpoint_files.append(full_path)
        except Exception as e:
            print(f"[CheckpointManager] Ошибка при сканировании: {e}")

        # Сортируем список (например, по алфавиту)
        self.checkpoint_files.sort()

    def get_all_checkpoints(self):
        """
        Возвращает список путей ко всем файлам-чекпоинтам, заканчивающимся на self.ext,
        за исключением config.CHECKPOINT_PATH.
        """
        return self.checkpoint_files

    def print_list(self):
        """
        Выводит в консоль список найденных чекпоинтов.
        """
        print("[CheckpointManager] Список найденных чекпоинтов:")
        for path in self.checkpoint_files:
            print("   ", path)
