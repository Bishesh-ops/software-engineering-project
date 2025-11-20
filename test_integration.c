// Integration Test - Complex C Program
// This tests lexer and parser working together

// Function declarations
int factorial(int n);
void printArray(int arr[], int size);

// Global variable declarations
int globalCounter = 0;
float PI = 3.14159;
char message[] = "Hello, World!";

// Struct definition
struct Point {
    int x;
    int y;
};

struct Node {
    int data;
    struct Node *next;
};

// Main function
int main() {
    int i;
    int n = 5;
    int result;

    // For loop
    for (i = 0; i < n; i = i + 1) {
        result = factorial(i);
    }

    // If-else statement
    if (result > 100) {
        result = 100;
    } else {
        result = result + 1;
    }

    // While loop
    while (result > 0) {
        result = result - 1;
        globalCounter = globalCounter + 1;
    }

    return 0;
}

// Function definition
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

// Array processing function
void printArray(int arr[], int size) {
    int i;
    for (i = 0; i < size; i = i + 1) {
        // Array access
        int value = arr[i];
    }
    return;
}

// Pointer manipulation
int* createArray(int size) {
    int *arr;
    int i;

    for (i = 0; i < size; i = i + 1) {
        arr[i] = i * 2;
    }

    return arr;
}

// Struct operations
struct Point createPoint(int x, int y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}
