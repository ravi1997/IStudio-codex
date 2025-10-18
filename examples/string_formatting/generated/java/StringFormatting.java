package istudio.generated;

public final class StringFormatting {
  private StringFormatting() {}

  public static String greet(String name) {
    return "Hello, " + name;
  }

  public static String decorated(String name, String punctuation) {
    final String base = greet(name);
    return base + punctuation;
  }
}
