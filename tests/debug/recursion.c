int rec(int n) {
    if (1 > n) { 
        return 0;
    }
    int a = rec(n-1);
    return a + 2;
}

int main() {
    return rec(4);
}