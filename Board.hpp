// include/game/Board.hpp
#pragma once
#include "DynamicArray.hpp"
#include <iostream>
#include <string>

enum class CellState : char {
    Empty = ' ',
    X = 'X',
    O = 'O'
};

struct Coord {
    int row;
    int col;

    Coord() : row(0), col(0) {}
    Coord(int r, int c) : row(r), col(c) {}

    bool operator==(const Coord& other) const {
        return row == other.row && col == other.col;
    }

    bool operator!=(const Coord& other) const {
        return !(*this == other);
    }
};

class Board {
private:
    DynamicArray<CellState> cells_;
    int size_;
    int winLength_;

public:
    Board(int size = 3, int winLength = 3)
        : size_(size), winLength_(winLength) {
        cells_.reserve(size * size);
        for (int i = 0; i < size * size; ++i) {
            cells_.push_back(CellState::Empty);
        }
    }

    int getSize() const { return size_; }
    int getWinLength() const { return winLength_; }

    CellState get(int row, int col) const {
        if (row < 0 || row >= size_ || col < 0 || col >= size_) {
            throw std::out_of_range("Invalid coordinates");
        }
        return cells_[row * size_ + col];
    }

    CellState get(const Coord& coord) const {
        return get(coord.row, coord.col);
    }

    void set(int row, int col, CellState state) {
        if (row < 0 || row >= size_ || col < 0 || col >= size_) {
            throw std::out_of_range("Invalid coordinates");
        }
        cells_[row * size_ + col] = state;
    }

    void set(const Coord& coord, CellState state) {
        set(coord.row, coord.col, state);
    }

    bool isEmpty(int row, int col) const {
        return get(row, col) == CellState::Empty;
    }

    bool isEmpty(const Coord& coord) const {
        return isEmpty(coord.row, coord.col);
    }

    bool isFull() const {
        for (int i = 0; i < size_ * size_; ++i) {
            if (cells_[i] == CellState::Empty) {
                return false;
            }
        }
        return true;
    }

    DynamicArray<Coord> getEmptyCells() const {
        DynamicArray<Coord> result;
        for (int row = 0; row < size_; ++row) {
            for (int col = 0; col < size_; ++col) {
                if (isEmpty(row, col)) {
                    result.push_back(Coord(row, col));
                }
            }
        }
        return result;
    }

    // Проверка победы
    bool checkWin(CellState player) const {
        if (player == CellState::Empty) return false;

        // Проверка строк
        for (int row = 0; row < size_; ++row) {
            for (int col = 0; col <= size_ - winLength_; ++col) {
                bool win = true;
                for (int k = 0; k < winLength_; ++k) {
                    if (get(row, col + k) != player) {
                        win = false;
                        break;
                    }
                }
                if (win) return true;
            }
        }

        // Проверка столбцов
        for (int col = 0; col < size_; ++col) {
            for (int row = 0; row <= size_ - winLength_; ++row) {
                bool win = true;
                for (int k = 0; k < winLength_; ++k) {
                    if (get(row + k, col) != player) {
                        win = false;
                        break;
                    }
                }
                if (win) return true;
            }
        }

        // Проверка диагоналей (вниз-вправо)
        for (int row = 0; row <= size_ - winLength_; ++row) {
            for (int col = 0; col <= size_ - winLength_; ++col) {
                bool win = true;
                for (int k = 0; k < winLength_; ++k) {
                    if (get(row + k, col + k) != player) {
                        win = false;
                        break;
                    }
                }
                if (win) return true;
            }
        }

        // Проверка диагоналей (вниз-влево)
        for (int row = 0; row <= size_ - winLength_; ++row) {
            for (int col = winLength_ - 1; col < size_; ++col) {
                bool win = true;
                for (int k = 0; k < winLength_; ++k) {
                    if (get(row + k, col - k) != player) {
                        win = false;
                        break;
                    }
                }
                if (win) return true;
            }
        }

        return false;
    }

    // Хеш для мемоизации
    size_t hash() const {
        size_t h = 0;
        for (int i = 0; i < size_ * size_; ++i) {
            h = h * 3 + static_cast<int>(cells_[i]);
        }
        return h;
    }

    bool operator==(const Board& other) const {
        if (size_ != other.size_) return false;
        for (int i = 0; i < size_ * size_; ++i) {
            if (cells_[i] != other.cells_[i]) return false;
        }
        return true;
    }

    void print() const {
        std::cout << "  ";
        for (int col = 0; col < size_; ++col) {
            std::cout << col << " ";
        }
        std::cout << std::endl;

        for (int row = 0; row < size_; ++row) {
            std::cout << row << " ";
            for (int col = 0; col < size_; ++col) {
                std::cout << static_cast<char>(get(row, col));
                if (col < size_ - 1) std::cout << "|";
            }
            std::cout << std::endl;

            if (row < size_ - 1) {
                std::cout << "  ";
                for (int col = 0; col < size_; ++col) {
                    std::cout << "-";
                    if (col < size_ - 1) std::cout << "+";
                }
                std::cout << std::endl;
            }
        }
    }
};

// Хеш-функция для std::hash<Board>
namespace std {
    template<>
    struct hash<Board> {
        size_t operator()(const Board& board) const {
            return board.hash();
        }
    };
}
