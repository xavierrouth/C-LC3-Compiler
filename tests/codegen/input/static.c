int accumulate() {
    static int x = 4;
    x = x + 1;
    return x;
}

int main() {
    accumulate();
    accumulate();
    accumulate();
    accumulate();
    return accumulate();
}