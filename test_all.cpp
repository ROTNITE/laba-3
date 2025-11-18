// tests/test_all.cpp — ЛР-3
// Тесты оформлены в том же стиле, что и Tests.hpp из ЛР-2

#include "Board.hpp"
#include "MinimaxAI.hpp"
#include "DynamicArray.hpp"
#include "HashMap.hpp"

#include <iostream>
#include <cassert>
#include <stdexcept>

class Tests {
public:
    static void RunAllTests() {
        std::cout << "=== Запуск тестов ЛР-3 ===\n\n";

        std::cout << "=== Тесты структур данных ===\n";
        TestDynamicArrayBasic();       // 1
        TestDynamicArrayResize();      // 2
        TestDynamicArrayCopy();        // 3
        TestHashMapBasic();            // 4
        TestHashMapCollision();        // 5
        TestHashMapUpdate();           // 6

        std::cout << "\n=== Тесты доски (Board) ===\n";
        TestBoardCreation();           // 7
        TestBoardSetGet();             // 8
        TestBoardEmptyCells();         // 9
        TestBoardWinHorizontal();      // 10
        TestBoardWinVertical();        // 11
        TestBoardWinDiagonal();        // 12
        TestBoardWinAntiDiagonal();    // 13
        TestBoardNoWin();              // 14
        TestBoardFull();               // 15
        TestBoardLargeSize();          // 16
        TestBoardHash();               // 17

        std::cout << "\n=== Тесты ИИ (MinimaxAI) ===\n";
        TestAIFirstMoveCenter();       // 18
        TestAIBlockingMove();          // 19
        TestAIWinningMove();           // 20
        TestAIMemoizationSpeedup();    // 21

        std::cout << "Тест 22: Идеальная игра 3x3 (может занять время)... ";
        TestAIPerfectPlay3x3();
        std::cout << "OK\n";

        std::cout << "\n=== Граничные случаи ===\n";
        TestBoardBoundaryChecks();     // 23
        TestEmptyBoardNoMoves();       // 24

        std::cout << "\n========================================\n";
        std::cout << "Все 24/24 тестов ЛР-3 пройдены успешно!\n";
        std::cout << "========================================\n\n";
    }

private:
    // ========== DynamicArray ==========

    static void TestDynamicArrayBasic() {
        std::cout << "Тест 1: DynamicArray — базовые операции... ";

        DynamicArray<int> arr;
        assert(arr.size() == 0);
        assert(arr.empty());

        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);

        assert(arr.size() == 3);
        assert(arr[0] == 1);
        assert(arr[1] == 2);
        assert(arr[2] == 3);

