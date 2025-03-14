#include "node.h"
#include <optional>

int main() {
    Node n0(0);
    Node n1(30);
    Node n2(65);
    Node n3(110);
    Node n4(160);
    Node n5(230);

    n0.join(nullptr); 
    n1.join(&n0);
    n2.join(&n1);
    n3.join(&n2);
    n4.join(&n3);
    n5.join(&n4);

    std::cout << "\nFinal finger tables after initial join:\n";
    n0.printFingerTable();
    n1.printFingerTable();
    n2.printFingerTable();
    n3.printFingerTable();
    n4.printFingerTable();
    n5.printFingerTable();

    n0.insert(3, 3);
    n1.insert(200);
    n2.insert(123);
    n3.insert(45, 3);
    n4.insert(99);
    n2.insert(60, 10);
    n0.insert(50, 8);
    n3.insert(100, 5);
    n3.insert(101, 4);
    n3.insert(102, 6);
    n5.insert(240, 8);
    n5.insert(250, 10);

    std::cout << "\nFinal keys stored in each node (before n2 leaves):\n";
    n0.printKeys();
    n1.printKeys();
    n2.printKeys();
    n3.printKeys();
    n4.printKeys();
    n5.printKeys();

    Node n6(100);
    n6.join(&n0); 

    std::vector<uint8_t> keys = {3, 200, 123, 45, 99, 60, 50, 100, 101, 102, 240, 250};
    Node* lookupNodes[] = { &n0, &n2, &n6 };

    for (Node* lookupNode : lookupNodes) {
        std::cout << "\n--------------------------------------\n";
        std::cout << "Lookup initiated at node " << (int)lookupNode->find(3) << "\n";
        for (uint8_t k : keys) {
            std::vector<uint8_t> path;
            std::optional<uint8_t> value;

            uint8_t res = lookupNode->iterativeLookup(k, path, value);
        
            std::cout << "Look-up result of key " << (int)k 
                      << " from node " << (int)lookupNode->getId()
                      << " with path [ ";
            for (auto nodeID : path) {
                std::cout << (int)nodeID << " ";
            }
            std::cout << "] value is ";
        
            if (value.has_value()) {
                std::cout << (int)value.value();
            } else {
                std::cout << "None";
            }
            std::cout << "\n";
        }
    }

    std::cout << "\nNow let node 65 (n2) leave the network...\n";
    n2.leave();

    std::cout << "\nFinger table for n0 after n2 leaves:\n";
    n0.printFingerTable();

    std::cout << "\nFinger table for n1 after n2 leaves:\n";
    n1.printFingerTable();

    std::cout << "\nKey distribution after n2 leaves:\n";
    n0.printKeys();
    n1.printKeys();
    n3.printKeys();
    n4.printKeys();
    n5.printKeys();
    n6.printKeys();

    return 0;
}
