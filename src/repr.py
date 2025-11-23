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


class GameState:
    """
    Represents the state of a Hanabi game.

    Manages the deck, player hands, hints, lives, color piles, and discard pile.
    """

    def __init__(self, num_players: int, seed: Optional[int] = None):
        """
        Initialize a new Hanabi game.

        Args:
            num_players: Number of players (2-5)
            seed: Optional random seed for deck shuffling

        Raises:
            ValueError: If num_players is not between 2 and 5
        """
        if num_players < 2 or num_players > 5:
            raise ValueError("Number of players must be between 2 and 5")

        self.num_players = num_players
        self.deck = Deck(seed=seed)

        # Deal hands: 5 cards for 2-3 players, 4 cards for 4-5 players
        cards_per_hand = 5 if num_players <= 3 else 4
        self.hands: list[Hand] = []
        for _ in range(num_players):
            hand_cards = []
            for _ in range(cards_per_hand):
                card = self.deck.draw()
                if card:
                    hand_cards.append(card)
            self.hands.append(Hand(cards=hand_cards))

        self.hints = 8
        self.lives = 3

        # Color piles: maps color to the highest number played (0 = none played)
        self.piles: list[int] = [0] * 5

        self.discard_pile: list[Card] = []
        self.turns_left = -1

    def play(self, card: Card) -> bool:
        """
        Play a card to the color pile.

        Args:
            card: The card to play

        Returns:
            True if the card was successfully played, False otherwise
        """
        color_index = int(card.color)
        current_pile = self.piles[color_index]

        # Check if this is the next number in sequence
        if card.number == current_pile + 1:
            # Successfully played
            self.piles[color_index] += 1

            # If a 5 was played, gain a hint (max 8)
            if card.number == Number.FIVE:
                self.hints = min(self.hints + 1, 8)

            return True
        else:
            # Failed to play - discard and lose a life
            self.discard_pile.append(card)
            self.lives -= 1

            # If lives reach 0, game over
            if self.lives == 0:
                self.turns_left = 0

            return False

    def game_over(self) -> bool:
        """Check if the game is over"""
        return self.turns_left == 0
