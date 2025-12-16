int main() {
  int n = 10;
  int first = 0;
  int second = 1;
  int next = 0;
  int c = 0;

  // Calculate the nth Fibonacci number
  while (c < n) {
    if (c <= 1) {
      next = c;
    }
    if (c > 1) {
      next = first + second;
      first = second;
      second = next;
    }
    c = c + 1;
  }

  return next; // Should return 34 for n=10
}
