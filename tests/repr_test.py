"""Unit tests for repr.py"""

import unittest
from collections import Counter
from src.repr import Color, Number, Card, Hand, Deck


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


class TestHand(unittest.TestCase):
    """Test the Hand dataclass"""

    def test_hand_creation_empty(self):
        """Test creating an empty hand"""
        hand = Hand(cards=[])
        self.assertEqual(len(hand.cards), 0)

    def test_hand_creation_with_cards(self):
        """Test creating a hand with cards"""
        cards = [
            Card(Color.RED, Number.ONE),
            Card(Color.BLUE, Number.TWO),
            Card(Color.GREEN, Number.THREE),
        ]
        hand = Hand(cards=cards)
        self.assertEqual(len(hand.cards), 3)
        self.assertEqual(hand.cards[0], Card(Color.RED, Number.ONE))
        self.assertEqual(hand.cards[1], Card(Color.BLUE, Number.TWO))
        self.assertEqual(hand.cards[2], Card(Color.GREEN, Number.THREE))

    def test_hand_mutable(self):
        """Test that hands are mutable (can add/remove cards)"""
        hand = Hand(cards=[])
        card1 = Card(Color.RED, Number.ONE)
        card2 = Card(Color.BLUE, Number.TWO)

        hand.cards.append(card1)
        self.assertEqual(len(hand.cards), 1)

        hand.cards.append(card2)
        self.assertEqual(len(hand.cards), 2)

        hand.cards.remove(card1)
        self.assertEqual(len(hand.cards), 1)
        self.assertEqual(hand.cards[0], card2)

    def test_hand_equality(self):
        """Test hand equality comparison"""
        cards1 = [Card(Color.RED, Number.ONE), Card(Color.BLUE, Number.TWO)]
        cards2 = [Card(Color.RED, Number.ONE), Card(Color.BLUE, Number.TWO)]
        cards3 = [Card(Color.RED, Number.ONE)]

        hand1 = Hand(cards=cards1)
        hand2 = Hand(cards=cards2)
        hand3 = Hand(cards=cards3)

        self.assertEqual(hand1, hand2)
        self.assertNotEqual(hand1, hand3)

    def test_hand_repr(self):
        """Test hand string representation"""
        cards = [Card(Color.RED, Number.ONE), Card(Color.BLUE, Number.TWO)]
        hand = Hand(cards=cards)
        expected = "Hand(cards=[Card(color=<Color.RED: 0>, number=<Number.ONE: 1>), Card(color=<Color.BLUE: 2>, number=<Number.TWO: 2>)])"
        self.assertEqual(repr(hand), expected)


class TestDeck(unittest.TestCase):
    """Test the Deck class"""

    def test_deck_size(self):
        """Test that a new deck has 50 cards"""
        deck = Deck(seed=42)
        self.assertEqual(len(deck), 50)

    def test_deck_composition(self):
        """
        Test that deck has correct card distribution.

        Per color: 3x1, 2x2, 2x3, 2x4, 1x5 (10 cards per color, 50 total)
        """
        deck = Deck(seed=42)
        card_counts = Counter((card.color, card.number) for card in deck.cards)

        for color in Color:
            self.assertEqual(card_counts[(color, Number.ONE)], 3)
            self.assertEqual(card_counts[(color, Number.TWO)], 2)
            self.assertEqual(card_counts[(color, Number.THREE)], 2)
            self.assertEqual(card_counts[(color, Number.FOUR)], 2)
            self.assertEqual(card_counts[(color, Number.FIVE)], 1)

    def test_deck_shuffled_with_seed(self):
        """Test that decks with the same seed are shuffled identically"""
        deck1 = Deck(seed=42)
        deck2 = Deck(seed=42)

        self.assertEqual(deck1.cards, deck2.cards)

    def test_deck_shuffled_different_seeds(self):
        """Test that decks with different seeds are shuffled differently"""
        deck1 = Deck(seed=42)
        deck2 = Deck(seed=123)

        self.assertNotEqual(deck1.cards, deck2.cards)

    def test_deck_shuffled_no_seed(self):
        """Test that decks without seed are shuffled randomly"""
        deck1 = Deck()
        deck2 = Deck()

        # Very unlikely to be the same with random shuffling
        self.assertNotEqual(deck1.cards, deck2.cards)

    def test_deck_draw(self):
        """Test drawing cards from the deck"""
        deck = Deck(seed=42)
        initial_size = len(deck)

        card = deck.draw()
        self.assertIsNotNone(card)
        self.assertIsInstance(card, Card)
        self.assertEqual(len(deck), initial_size - 1)

    def test_deck_draw_all_cards(self):
        """Test drawing all cards from the deck"""
        deck = Deck(seed=42)

        cards_drawn = []
        while len(deck) > 0:
            card = deck.draw()
            self.assertIsNotNone(card)
            cards_drawn.append(card)

        self.assertEqual(len(cards_drawn), 50)
        self.assertEqual(len(deck), 0)

    def test_deck_draw_when_empty(self):
        """Test that drawing from an empty deck returns None"""
        deck = Deck(seed=42)

        # Draw all cards
        while len(deck) > 0:
            deck.draw()

        # Try to draw from empty deck
        card = deck.draw()
        self.assertIsNone(card)
        self.assertEqual(len(deck), 0)

    def test_deck_len(self):
        """Test that __len__ returns correct count"""
        deck = Deck(seed=42)
        self.assertEqual(len(deck), 50)

        deck.draw()
        self.assertEqual(len(deck), 49)

        for _ in range(10):
            deck.draw()
        self.assertEqual(len(deck), 39)


if __name__ == "__main__":
    unittest.main()
