

#include "common.h"

template <typename T> struct LinkedList {
    struct Node {
        T value;
        Node* next = nullptr;
    };

    Node* head = nullptr;
    Node* tail = nullptr;
    int _size = 0;

    // Pushes a value to the end of the list
    void push(T value) {
        Node* node = new Node();
        node->value = value;
        if (head == nullptr) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
        _size++;
    }

    // Pops the first value from the list
    T pop() {
        if (head == nullptr) return T();
        Node* node = head;
        head = head->next;
        T value = node->value;
        delete node;
        _size--;
        return value;
    }

    // Returns the first value from the list
    T peek(int depth = 0) {
        if (head == nullptr) return T();
        Node* node = head;
        for (int i = 0; i < depth; i++) {
            if (node->next == nullptr) return T();
            node = node->next;
        }
        return node->value;
    }

    // Returns the number of elements in the list
    int size() { return _size; }
};

LinkedList<int> integers;
LinkedList<char> characters;

int step = 0;

WASM_EXPORT void doSomething() {
    if (step == 0) {
        integers.push(1);
        integers.push(2);
        characters.push('a');
        characters.push('b');
    } else if (step == 1) {
        integers.pop();
        characters.pop();
        integers.pop();
        characters.pop();
    }
    step = (step + 1) % 2;
}