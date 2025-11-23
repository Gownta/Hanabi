"""
Representation of Hanabi game elements including cards, colors, and numbers.
"""

from dataclasses import dataclass
from enum import IntEnum


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
