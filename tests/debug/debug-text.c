int mul(int a, int b) {
    int product = 0;
    for (int i = 0; b > i; i++) {
        product = product + a;
    }
    return product;
}

int main() {
    int c = 0;
    c = mul(7, 12);
    return c;
}