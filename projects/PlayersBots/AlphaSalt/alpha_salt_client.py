# alpha_salt_client.py
import socket
import sys

from AlphaSaltAPI import setTimeLimit, resetGame, applyMove, makeMove


###############################################################################
# Утилита для строчного обмена по TCP
###############################################################################
class Communicator:
    """
    Потоковое чтение до '\n'  +  гарантированная отправка с переводом строки.
    """

    def __init__(self, sock: socket.socket):
        self.sock = sock
        self.buffer = b""  # накопительный буфер для RECEIVE_LINE

    # --------------------------------------------------------------------- #
    def receive_line(self) -> str:
        """
        Блокирующе читает одну строку (без символа '\n').
        Возвращает '' при закрытом соединении или ошибке.
        """
        while True:
            nl_pos = self.buffer.find(b"\n")
            if nl_pos != -1:
                line = self.buffer[:nl_pos].decode("utf-8", "ignore")
                self.buffer = self.buffer[nl_pos + 1:]
                return line

            try:
                chunk = self.sock.recv(1024)
            except OSError:
                return ""
            if not chunk:  # сокет закрыт
                return ""
            self.buffer += chunk

    # --------------------------------------------------------------------- #
    def send(self, msg: str):
        """
        Отправляет строку, гарантируя '\n' в конце.
        """
        if not msg.endswith("\n"):
            msg += "\n"
        self.sock.sendall(msg.encode("utf-8"))


###############################################################################
# Основной класс клиента AlphaSalt
###############################################################################
class AlphaSaltClient:
    def __init__(self, port: int):
        self.port = port
        self.sock: socket.socket | None = None
        self.comm: Communicator | None = None

    # --------------------------------------------------------------------- #
    def connect(self) -> bool:
        """Создаём TCP-сокет и коннектимся к 127.0.0.1:<port>."""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect(("127.0.0.1", self.port))
            self.comm = Communicator(self.sock)
            return True
        except OSError as e:
            print(f"[AlphaSaltClient] connect() failed: {e}", file=sys.stderr)
            return False

    # --------------------------------------------------------------------- #
    def main_loop(self):
        """
        Главный цикл:
        1) соединяемся, шлём 'OK'
        2) принимаем команды, реагируем и посылаем ответы
        """
        if not self.connect():
            return

        # handshake
        self.comm.send("OK")

        while True:
            line = self.comm.receive_line()
            if not line:
                print("[AlphaSaltClient] server closed connection.", file=sys.stderr)
                break

            command, *rest = line.split(":", 1)
            param = rest[0] if rest else ""

            ################################################################
            # обработка конкретных команд
            ################################################################
            if command == "EXIT":
                self.comm.send("OK")
                break

            elif command == "SET_TIME_PER_MOVE":
                # param — число с плавающей точкой
                try:
                    seconds = float(param)
                    setTimeLimit(seconds)
                except ValueError:
                    print(f"[AlphaSaltClient] bad seconds '{param}'", file=sys.stderr)
                self.comm.send("OK")

            elif command == "GAME_RESET":
                resetGame()
                self.comm.send("OK")

            elif command == "APPLY_MOVE":
                # От рефери: байт 0xBC
                try:
                    mv_byte = int(param)
                    flat = self.byte_to_flat(mv_byte)
                    applyMove(flat)
                except ValueError:
                    print(f"[AlphaSaltClient] bad move byte '{param}'", file=sys.stderr)
                self.comm.send("OK")

            elif command == "MAKE_MOVE":
                # Получаем плоский индекс 0..80
                flat = makeMove()
                # Конвертим в байт 0xBC
                mv_byte = self.flat_to_byte(flat)
                self.comm.send(f"MOVE:{mv_byte}")

            else:
                print(f"[AlphaSaltClient] unknown command '{command}'", file=sys.stderr)
                # по желанию: self.comm.send("ERR") или просто игнор
        # END while

        # аккуратно закрываем всё
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        self.sock.close()

    @staticmethod
    def byte_to_flat(move_byte: int) -> int:
        """
        Преобразует байт 0xBC, где B = boardIndex, C = cellIndex,
        в плоский индекс 0..80 = boardIndex*9 + cellIndex.
        """
        board = (move_byte >> 4) & 0xF
        cell = move_byte & 0xF
        return board * 9 + cell

    @staticmethod
    def flat_to_byte(flat: int) -> int:
        """
        Преобразует плоский индекс 0..80 в байт 0xBC:
        B = flat//9, C = flat%9.
        """
        board = flat // 9
        cell = flat % 9
        return (board << 4) | cell


###############################################################################
# Разбор аргументов: <порт> <timePerMove>
###############################################################################
def parse_args(argv: list[str]) -> tuple[int, float]:
    if len(argv) < 3:
        print(f"Usage: {argv[0]} <port> <timePerMove>", file=sys.stderr)
        return -1, -1.0
    try:
        port = int(argv[1])
        tpm = float(argv[2])
        return port, tpm
    except ValueError:
        print(f"Bad arguments: port must be int, timePerMove float", file=sys.stderr)
        return -1, -1.0


if __name__ == "__main__":
    port, tpm = parse_args(sys.argv)
    if port < 0:
        sys.exit(1)

    # Устанавливаем начальный лимит прямо из аргумента
    setTimeLimit(tpm)

    client = AlphaSaltClient(port)
    client.main_loop()
