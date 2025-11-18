// main.cpp — ЛР-3, "Крестики-нолики с ИИ (минимакс)"
#include "Board.hpp"
#include "MinimaxAI.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <random>
#include <thread>
#include <chrono>

class Game {
private:
    Board board_;
    MinimaxAI aiX_;
    MinimaxAI aiO_;
    bool humanX_;
    bool humanO_;

    // Новые поля для режима показа и случайных начал
    int  speedMode_;                // 1 — быстро, 2 — медленно, 3 — пошагово
    int  openingRandomMovesDone_;   // сколько случайных начальных ходов уже сделано
    int  openingRandomMovesLimit_;  // максимум случайных ходов в начале партии
    std::mt19937 rng_;              // генератор случайных чисел для ИИ vs ИИ

    void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    }

    // Пауза/шаг для ходов ИИ
    void applyAIPause(bool demoMode) {
        // Если есть человек — старое поведение: всегда "нажмите Enter"
        if (!demoMode) {
            std::cout << "\nНажмите Enter, чтобы продолжить...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
            return;
        }

        // Демонстрационный режим (ИИ vs ИИ)
        if (speedMode_ == 1) {
            // Быстро — без пауз
            return;
        } else if (speedMode_ == 2) {
            // Медленно — просто задержка
            std::this_thread::sleep_for(std::chrono::milliseconds(700));
        } else {
            // Пошагово
            std::cout << "\nНажмите Enter, чтобы сделать следующий ход...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        }
    }

    // Ввод хода человека с проверкой
    Coord getHumanMove() {
        int row, col;
        while (true) {
            std::cout << "Введите номер строки и столбца (0-"
                      << (board_.getSize() - 1) << "): ";

            if (!(std::cin >> row >> col)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Некорректный ввод. Введите два целых числа.\n";
                continue;
            }

            if (row >= 0 && row < board_.getSize() &&
                col >= 0 && col < board_.getSize() &&
                board_.isEmpty(row, col)) {
                return Coord(row, col);
            }

            std::cout << "Неверный ход. Попробуйте ещё раз.\n";
        }
    }

    // Один ход ИИ (с учётом случайного открытия и пауз)
    Coord makeAIMove(Player currentPlayer,
                     DynamicArray<AIStatistics>& statsHistory) {
        bool demoMode = (!humanX_ && !humanO_);
        Coord move;

        // 1) Немного случайности только в начале партии (для разнообразия)
        if (demoMode && openingRandomMovesDone_ < openingRandomMovesLimit_) {
            DynamicArray<Coord> emptyCells = board_.getEmptyCells();
            if (!emptyCells.empty()) {
                std::uniform_int_distribution<int> idxDist(
                    0, static_cast<int>(emptyCells.size()) - 1
                );
                int idx = idxDist(rng_);
                move = emptyCells[static_cast<std::size_t>(idx)];

                ++openingRandomMovesDone_;

                std::cout << "Случайный начальный ход ИИ: ("
                          << move.row << ", " << move.col << ")\n";

                applyAIPause(demoMode);
                return move;
            }
        }

        // 2) Обычный minimax для всех остальных ходов
        std::cout << "ИИ думает...\n";
        MinimaxAI& ai =
            (currentPlayer == Player::X) ? aiX_ : aiO_;
        MoveEvaluation eval = ai.findBestMove(board_);
        move = eval.move;

        std::cout << "ИИ выбрал ход: (" << move.row << ", "
                  << move.col << ")\n";
        std::cout << "Оценка позиции: " << eval.score << "\n\n";

        ai.getStatistics().print();
        statsHistory.push_back(ai.getStatistics());

        applyAIPause(demoMode);
        return move;
    }

    // Экспорт статистики ИИ в CSV
    void exportStatisticsToCSV(const std::string& filename,
                               const DynamicArray<AIStatistics>& stats) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Не удалось открыть файл: " << filename << std::endl;
            return;
        }

        // Можно и по-русски, но обычно для CSV удобнее латиница
        file << "Move,NodesVisited,NodesGenerated,CacheHits,CacheMisses,TimeMs\n";

        for (size_t i = 0; i < stats.size(); ++i) {
            file << (i + 1) << ","
                 << stats[i].nodesVisited << ","
                 << stats[i].nodesGenerated << ","
                 << stats[i].cacheHits << ","
                 << stats[i].cacheMisses << ","
                 << stats[i].timeMs << "\n";
        }

        file.close();
        std::cout << "Статистика сохранена в файл " << filename << std::endl;
    }