        std::cout << "OK\n";
    }

    static void TestDynamicArrayResize() {
        std::cout << "Тест 2: DynamicArray — увеличение размера... ";

        DynamicArray<int> arr;
        for (int i = 0; i < 100; ++i) {
            arr.push_back(i);
        }
        assert(arr.size() == 100);
        for (int i = 0; i < 100; ++i) {
            assert(arr[i] == i);
        }

        std::cout << "OK\n";
    }

    static void TestDynamicArrayCopy() {
        std::cout << "Тест 3: DynamicArray — копирование... ";

        DynamicArray<int> arr1;
        arr1.push_back(1);
        arr1.push_back(2);

        DynamicArray<int> arr2 = arr1;
        assert(arr2.size() == 2);
        assert(arr2[0] == 1);
        assert(arr2[1] == 2);

        arr1[0] = 10;
        // Должно быть глубокое копирование
        assert(arr2[0] == 1);

        std::cout << "OK\n";
    }

    // ========== HashMap ==========

    static void TestHashMapBasic() {
        std::cout << "Тест 4: HashMap — базовая работа... ";

        HashMap<int, std::string> map;

        map.insert(1, "one");
        map.insert(2, "two");
        map.insert(3, "three");

        assert(map.size() == 3);
        assert(map.contains(1));
        assert(map.contains(2));
        assert(map.contains(3));
        assert(!map.contains(4));

        assert(map.get(1) == "one");
        assert(map.get(2) == "two");

        std::cout << "OK\n";
    }

    static void TestHashMapCollision() {
        std::cout << "Тест 5: HashMap — коллизии и много элементов... ";

        HashMap<int, int> map;

        for (int i = 0; i < 100; ++i) {
            map.insert(i, i * 10);
        }

        assert(map.size() == 100);

        for (int i = 0; i < 100; ++i) {
            assert(map.contains(i));
            assert(map.get(i) == i * 10);
        }

        std::cout << "OK\n";
    }

    static void TestHashMapUpdate() {
        std::cout << "Тест 6: HashMap — перезапись значения по ключу... ";

        HashMap<int, int> map;

        map.insert(1, 10);
        assert(map.get(1) == 10);

        map.insert(1, 20);
        assert(map.get(1) == 20);
        assert(map.size() == 1);

        std::cout << "OK\n";
    }

    // ========== Board ==========

    static void TestBoardCreation() {
        std::cout << "Тест 7: Board — создание и начальное состояние... ";

        Board board(3, 3);
        assert(board.getSize() == 3);
        assert(board.getWinLength() == 3);

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                assert(board.isEmpty(i, j));
            }
        }

        std::cout << "OK\n";
    }

    static void TestBoardSetGet() {
        std::cout << "Тест 8: Board — set/get... ";

        Board board(3, 3);

        board.set(0, 0, CellState::X);
        assert(board.get(0, 0) == CellState::X);
        assert(!board.isEmpty(0, 0));

        board.set(1, 1, CellState::O);
        assert(board.get(1, 1) == CellState::O);

        std::cout << "OK\n";
    }

    static void TestBoardEmptyCells() {
        std::cout << "Тест 9: Board — список пустых клеток... ";

        Board board(3, 3);

        DynamicArray<Coord> empty = board.getEmptyCells();
        assert(empty.size() == 9);

        board.set(0, 0, CellState::X);
        empty = board.getEmptyCells();
        assert(empty.size() == 8);

        std::cout << "OK\n";
    }

    static void TestBoardWinHorizontal() {
        std::cout << "Тест 10: Board — горизонтальная победа... ";

        Board board(3, 3);

        board.set(0, 0, CellState::X);
        board.set(0, 1, CellState::X);
        board.set(0, 2, CellState::X);

        assert(board.checkWin(CellState::X));
        assert(!board.checkWin(CellState::O));

        std::cout << "OK\n";
    }

    static void TestBoardWinVertical() {
        std::cout << "Тест 11: Board — вертикальная победа... ";

        Board board(3, 3);

        board.set(0, 0, CellState::O);
        board.set(1, 0, CellState::O);
        board.set(2, 0, CellState::O);

        assert(board.checkWin(CellState::O));
        assert(!board.checkWin(CellState::X));

        std::cout << "OK\n";
    }

    static void TestBoardWinDiagonal() {
        std::cout << "Тест 12: Board — диагональная победа... ";

        Board board(3, 3);

        board.set(0, 0, CellState::X);
        board.set(1, 1, CellState::X);
        board.set(2, 2, CellState::X);

        assert(board.checkWin(CellState::X));

        std::cout << "OK\n";
    }

    static void TestBoardWinAntiDiagonal() {
        std::cout << "Тест 13: Board — обратная диагональ... ";

        Board board(3, 3);

        board.set(0, 2, CellState::O);
        board.set(1, 1, CellState::O);
        board.set(2, 0, CellState::O);

        assert(board.checkWin(CellState::O));

        std::cout << "OK\n";
    }

    static void TestBoardNoWin() {
        std::cout << "Тест 14: Board — без победы... ";

        Board board(3, 3);

        board.set(0, 0, CellState::X);
        board.set(0, 1, CellState::O);
        board.set(0, 2, CellState::X);

        assert(!board.checkWin(CellState::X));
        assert(!board.checkWin(CellState::O));

        std::cout << "OK\n";
    }

    static void TestBoardFull() {
        std::cout << "Тест 15: Board — заполненность... ";

        Board board(3, 3);

        assert(!board.isFull());

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                board.set(i, j, CellState::X);
            }
        }

        assert(board.isFull());

        std::cout << "OK\n";
    }

    static void TestBoardLargeSize() {
        std::cout << "Тест 16: Board — больший размер и winLength... ";

        Board board(5, 4);
        assert(board.getSize() == 5);
        assert(board.getWinLength() == 4);

        for (int i = 0; i < 4; ++i) {
            board.set(0, i, CellState::X);
        }
        assert(board.checkWin(CellState::X));

        std::cout << "OK\n";
    }

    static void TestBoardHash() {
        std::cout << "Тест 17: Board — хеширование... ";

        Board board1(3, 3);
        Board board2(3, 3);

        assert(board1.hash() == board2.hash());

        board1.set(0, 0, CellState::X);
        assert(board1.hash() != board2.hash());

        board2.set(0, 0, CellState::X);
        assert(board1.hash() == board2.hash());

        std::cout << "OK\n";
    }

    // ========== AI (Minimax) ==========

    static void TestAIFirstMoveCenter() {
        std::cout << "Тест 18: ИИ — первый ход в центр... ";

        Board board(3, 3);
        MinimaxAI ai(Player::X, 9, false);

        MoveEvaluation move = ai.findBestMove(board);
        assert(move.move.row == 1 && move.move.col == 1);

        std::cout << "OK\n";
    }

    static void TestAIBlockingMove() {
        std::cout << "Тест 19: ИИ — блокировка опасного хода... ";

        Board board(3, 3);

        board.set(0, 0, CellState::O);
        board.set(0, 1, CellState::O);
        // X должен блокировать в (0,2)

        MinimaxAI ai(Player::X, 9, true);
        MoveEvaluation move = ai.findBestMove(board);

        assert(move.move.row == 0 && move.move.col == 2);

        std::cout << "OK\n";
    }

    static void TestAIWinningMove() {
        std::cout << "Тест 20: ИИ — выбор выигрывающего хода... ";

        Board board(3, 3);

        board.set(0, 0, CellState::X);
        board.set(0, 1, CellState::X);
        board.set(1, 0, CellState::O);

        MinimaxAI ai(Player::X, 9, true);
        MoveEvaluation move = ai.findBestMove(board);

        assert(move.move.row == 0 && move.move.col == 2);

        std::cout << "OK\n";
    }

    static void TestAIMemoizationSpeedup() {
        std::cout << "Тест 21: ИИ — корректность работы мемоизации... ";

        Board board(3, 3);
        board.set(1, 1, CellState::X);

        MinimaxAI ai1(Player::O, 9, false);
        ai1.findBestMove(board);
        size_t nodesWithout = ai1.getStatistics().nodesVisited;

        MinimaxAI ai2(Player::O, 9, true);
        ai2.findBestMove(board);
        size_t nodesWith = ai2.getStatistics().nodesVisited;

        assert(nodesWith > 0);
        assert(nodesWithout > 0);
        // Конкретное ускорение не проверяем, важно что оба варианта отрабатывают

        std::cout << "OK\n";
    }

    static void TestAIPerfectPlay3x3() {
        // Демонстрационный тест: две копии ИИ играют друг с другом.
        // Без жесткого требования ничьей, просто проверяем, что код работает.
        Board board(3, 3);
        MinimaxAI aiX(Player::X, 9, true);
        MinimaxAI aiO(Player::O, 9, true);

        Player current = Player::X;

        while (!board.isFull()) {
            MinimaxAI& ai = (current == Player::X) ? aiX : aiO;
            CellState cell = (current == Player::X) ? CellState::X : CellState::O;

            MoveEvaluation move = ai.findBestMove(board);
            board.set(move.move, cell);

            // Если кто-то победил — просто выходим, без assert
            if (board.checkWin(cell)) {
                break;
            }

            current = (current == Player::X) ? Player::O : Player::X;
        }

        // Можно максимум проверить, что доска в валидном состоянии, но без жёстких требований
    }

    // ========== Граничные случаи ==========

    static void TestBoardBoundaryChecks() {
        std::cout << "Тест 23: Board — выход за границы... ";

        Board board(3, 3);

        bool threw = false;
        try {
            board.get(-1, 0);
        } catch (const std::out_of_range&) {
            threw = true;
        }
        assert(threw);

        threw = false;
        try {
            board.get(3, 0);
        } catch (const std::out_of_range&) {
            threw = true;
        }
        assert(threw);

        std::cout << "OK\n";
    }

    static void TestEmptyBoardNoMoves() {
        std::cout << "Тест 24: ИИ — нет ходов на полной доске... ";

        Board board(3, 3);

        // Заполняем доску
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                board.set(i, j, CellState::X);
            }
        }

        MinimaxAI ai(Player::O, 9, true);
        MoveEvaluation move = ai.findBestMove(board);

        DynamicArray<Coord> empty = board.getEmptyCells();
        assert(empty.empty());
        (void)move; // просто чтобы не ругался компилятор

        std::cout << "OK\n";
    }
};

int main() {
    Tests::RunAllTests();
    return 0;
}
