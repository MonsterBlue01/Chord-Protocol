#include "node.h"

int main() {
    Node n0(5);
    n0.join(nullptr);

    Node n1(63);
    n1.join(&n0);

    n0.find(63);

    return 0;
}
