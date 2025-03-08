#include "node.h"

FingerTable::FingerTable(uint8_t nodeId) : nodeId_(nodeId) {
    fingerTable_.resize(BITLENGTH + 1, nullptr);
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

bool Node::inInterval(uint8_t key, uint8_t start, uint8_t end, bool inclusiveEnd) {
    if (start < end) {
        return inclusiveEnd ? (key > start && key <= end) : (key > start && key < end);
    } else {
        return inclusiveEnd ? (key > start || key <= end) : (key > start || key < end);
    }
}

Node* Node::findSuccessor(uint8_t key) {
    if (inInterval(key, this->id_, fingerTable_.get(1)->id_, true)) {
        std::cout << "Visited Node " << (int)this->id_ << "\n";
        return fingerTable_.get(1);
    } else {
        Node* n0 = closestPrecedingFinger(key);
        std::cout << "Visited Node " << (int)this->id_ << "\n";
        return n0->findSuccessor(key);
    }
}

Node* Node::closestPrecedingFinger(uint8_t key) {
    for (int i = BITLENGTH; i >= 1; i--) {
        Node* finger = fingerTable_.get(i);
        if (finger != nullptr && inInterval(finger->id_, this->id_, key, false))
            return finger;
    }
    return this;
}

void Node::join(Node* node) {
    if (node == nullptr) {
        for (int i = 1; i <= BITLENGTH; i++) {
            fingerTable_.set(i, this);
        }
        predecessor_ = this;
        std::cout << "Node " << (int)id_ << " joined as the first node.\n";
    } else {
        std::cout << "Node " << (int)id_ << " is joining using Node " << (int)node->id_ << ".\n";
        Node* succ = node->findSuccessor(this->id_);
        fingerTable_.set(1, succ);
        
        predecessor_ = succ->predecessor_;
        succ->predecessor_ = this;
        
        for (int i = 1; i < BITLENGTH; i++) {
            uint8_t start = (this->id_ + (1 << i)) % 256;
            if (inInterval(start, this->id_, fingerTable_.get(i)->id_, false))
                fingerTable_.set(i+1, fingerTable_.get(i));
            else {
                Node* found = node->findSuccessor(start);
                fingerTable_.set(i+1, found);
            }
        }
        
        Node* successor = fingerTable_.get(1);
        std::vector<uint8_t> keysToMigrate;
        for (auto &pair : successor->localKeys_) {
            uint8_t key = pair.first;
            if (inInterval(key, predecessor_->id_, this->id_, true)) {
                keysToMigrate.push_back(key);
            }
        }
        for (auto key : keysToMigrate) {
            this->localKeys_[key] = successor->localKeys_[key];
            successor->localKeys_.erase(key);
            std::cout << "Migrated key " << (int)key << " to Node " << (int)this->id_ << "\n";
        }
    }
    fingerTable_.prettyPrint();
}

uint8_t Node::find(uint8_t key) {
    Node* n = findSuccessor(key);
    std::cout << "Key " << (int)key << " is located at Node " << (int)n->id_ << "\n";
    return n->id_;
}
