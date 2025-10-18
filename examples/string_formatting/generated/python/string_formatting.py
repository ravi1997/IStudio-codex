def greet(name: str) -> str:
    """Return a friendly greeting for the given name."""
    return "Hello, " + name


def decorated(name: str, punctuation: str) -> str:
    """Return a greeting with custom punctuation."""
    base = greet(name)
    return base + punctuation
