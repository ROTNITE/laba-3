// MinimaxAI.hpp
#pragma once
#include "Board.hpp"
#include "HashMap.hpp"
#include "DynamicArray.hpp"
#include <limits>
#include <chrono>
#include <iostream>

enum class Player {
    X,
    O
};

struct MoveEvaluation {
    Coord move;
    int score;

    MoveEvaluation() : move(0, 0), score(0) {}
    MoveEvaluation(const Coord& m, int s) : move(m), score(s) {}
};

struct AIStatistics {
    size_t nodesVisited;
    size_t nodesGenerated;
    size_t cacheHits;
    size_t cacheMisses;
    long long timeMs;

    AIStatistics()
        : nodesVisited(0),
          nodesGenerated(0),
          cacheHits(0),
          cacheMisses(0),
          timeMs(0) {}

    void reset() {
        nodesVisited = 0;
        nodesGenerated = 0;
        cacheHits = 0;
        cacheMisses = 0;
        timeMs = 0;
    }

    void print() const {
        std::cout << "Статистика работы ИИ:\n";
        std::cout << "  Посещено узлов: " << nodesVisited << "\n";
        std::cout << "  Сгенерировано узлов: " << nodesGenerated << "\n";
        std::cout << "  Попаданий в кеш: " << cacheHits << "\n";
        std::cout << "  Промахов мимо кеша: " << cacheMisses << "\n";
        std::cout << "  Время работы: " << timeMs << " мс\n";
        if (cacheHits + cacheMisses > 0) {
            double hitRate =
                100.0 * static_cast<double>(cacheHits)
                / static_cast<double>(cacheHits + cacheMisses);
            std::cout << "  Доля попаданий в кеш: " << hitRate << "%\n";
        }
    }
};

class MinimaxAI {
private:
    Player player_;
    Player opponent_;
    int maxDepth_;
    bool useMemoization_;

    // Транспозиционная таблица для мемоизации
    HashMap<size_t, int> transpositionTable_;

    AIStatistics stats_;

    CellState playerToCell(Player p) const {
        return p == Player::X ? CellState::X : CellState::O;
    }

    Player getOpponent(Player p) const {
        return p == Player::X ? Player::O : Player::X;
    }

    // Эвристическая оценка позиции
    int evaluate(const Board& board) const {
        CellState playerCell = playerToCell(player_);
        CellState opponentCell = playerToCell(opponent_);

        // Проверка победы
        if (board.checkWin(playerCell)) {
            return 1000;
        }
        if (board.checkWin(opponentCell)) {
            return -1000;
        }

        int score = 0;
        int size = board.getSize();
        int winLen = board.getWinLength();
        (void)winLen; // пока winLen напрямую не используется в эвристике

        // Оценка позиций (центр лучше краёв)
        for (int row = 0; row < size; ++row) {
            for (int col = 0; col < size; ++col) {
                CellState cell = board.get(row, col);
                if (cell == CellState::Empty) continue;

                int centerDist =
                    std::abs(row - size / 2) + std::abs(col - size / 2);
                int posValue = (size - centerDist);

                if (cell == playerCell) {
                    score += posValue;
                } else if (cell == opponentCell) {
                    score -= posValue;
                }
            }
        }

        return score;
    }

    // Минимакс с альфа-бета отсечением
    int minimax(Board& board, int depth, int alpha, int beta,
                Player currentPlayer, bool isMaximizing) {

        stats_.nodesVisited++;

        CellState playerCell = playerToCell(player_);
        CellState opponentCell = playerToCell(opponent_);

        // Проверка терминального состояния
        if (board.checkWin(playerCell)) {
            return 1000 + depth; // предпочитаем более быстрые победы
        }
        if (board.checkWin(opponentCell)) {
            return -1000 - depth;
        }
        if (board.isFull() || depth <= 0) {
            return evaluate(board);
        }

        // Проверка кеша
        if (useMemoization_) {
            size_t hash = board.hash();
            if (transpositionTable_.contains(hash)) {
                stats_.cacheHits++;
                return transpositionTable_.get(hash);
            }
            stats_.cacheMisses++;
        }

        DynamicArray<Coord> moves = board.getEmptyCells();
        stats_.nodesGenerated += moves.size();

        int bestScore;
        CellState currentCell = playerToCell(currentPlayer);

        if (isMaximizing) {
            bestScore = std::numeric_limits<int>::min();

            for (size_t i = 0; i < moves.size(); ++i) {
                board.set(moves[i], currentCell);
                int score = minimax(board, depth - 1, alpha, beta,
                                    getOpponent(currentPlayer), false);
                board.set(moves[i], CellState::Empty);

                bestScore = std::max(bestScore, score);
                alpha = std::max(alpha, bestScore);

                if (beta <= alpha) {
                    break; // альфа-бета отсечение
                }
            }
        } else {
            bestScore = std::numeric_limits<int>::max();

            for (size_t i = 0; i < moves.size(); ++i) {
                board.set(moves[i], currentCell);
                int score = minimax(board, depth - 1, alpha, beta,
                                    getOpponent(currentPlayer), true);
                board.set(moves[i], CellState::Empty);

                bestScore = std::min(bestScore, score);
                beta = std::min(beta, bestScore);

                if (beta <= alpha) {
                    break;
                }
            }
        }

        // Сохранение в кеш
        if (useMemoization_) {
            transpositionTable_.insert(board.hash(), bestScore);
        }

        return bestScore;
    }

public:
    MinimaxAI(Player player, int maxDepth = 9, bool useMemoization = true)
        : player_(player),
          opponent_(getOpponent(player)),
          maxDepth_(maxDepth),
          useMemoization_(useMemoization) {}

    MoveEvaluation findBestMove(Board& board) {
        stats_.reset();
        auto startTime = std::chrono::high_resolution_clock::now();

        DynamicArray<Coord> moves = board.getEmptyCells();
        if (moves.empty()) {
            return MoveEvaluation();
        }

        // Если доска пустая — ходим в центр
        if (moves.size() == static_cast<size_t>(board.getSize() * board.getSize())) {
            int center = board.getSize() / 2;
            stats_.timeMs = 0;
            return MoveEvaluation(Coord(center, center), 0);
        }

        MoveEvaluation bestMove;
        bestMove.score = std::numeric_limits<int>::min();
        CellState playerCell = playerToCell(player_);

        int alpha = std::numeric_limits<int>::min();
        int beta = std::numeric_limits<int>::max();

        for (size_t i = 0; i < moves.size(); ++i) {
            board.set(moves[i], playerCell);

            int score = minimax(board, maxDepth_ - 1, alpha, beta,
                                opponent_, false);

            board.set(moves[i], CellState::Empty);

            if (score > bestMove.score) {
                bestMove.score = score;
                bestMove.move = moves[i];
            }

            alpha = std::max(alpha, score);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        stats_.timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime).count();

        return bestMove;
    }

    const AIStatistics& getStatistics() const {
        return stats_;
    }

    void clearCache() {
        transpositionTable_.clear();
    }

    void setUseMemoization(bool use) {
        useMemoization_ = use;
    }
};
