"""
Representation of Hanabi game elements including cards, colors, and numbers.
"""

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


class Card:
    """
    Represents a Hanabi card encoded as an integer.

    The card is represented by: color * 6 + number
    """

    def __init__(self, color: Color, number: Number):
        """
        Initialize a card with a color and number.

        Args:
            color: The color of the card
            number: The number of the card
        """
        self._value = color * 6 + number

    @classmethod
    def from_value(cls, value: int) -> "Card":
        """Create a Card from its integer representation"""
        color = Color(value // 6)
        number = Number(value % 6)
        card = cls.__new__(cls)
        card._value = value
        return card

    def get_color(self) -> Color:
        """Get the color of the card"""
        return Color(self._value // 6)

    def get_number(self) -> Number:
        """Get the number of the card"""
        return Number(self._value % 6)

    def value(self) -> int:
        """Get the integer representation of the card"""
        return self._value

    def __repr__(self) -> str:
        return f"Card({self.get_color().name}, {self.get_number().name})"

    def __eq__(self, other) -> bool:
        if not isinstance(other, Card):
            return False
        return self._value == other._value

    def __hash__(self) -> int:
        return hash(self._value)
