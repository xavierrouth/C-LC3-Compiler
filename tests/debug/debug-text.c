int mul(int a, int b) {
    int counter = 0;
    int product = 0;
    while (b > counter) {
        counter = counter + 1;
        product = product + a;
    }
    return product;
}

int main() {
    int c = 0;
    c = mul(7, 11);
    return c;
}