#pragma once

constexpr int CARDS_PER_PLAYER[] = {
    -1, // 0
    -1, // 1
    5, // 2
    5, // 3
    4, // 4
    4 // 5
};

constexpr int CARDS_PER_VALUE[] = {
    -1, // 0
    3, // 1
    2, // 2
    2, // 3
    2, // 4
    1, // 5
};

constexpr int NUM_COLOURS = 5;
constexpr int NUM_UNIQUE_CARDS = 25;
constexpr int DECK_SIZE = 50;

constexpr int STARTING_BOMBS = 3;
constexpr int STARTING_HINTS = 8;
constexpr int MAX_HINTS = 8;

constexpr int PRINT_WIDTH = 20;