public:
    Game(int boardSize, int winLength, bool humanX, bool humanO,
         int aiDepth, bool useMemoization,
         int speedMode = 3,
         int openingRandomMovesLimit = 0)
        : board_(boardSize, winLength),
          aiX_(Player::X, aiDepth, useMemoization),
          aiO_(Player::O, aiDepth, useMemoization),
          humanX_(humanX),
          humanO_(humanO),
          speedMode_(speedMode),
          openingRandomMovesDone_(0),
          openingRandomMovesLimit_(openingRandomMovesLimit),
          rng_(std::random_device{}()) {}

    void play() {
        Player currentPlayer = Player::X;
        DynamicArray<AIStatistics> statsHistory;

        while (true) {
            clearScreen();
            board_.print();
            std::cout << std::endl;

            CellState currentCell =
                (currentPlayer == Player::X) ? CellState::X : CellState::O;
            bool isHuman =
                (currentPlayer == Player::X) ? humanX_ : humanO_;

            std::cout << "Ход игрока " << static_cast<char>(currentCell) << "\n";

            Coord move;
            if (isHuman) {
                move = getHumanMove();
            } else {
                move = makeAIMove(currentPlayer, statsHistory);
            }

            board_.set(move, currentCell);

            // Проверка победы
            if (board_.checkWin(currentCell)) {
                clearScreen();
                board_.print();
                std::cout << "\nПобедил игрок "
                          << static_cast<char>(currentCell) << "!\n";

                if (!statsHistory.empty()) {
                    std::cout << "\nСохранить статистику в CSV? (y/n): ";
                    char choice;
                    while (true) {
                        if (!(std::cin >> choice)) {
                            std::cin.clear();
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                            continue;
                        }
                        if (choice == 'y' || choice == 'Y' ||
                            choice == 'n' || choice == 'N') {
                            break;
                        }
                        std::cout << "Введите 'y' или 'n': ";
                    }
                    if (choice == 'y' || choice == 'Y') {
                        exportStatisticsToCSV("game_statistics.csv", statsHistory);
                    }
                }
                break;
            }

            // Проверка ничьей
            if (board_.isFull()) {
                clearScreen();
                board_.print();
                std::cout << "\nНичья!\n";

                if (!statsHistory.empty()) {
                    std::cout << "\nСохранить статистику в CSV? (y/n): ";
                    char choice;
                    while (true) {
                        if (!(std::cin >> choice)) {
                            std::cin.clear();
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                            continue;
                        }
                        if (choice == 'y' || choice == 'Y' ||
                            choice == 'n' || choice == 'N') {
                            break;
                        }
                        std::cout << "Введите 'y' или 'n': ";
                    }
                    if (choice == 'y' || choice == 'Y') {
                        exportStatisticsToCSV("game_statistics.csv", statsHistory);
                    }
                }
                break;
            }

            // Смена игрока
            currentPlayer = (currentPlayer == Player::X)
                          ? Player::O : Player::X;
        }
    }
};

void printMenu() {
    std::cout << "╔═══════════════════════════════════════════════╗\n";
    std::cout << "║  Крестики-нолики с ИИ (минимакс)              ║\n";
    std::cout << "╚═══════════════════════════════════════════════╝\n\n";

    std::cout << "Главное меню:\n";
    std::cout << "1. Человек против ИИ\n";
    std::cout << "2. Демонстрация (AI vs AI)\n";
    std::cout << "3. Человек против человека\n";
    std::cout << "4. Сравнить алгоритмы (с/без мемоизации)\n";
    std::cout << "0. Выход\n\n";
    std::cout << "Выберите опцию: ";
}

