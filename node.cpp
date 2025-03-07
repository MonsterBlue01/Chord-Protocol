#include "node.h"
#include <iostream>

FingerTable::FingerTable(uint8_t nodeId) : nodeId_(nodeId) {
    fingerTable_.resize(BITLENGTH + 1);
}

void FingerTable::prettyPrint() {
    std::cout << "Finger Table for Node " << (int)nodeId_ << ":\n";
    for (size_t i = 1; i <= BITLENGTH; i++) {
        Node* successor = fingerTable_[i];
        if (successor != nullptr)
            std::cout << "Entry " << i << ": Node " << (int)successor->id_ << "\n";
        else
            std::cout << "Entry " << i << ": NULL\n";
    }
}

void Node::join(Node* node) {
    if (node == nullptr) {
        for (int i = 1; i <= BITLENGTH; i++) {
            fingerTable_.set(i, this);
        }
        std::cout << "Node " << (int)id_ << " joined as the first node.\n";
    } else {
        std::cout << "Node " << (int)id_ << " is joining using Node " << (int)node->id_ << ".\n";
    }
    fingerTable_.prettyPrint();
}
