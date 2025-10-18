from dataclasses import dataclass
from typing import Generic, TypeVar

T = TypeVar("T")


@dataclass(frozen=True)
class Pair(Generic[T]):
    first: T
    second: T


def make_pair(first: T, second: T) -> "Pair[T]":
    """Construct a Pair from the provided values."""
    return Pair(first=first, second=second)


def swap(input: "Pair[T]") -> "Pair[T]":
    """Return a new Pair with elements flipped."""
    return Pair(first=input.second, second=input.first)
