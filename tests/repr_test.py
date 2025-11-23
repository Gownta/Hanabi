"""Unit tests for repr.py"""

import unittest
from collections import Counter
from src.repr import Color, Number, Card, Hand, Deck, GameState


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


class TestGameState(unittest.TestCase):
    """Test the GameState class"""

    def test_game_creation_2_players(self):
        """Test creating a game with 2 players"""
        game = GameState(num_players=2, seed=42)
        self.assertEqual(game.num_players, 2)
        self.assertEqual(len(game.hands), 2)
        self.assertEqual(len(game.hands[0].cards), 5)
        self.assertEqual(len(game.hands[1].cards), 5)
        self.assertEqual(len(game.deck), 40)  # 50 - 10 dealt

    def test_game_creation_3_players(self):
        """Test creating a game with 3 players"""
        game = GameState(num_players=3, seed=42)
        self.assertEqual(game.num_players, 3)
        self.assertEqual(len(game.hands), 3)
        for hand in game.hands:
            self.assertEqual(len(hand.cards), 5)
        self.assertEqual(len(game.deck), 35)  # 50 - 15 dealt

    def test_game_creation_4_players(self):
        """Test creating a game with 4 players"""
        game = GameState(num_players=4, seed=42)
        self.assertEqual(game.num_players, 4)
        self.assertEqual(len(game.hands), 4)
        for hand in game.hands:
            self.assertEqual(len(hand.cards), 4)
        self.assertEqual(len(game.deck), 34)  # 50 - 16 dealt

    def test_game_creation_5_players(self):
        """Test creating a game with 5 players"""
        game = GameState(num_players=5, seed=42)
        self.assertEqual(game.num_players, 5)
        self.assertEqual(len(game.hands), 5)
        for hand in game.hands:
            self.assertEqual(len(hand.cards), 4)
        self.assertEqual(len(game.deck), 30)  # 50 - 20 dealt

    def test_game_creation_invalid_players(self):
        """Test that invalid player counts raise ValueError"""
        with self.assertRaises(ValueError):
            GameState(num_players=1)
        with self.assertRaises(ValueError):
            GameState(num_players=6)
        with self.assertRaises(ValueError):
            GameState(num_players=0)

    def test_initial_state(self):
        """Test initial game state values"""
        game = GameState(num_players=3, seed=42)
        self.assertEqual(game.hints, 8)
        self.assertEqual(game.lives, 3)
        self.assertEqual(game.piles, [0, 0, 0, 0, 0])
        self.assertEqual(game.discard_pile, [])
        self.assertEqual(game.turns_left, -1)

    def test_play_successful(self):
        """Test successfully playing a card"""
        game = GameState(num_players=2, seed=42)
        card = Card(Color.RED, Number.ONE)

        result = game.play(card)
        self.assertTrue(result)
        self.assertEqual(game.piles[Color.RED], 1)
        self.assertEqual(len(game.discard_pile), 0)
        self.assertEqual(game.lives, 3)

    def test_play_sequence(self):
        """Test playing a sequence of cards"""
        game = GameState(num_players=2, seed=42)

        # Play RED 1, 2, 3
        self.assertTrue(game.play(Card(Color.RED, Number.ONE)))
        self.assertEqual(game.piles[Color.RED], 1)

        self.assertTrue(game.play(Card(Color.RED, Number.TWO)))
        self.assertEqual(game.piles[Color.RED], 2)

        self.assertTrue(game.play(Card(Color.RED, Number.THREE)))
        self.assertEqual(game.piles[Color.RED], 3)

    def test_play_wrong_card(self):
        """Test playing a card that doesn't match"""
        game = GameState(num_players=2, seed=42)

        # Try to play RED 2 when pile is at 0
        result = game.play(Card(Color.RED, Number.TWO))
        self.assertFalse(result)
        self.assertEqual(game.piles[Color.RED], 0)
        self.assertEqual(len(game.discard_pile), 1)
        self.assertEqual(game.discard_pile[0], Card(Color.RED, Number.TWO))
        self.assertEqual(game.lives, 2)

    def test_play_five_gives_hint(self):
        """Test that playing a 5 gives a hint"""
        game = GameState(num_players=2, seed=42)

        # Play RED 1-5
        game.play(Card(Color.RED, Number.ONE))
        game.play(Card(Color.RED, Number.TWO))
        game.play(Card(Color.RED, Number.THREE))
        game.play(Card(Color.RED, Number.FOUR))

        self.assertEqual(game.hints, 8)
        game.play(Card(Color.RED, Number.FIVE))
        self.assertEqual(game.hints, 8)  # Already at max, stays at 8

    def test_play_five_gives_hint_when_not_max(self):
        """Test that playing a 5 increases hints when not at max"""
        game = GameState(num_players=2, seed=42)
        game.hints = 5

        # Play RED 1-5
        game.play(Card(Color.RED, Number.ONE))
        game.play(Card(Color.RED, Number.TWO))
        game.play(Card(Color.RED, Number.THREE))
        game.play(Card(Color.RED, Number.FOUR))
        game.play(Card(Color.RED, Number.FIVE))

        self.assertEqual(game.hints, 6)

    def test_lives_and_game_over(self):
        """Test that losing all lives ends the game"""
        game = GameState(num_players=2, seed=42)

        self.assertFalse(game.game_over())
        self.assertEqual(game.turns_left, -1)

        # Lose first life
        game.play(Card(Color.RED, Number.TWO))
        self.assertEqual(game.lives, 2)
        self.assertFalse(game.game_over())

        # Lose second life
        game.play(Card(Color.RED, Number.THREE))
        self.assertEqual(game.lives, 1)
        self.assertFalse(game.game_over())

        # Lose third life - game over
        game.play(Card(Color.RED, Number.FOUR))
        self.assertEqual(game.lives, 0)
        self.assertEqual(game.turns_left, 0)
        self.assertTrue(game.game_over())

    def test_multiple_color_piles(self):
        """Test playing cards to different color piles"""
        game = GameState(num_players=2, seed=42)

        game.play(Card(Color.RED, Number.ONE))
        game.play(Card(Color.GREEN, Number.ONE))
        game.play(Card(Color.BLUE, Number.ONE))

        self.assertEqual(game.piles[Color.RED], 1)
        self.assertEqual(game.piles[Color.GREEN], 1)
        self.assertEqual(game.piles[Color.BLUE], 1)
        self.assertEqual(game.piles[Color.WHITE], 0)
        self.assertEqual(game.piles[Color.YELLOW], 0)


if __name__ == "__main__":
    unittest.main()
