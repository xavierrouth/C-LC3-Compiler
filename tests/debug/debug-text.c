int fib(int n) {
    if (2 > n) {
        return n;
    }
    int a;
    int b;
    a = fib(n-1);
    b = fib(n-2);
    return a + b;
}

int main() {
    return fib(5);
}