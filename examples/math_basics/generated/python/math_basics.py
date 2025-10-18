def add(a: int, b: int) -> int:
    """Return the sum of two integers."""
    return a + b


def triple(value: int) -> int:
    """Return three times the provided value using add()."""
    doubled = add(value, value)
    return doubled + value
