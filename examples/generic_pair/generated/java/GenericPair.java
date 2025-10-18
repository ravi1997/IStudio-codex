package istudio.generated;

public final class GenericPair {
  private GenericPair() {}

  public static final class Pair<T> {
    public final T first;
    public final T second;

    public Pair(T first, T second) {
      this.first = first;
      this.second = second;
    }
  }

  public static <T> Pair<T> makePair(T first, T second) {
    return new Pair<>(first, second);
  }

  public static <T> Pair<T> swap(Pair<T> input) {
    return new Pair<>(input.second, input.first);
  }
}