void compareAlgorithms() {
    std::cout << "\n=== Сравнение алгоритмов ===\n";

    int size;
    std::cout << "Размер поля: ";
    while (!(std::cin >> size) || size < 3) {
        std::cout << "Некорректный размер. Введите число >= 3: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    int winLen;
    std::cout << "Длина линии для выигрыша (3-" << size << "): ";
    while (!(std::cin >> winLen) || winLen < 3 || winLen > size) {
        std::cout << "Некорректная длина. Введите число от 3 до " << size << ": ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    int depth;
    std::cout << "Глубина поиска ИИ: ";
    while (!(std::cin >> depth) || depth < 1) {
        std::cout << "Некорректная глубина. Введите число >= 1: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "\n--- Без мемоизации ---\n";
    Board board1(size, winLen);
    MinimaxAI ai1(Player::X, depth, false);
    MoveEvaluation eval1 = ai1.findBestMove(board1);
    ai1.getStatistics().print();

    std::cout << "\n--- С мемоизацией ---\n";
    Board board2(size, winLen);
    MinimaxAI ai2(Player::X, depth, true);
    MoveEvaluation eval2 = ai2.findBestMove(board2);
    ai2.getStatistics().print();

    std::cout << "\n--- Сравнение ---\n";
    const AIStatistics& s1 = ai1.getStatistics();
    const AIStatistics& s2 = ai2.getStatistics();

    double speedup =
        (s2.timeMs > 0) ? static_cast<double>(s1.timeMs) / s2.timeMs : 0.0;
    double nodeReduction =
        (s1.nodesVisited > 0)
        ? 100.0 * (1.0 - static_cast<double>(s2.nodesVisited) / s1.nodesVisited)
        : 0.0;

    std::cout << "Ускорение по времени: " << speedup << "x\n";
    std::cout << "Снижение числа посещённых узлов: " << nodeReduction << "%\n";

    std::cout << "\nНажмите Enter, чтобы продолжить...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int main() {
    while (true) {
        printMenu();

        int choice;
        while (!(std::cin >> choice)) {
            std::cout << "Некорректный ввод. Введите номер пункта меню: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        if (choice == 0) {
            std::cout << "До свидания!\n";
            break;
        }

        if (choice == 4) {
            compareAlgorithms();
            continue;
        }

        if (choice < 1 || choice > 3) {
            std::cout << "Некорректный выбор пункта.\n";
            continue;
        }

        int size;
        std::cout << "\nРазмер поля (3-10): ";
        while (!(std::cin >> size) || size < 3 || size > 10) {
            std::cout << "Некорректный размер. Введите число от 3 до 10: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        int winLen;
        std::cout << "Длина линии для выигрыша (3-" << size << "): ";
        while (!(std::cin >> winLen) || winLen < 3 || winLen > size) {
            std::cout << "Некорректная длина. Введите число от 3 до " << size << ": ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        int aiDepth = 9;
        bool useMemo = true;
        int speedMode = 3;
        int openingRandomMovesLimit = 0;

        if (choice == 1 || choice == 2) {
            int recommended = std::min(9, size * 2);
            std::cout << "Глубина поиска ИИ (1-9, рекомендовано "
                      << recommended << "): ";
            while (!(std::cin >> aiDepth) || aiDepth < 1 || aiDepth > 9) {
                std::cout << "Некорректная глубина. Введите число от 1 до 9: ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            std::cout << "Использовать мемоизацию? (1=да, 0=нет): ";
            int memo;
            while (!(std::cin >> memo) || (memo != 0 && memo != 1)) {
                std::cout << "Введите 1 (да) или 0 (нет): ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            useMemo = (memo == 1);
        }

        bool humanX = false;
        bool humanO = false;

        if (choice == 1) {
            // --- Человек против ИИ ---
            std::cout << "\nЗа кого хотите играть?\n";
            std::cout << "1. X (вы ходите первым)\n";
            std::cout << "2. O (ИИ ходит первым)\n";
            std::cout << "Ваш выбор: ";

            int side = 0;
            while (!(std::cin >> side) || (side != 1 && side != 2)) {
                std::cout << "Ошибка. Введите 1 (X) или 2 (O): ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            if (side == 1) {
                humanX = true;
                humanO = false;
            } else {
                humanX = false;
                humanO = true;
            }
        }
        else if (choice == 2) {
            // --- ИИ против ИИ ---
            humanX = false;
            humanO = false;
        }
        else if (choice == 3) {
            // --- Человек против Человека ---
            humanX = true;
            humanO = true;
        }

        // Для ИИ vs ИИ — выбор скорости и немного рандома в начале
        if (!humanX && !humanO) {
            std::cout << "\nРежим показа партии (ИИ vs ИИ):\n";
            std::cout << "1. Быстро (без пауз)\n";
            std::cout << "2. Медленно (пауза между ходами)\n";
            std::cout << "3. Пошагово (по нажатию Enter)\n";
            std::cout << "Выберите режим: ";
            while (!(std::cin >> speedMode) || speedMode < 1 || speedMode > 3) {
                std::cout << "Некорректный выбор. Введите 1, 2 или 3: ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            // КОЛИТЧЕСТВО РАНДОМНЫХ ХОДОВ В НАЧАЛЕ ОТ БОТОВ
            openingRandomMovesLimit = 2;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        Game game(size, winLen, humanX, humanO,
                  aiDepth, useMemo,
                  speedMode, openingRandomMovesLimit);

        game.play();
    }

    return 0;
}
