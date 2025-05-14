import os
import torch
import networkx as nx
import matplotlib.pyplot as plt


##############################################################################
#                 НАСТРОЙКА ЧИСЛА ПОТОКОВ (CPU)
##############################################################################

def configure_cpu_threads(num_threads: int = None):
    """
    Если num_threads передан, пытается установить число потоков,
    которые PyTorch/BLAS будут использовать для параллельного выполнения.
    Если num_threads не передан, можно задать через переменные окружения
    или оставить поведение по умолчанию.
    """
    if num_threads is not None and num_threads > 0:
        torch.set_num_threads(num_threads)
        # Можно дополнительно установить количество потоков для интероперабельности:
        # torch.set_num_interop_threads(num_threads)
        print(f"[INFO] Установлено torch.set_num_threads({num_threads})")
    else:
        print("[INFO] Число потоков не задано, используется настройка по умолчанию.")


##############################################################################
#                 ФУНКЦИИ ДЛЯ ПОСТРОЕНИЯ РАСПИСАНИЯ / БИТСЕТА
##############################################################################

def full_round_robin_schedule(n):
    """
    Генерирует полный круговой (round-robin) граф расписания для n игроков.
    Если n нечетно, добавляется фиктивный игрок (-1), который затем игнорируется.
    Возвращает список раундов, где каждый раунд — это список пар (u, v).
    """
    rounds = []
    players = list(range(n))
    if n % 2 == 1:
        players.append(-1)
        n += 1
    num_rounds = n - 1
    for _ in range(num_rounds):
        round_pairs = []
        for i in range(n // 2):
            a = players[i]
            b = players[n - 1 - i]
            if a != -1 and b != -1:
                round_pairs.append((a, b))
        rounds.append(round_pairs)
        # Круговой метод: фиксируем первого игрока, а остальных "вращаем"
        players = [players[0]] + [players[-1]] + players[1:-1]
    return rounds


def bit_index(n, u, v):
    """
    Функция для определения индекса бита в битсете для ребра (u, v), предполагая u < v.
    Нумерует верхнетреугольную часть матрицы (без диагонали).
    """
    if u > v:
        u, v = v, u
    return (u * (2 * n - u - 1)) // 2 + (v - u - 1)


def set_edge_in_bitset(bitset, n, u, v):
    """
    Устанавливает бит, соответствующий ребру (u, v), в bitset.
    Возвращает новый bitset.
    """
    idx = bit_index(n, u, v)
    return bitset | (1 << idx)


##############################################################################
#          ФУНКЦИИ ДЛЯ БАТЧИНГА ВЫЧИСЛЕНИЯ ЗНАЧЕНИЯ ФЕДЛЕРА (CPU)
##############################################################################

def build_batch_adjacency(bitsets, n):
    """
    Создаёт батч матриц смежности размера (B, n, n) на CPU (float32).
    """
    B = len(bitsets)
    adj_batch = torch.zeros((B, n, n), dtype=torch.float32)
    for i, bitset in enumerate(bitsets):
        idx = 0
        for r in range(n):
            for c in range(r + 1, n):
                if bitset & (1 << idx):
                    adj_batch[i, r, c] = 1.0
                    adj_batch[i, c, r] = 1.0
                idx += 1
    return adj_batch


def batch_laplacian_fiedler(bitsets, n, edge_counts):
    """
    Вычисляет Fiedler-value (вторая наименьшая собственная величина лапласиана)
    для набора графов (bitsets). Возвращает список float длины B.

    Если edge_count[i] < n-1, то граф заведомо не связен => Fiedler = 0.
    Иначе считаем лапласиан и делаем eigvalsh (batched).
    """
    B = len(bitsets)
    # Формируем флаг, кто заведомо не связен
    not_connected_mask = [(edge_counts[i] < n - 1) for i in range(B)]
    need_compute_indices = [i for i, nc in enumerate(not_connected_mask) if not nc]

    # Подготавливаем результат (B,)
    fiedlers = [0.0] * B
    if len(need_compute_indices) == 0:
        return fiedlers

    compute_bitsets = [bitsets[i] for i in need_compute_indices]

    # 1) Создаём батч матриц смежности
    adj_batch = build_batch_adjacency(compute_bitsets, n)

    # 2) Лапласиан: L = D - A
    deg_batch = torch.sum(adj_batch, dim=2)  # (B_eff, n)
    L_batch = -adj_batch.clone()  # (B_eff, n, n)
    for i in range(L_batch.shape[0]):
        diag_idx = torch.arange(n)
        L_batch[i, diag_idx, diag_idx] = deg_batch[i]

    # 3) Batched eigvalsh: получаем (B_eff, n) отсортированных собственных значений
    eigvals_all = torch.linalg.eigvalsh(L_batch)

    # 4) Вторая по величине собственная величина (индекс 1)
    for local_i, global_i in enumerate(need_compute_indices):
        fiedlers[global_i] = eigvals_all[local_i, 1].item()

    return fiedlers


##############################################################################
#      ИТЕРАТИВНОЕ DP (с ограничением числа состояний на каждом уровне)
##############################################################################

def dp_select_rounds_iterative_batched(rounds, d, n, beam_size=200):
    """
    Итеративное DP + Batched вычисление алгебраической связности (Fiedler value) на CPU.
    beam_size ограничивает число состояний, чтобы не взрывать память.

    Возвращает (best_selected_rounds, best_connectivity).
    """
    num_rounds = len(rounds)
    # dp[i][k] = список состояний (bitset, edge_count, connectivity, path)
    dp = [[[] for _ in range(d + 1)] for _ in range(num_rounds + 1)]
    dp[0][0] = [(0, 0, 0.0, [])]  # стартовое состояние

    def beam_filter(states_list):
        # Сортируем по connectivity убыванию и оставляем beam_size лучших
        states_list.sort(key=lambda x: x[2], reverse=True)
        if len(states_list) > beam_size:
            states_list[:] = states_list[:beam_size]

    for i in range(num_rounds):
        round_pairs = rounds[i]
        for k_cur in range(d + 1):
            if not dp[i][k_cur]:
                continue

            # 1) Переход "skip"
            dp[i + 1][k_cur].extend(dp[i][k_cur])
            beam_filter(dp[i + 1][k_cur])

            # 2) Переход "take"
            if k_cur < d:
                current_states = dp[i][k_cur]
                new_states_dict = {}
                for (bs, ec, conn_val, path) in current_states:
                    new_bs = bs
                    new_ec = ec
                    for (u, v) in round_pairs:
                        nb = set_edge_in_bitset(new_bs, n, u, v)
                        if nb != new_bs:
                            new_bs = nb
                            new_ec += 1
                    new_path = path + [i]
                    key = (new_bs, new_ec, tuple(new_path))
                    new_states_dict.setdefault(key, []).append(1)

                unique_keys = list(new_states_dict.keys())
                bitsets_batch = [uk[0] for uk in unique_keys]
                ec_batch = [uk[1] for uk in unique_keys]

                # Batched вычисление Fiedler
                fiedlers = batch_laplacian_fiedler(bitsets_batch, n, ec_batch)

                for uk, fv in zip(unique_keys, fiedlers):
                    (new_bs, new_ec, path_tuple) = uk
                    dp[i + 1][k_cur + 1].append((new_bs, new_ec, fv, list(path_tuple)))

                beam_filter(dp[i + 1][k_cur + 1])

    final_states = dp[num_rounds][d]
    if not final_states:
        return [], 0.0
    best_state = max(final_states, key=lambda x: x[2])
    return best_state[3], best_state[2]


##############################################################################
#                 ВСПОМОГАТЕЛЬНАЯ ВИЗУАЛИЗАЦИЯ
##############################################################################

def draw_schedule_graph(n, schedule):
    """
    Создает и отображает граф встреч по итоговому расписанию игр.
    """
    G = nx.Graph()
    G.add_nodes_from(range(n))
    for (u, v) in schedule:
        if G.has_edge(u, v):
            G[u][v]['weight'] += 1
        else:
            G.add_edge(u, v, weight=1)
    pos = nx.spring_layout(G, seed=42)
    edge_labels = nx.get_edge_attributes(G, 'weight')

    plt.figure(figsize=(10, 10))
    nx.draw_networkx_nodes(G, pos, node_size=200)
    nx.draw_networkx_edges(G, pos, width=[edge_labels[edge] for edge in G.edges()])
    nx.draw_networkx_labels(G, pos)
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels)
    plt.title("Граф расписания игр")
    plt.axis('off')
    plt.show()


##############################################################################
#                             MAIN
##############################################################################

if __name__ == "__main__":
    G = 2500  # фиксированное суммарное число игр
    n = 20  # число игроков
    print("n = ", n, " G = ", G)
    for d in range(2, n):
        # skip = 2 * G / (d * n)
        # if skip % 1 != 0 or skip % 2 != 0:
        #     continue
        # configure_cpu_threads(num_threads=6)
        rounds = full_round_robin_schedule(n)
        selected_rounds, best_conn = dp_select_rounds_iterative_batched(rounds, d, n, beam_size=500)
        # Восстанавливаем итоговое расписание
        schedule = []
        for idx in selected_rounds:
            schedule.extend(rounds[idx])

        M = len(schedule)  # число ребер (пар) (матчей)
        g = G / M  # сколько игр на каждую пару

        score = best_conn * g
        print("d = ", d, " g = ", g, " Fiedler = ", best_conn, " Score:", score, " M = ", M)
        print("Оптимально выбранные раунды (индексы):", selected_rounds)
        print("Оптимальное расписание (пары):", schedule, "\n")

        # Визуализация
        draw_schedule_graph(n, schedule)

# d=M/(n/2)
# M = d*n/2
# g = G/M
#
# g = 2*G/(d*n)