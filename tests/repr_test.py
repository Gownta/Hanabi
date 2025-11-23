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
    """Test the Card class"""

    def test_card_creation(self):
        """Test creating a card with color and number"""
        card = Card(Color.RED, Number.ONE)
        self.assertEqual(card.get_color(), Color.RED)
        self.assertEqual(card.get_number(), Number.ONE)

    def test_card_value_formula(self):
        """Test that card value follows color * 6 + number formula"""
        card = Card(Color.RED, Number.ONE)
        self.assertEqual(card.value(), 0 * 6 + 1)  # 1

        card = Card(Color.GREEN, Number.THREE)
        self.assertEqual(card.value(), 1 * 6 + 3)  # 9

        card = Card(Color.YELLOW, Number.FIVE)
        self.assertEqual(card.value(), 4 * 6 + 5)  # 29

    def test_get_color(self):
        """Test getting the color from a card"""
        card = Card(Color.BLUE, Number.TWO)
        self.assertEqual(card.get_color(), Color.BLUE)

    def test_get_number(self):
        """Test getting the number from a card"""
        card = Card(Color.WHITE, Number.FOUR)
        self.assertEqual(card.get_number(), Number.FOUR)

    def test_card_from_value(self):
        """Test creating a card from its integer value"""
        card = Card.from_value(1 * 6 + 3)  # GREEN THREE
        self.assertEqual(card.get_color(), Color.GREEN)
        self.assertEqual(card.get_number(), Number.THREE)

    def test_card_roundtrip(self):
        """Test that creating a card and converting to/from value works"""
        original = Card(Color.YELLOW, Number.TWO)
        value = original.value()
        recreated = Card.from_value(value)
        self.assertEqual(original, recreated)

    def test_all_combinations(self):
        """Test all valid color and number combinations"""
        for color in Color:
            for number in Number:
                card = Card(color, number)
                self.assertEqual(card.get_color(), color)
                self.assertEqual(card.get_number(), number)
                self.assertEqual(card.value(), color * 6 + number)

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
        self.assertEqual(len(card_set), 2)  # card1 and card2 are the same

    def test_card_repr(self):
        """Test card string representation"""
        card = Card(Color.RED, Number.FIVE)
        self.assertEqual(repr(card), "Card(RED, FIVE)")


if __name__ == "__main__":
    unittest.main()
