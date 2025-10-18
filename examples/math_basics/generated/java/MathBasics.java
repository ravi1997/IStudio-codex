package istudio.generated;

public final class MathBasics {
  private MathBasics() {}

  public static int add(int a, int b) {
    return a + b;
  }

  public static int triple(int value) {
    final int doubled = add(value, value);
    return doubled + value;
  }
}
