int fib(int n) {
    if (2 > n) {
        return n;
    }
    int a = fib(n-1);
    int b = fib(n-2);
    return a + b;
}

int main() {
    return fib(11);
}