void change_to_five(int* a) {
    *a = 5;
    return;
}


int main() {
    int b = 10;
    change_to_five(&b);
    return b;
}