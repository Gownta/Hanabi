"""Unit tests for repr.py"""

import unittest
from src.repr import Color, Number, Card


class TestColor(unittest.TestCase):
    """Test the Color enum"""

    def test_color_values(self):
        """Verify Color enum values"""
        self.assertEqual(Color.RED, 0)
        self.assertEqual(Color.GREEN, 1)
        self.assertEqual(Color.BLUE, 2)
        self.assertEqual(Color.WHITE, 3)
        self.assertEqual(Color.YELLOW, 4)


class TestNumber(unittest.TestCase):
    """Test the Number enum"""

    def test_number_values(self):
        """Verify Number enum values are one-indexed"""
        self.assertEqual(Number.ONE, 1)
        self.assertEqual(Number.TWO, 2)
        self.assertEqual(Number.THREE, 3)
        self.assertEqual(Number.FOUR, 4)
        self.assertEqual(Number.FIVE, 5)


class TestCard(unittest.TestCase):
    """Test the Card dataclass"""

    def test_card_creation(self):
        """Test creating a card with color and number"""
        card = Card(Color.RED, Number.ONE)
        self.assertEqual(card.color, Color.RED)
        self.assertEqual(card.number, Number.ONE)

    def test_all_combinations(self):
        """Test all valid color and number combinations"""
        for color in Color:
            for number in Number:
                card = Card(color, number)
                self.assertEqual(card.color, color)
                self.assertEqual(card.number, number)

    def test_card_equality(self):
        """Test card equality comparison"""
        card1 = Card(Color.RED, Number.ONE)
        card2 = Card(Color.RED, Number.ONE)
        card3 = Card(Color.BLUE, Number.ONE)

        self.assertEqual(card1, card2)
        self.assertNotEqual(card1, card3)

    def test_card_hash(self):
        """Test that cards can be hashed (for use in sets/dicts)"""
        card1 = Card(Color.RED, Number.ONE)
        card2 = Card(Color.RED, Number.ONE)
        card3 = Card(Color.BLUE, Number.ONE)

        self.assertEqual(hash(card1), hash(card2))
        card_set = {card1, card2, card3}
        self.assertEqual(len(card_set), 2)

    def test_card_repr(self):
        """Test card string representation"""
        card = Card(Color.RED, Number.FIVE)
        self.assertEqual(
            repr(card), "Card(color=<Color.RED: 0>, number=<Number.FIVE: 5>)"
        )

    def test_card_immutable(self):
        """Test that cards are immutable (frozen dataclass)"""
        card = Card(Color.RED, Number.ONE)
        with self.assertRaises(Exception):
            card.color = Color.BLUE


if __name__ == "__main__":
    unittest.main()
