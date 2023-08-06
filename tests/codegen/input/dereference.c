int main() {
    int a = 15;
    int b = *(&a);
    int *c = &a;
    return *c + b;
}