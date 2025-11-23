"""
Representation of Hanabi game elements including cards, colors, and numbers.
"""

import random
from dataclasses import dataclass
from enum import IntEnum
from typing import Optional


class Color(IntEnum):
    """Card colors in Hanabi"""

    RED = 0
    GREEN = 1
    BLUE = 2
    WHITE = 3
    YELLOW = 4


class Number(IntEnum):
    """Card numbers in Hanabi (one-indexed)"""

    ONE = 1
    TWO = 2
    THREE = 3
    FOUR = 4
    FIVE = 5


@dataclass(frozen=True)
class Card:
    """Represents a Hanabi card with a color and number"""

    color: Color
    number: Number


@dataclass
class Hand:
    """Represents a hand of Hanabi cards"""

    cards: list[Card]


class Deck:
    """
    Represents a Hanabi deck of cards.

    A Hanabi deck contains 50 cards total (10 cards per color):
    - 3x ones
    - 2x twos
    - 2x threes
    - 2x fours
    - 1x five
    """

    def __init__(self, seed: Optional[int] = None):
        """
        Initialize a shuffled Hanabi deck.

        Args:
            seed: Optional random seed for shuffling
        """
        self.cards: list[Card] = []

        # Card distribution per color: 3x1, 2x2, 2x3, 2x4, 1x5
        card_counts = {
            Number.ONE: 3,
            Number.TWO: 2,
            Number.THREE: 2,
            Number.FOUR: 2,
            Number.FIVE: 1,
        }

        for color in Color:
            for number, count in card_counts.items():
                for _ in range(count):
                    self.cards.append(Card(color, number))

        # Shuffle the deck
        rng = random.Random(seed)
        rng.shuffle(self.cards)

    def draw(self) -> Optional[Card]:
        """Draw a card from the top of the deck, or None if empty"""
        if self.cards:
            return self.cards.pop()
        return None

    def __len__(self) -> int:
        """Return the number of cards remaining in the deck"""
        return len(self.cards)
