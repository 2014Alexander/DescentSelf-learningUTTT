# AlphaSaltAPI.py
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
os.chdir(SCRIPT_DIR)
from UtttBoard import UtttBoard
from TrainingManager import TrainingManager
from MCTS import MCTS

# Global state
move_counter = 0
tm = TrainingManager()
search = MCTS(tm.net)
board = UtttBoard()

# Default time limit for MCTS (in seconds)
_time_limit = 10.0


def setTimeLimit(seconds):
    """
    Set the time limit for MCTS search per move.
    """
    global _time_limit
    _time_limit = seconds


def resetGame():
    """
    Reset the game: create a new board and new MCTS search,
    reset move counter.
    """
    global board, search, move_counter
    board = UtttBoard()
    # Initialize search without visit limit; time-based search will be used
    search = MCTS(tm.net)
    move_counter = 0
    print(f"Game reset. Time limit per move: {_time_limit}s.", flush=True)


def applyMove(move):
    """
    Apply a move (from any player) to the board.
    """
    if board.get_game_ended():
        print("Game has already ended.", flush=True)
        return
    if not board.is_valid_move(move):
        print(f"Invalid move: {move}", flush=True)
        return
    board.process_move(move)
    print(f"Move applied at {move}.", flush=True)


def makeMove():
    """
    Perform bot move with time-limited MCTS, apply it, and return the move.
    """
    global move_counter
    if board.get_game_ended():
        print("Game has already ended.", flush=True)
        return -1

    move_counter += 1
    # No training noise; always deterministic
    temperature = 0
    # Use time-limited search
    pi, mv = search.get_probabilities_and_best_move_with_time(
        board, temperature, _time_limit
    )
    board.process_move(mv)
    print(f"Bot moved to {mv} with probability {pi[mv]:.3f}.", flush=True)
    return mv


if __name__ == "__main__":
    # Run N self-play games (bot vs itself) using API functions
    N = 5
    setTimeLimit(3.0)  # 1 second per move
    for game_idx in range(N):
        print(f"\n=== Starting Self-Play Game {game_idx + 1}/{N} ===", flush=True)
        resetGame()
        print(board.to_string(), flush=True)
        # Bot plays both sides
        while not board.get_game_ended():
            makeMove()
            print(board.to_string(), flush=True)
            if board.get_game_ended():
                break
            makeMove()
            print(board.to_string(), flush=True)
        result = board.get_win_result()
        print(f"Self-Play Game {game_idx + 1} finished. Result: {result} (0=draw,1=first,2=second)", flush=True)
    print("All self-play games completed.", flush=True)
