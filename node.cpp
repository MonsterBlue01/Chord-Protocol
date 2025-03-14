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

        Node* succ = node->findSuccessor((this->id_ + 1) % 256);
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
            uint8_t oldOwnerId = successor->id_;
            this->localKeys_[key] = successor->localKeys_[key];
            successor->localKeys_.erase(key);
            std::cout << "migrate " << (int)key << " from node " << (int)oldOwnerId
                      << " to node " << (int)this->id_ << "\n";
        }
        updateOthers();
    }
    fingerTable_.prettyPrint();
}

uint8_t Node::find(uint8_t key) {
    Node* n = findSuccessor(key);
    std::cout << "Key " << (int)key << " is located at Node " << (int)n->id_ << "\n";
    return n->id_;
}

void Node::insert(uint8_t key) {
    Node* successor = findSuccessor(key);
    successor->localKeys_[key] = std::nullopt;
    std::cout << "Key " << (int)key 
              << " inserted with no value at Node " 
              << (int)successor->id_ << "\n";
}

void Node::insert(uint8_t key, uint8_t value) {
    Node* successor = findSuccessor(key);
    successor->localKeys_[key] = value;
    std::cout << "Key " << (int)key 
              << " inserted with value " << (int)value 
              << " at Node " << (int)successor->id_ << "\n";
}


Node* Node::findPredecessor(uint8_t key) {
    Node* n = this;
    while (!inInterval(key, n->id_, n->fingerTable_.get(1)->id_, true)) {
        n = n->closestPrecedingFinger(key);
    }
    return n;
}

void Node::updateFingerTable(Node* s, int i) {
    if (inInterval(s->id_, this->id_, fingerTable_.get(i)->id_, false)) {
        fingerTable_.set(i, s);
        if (predecessor_ != s) {
            predecessor_->updateFingerTable(s, i);
        }
    }
}

void Node::updateOthers() {
    for (int i = 1; i <= BITLENGTH; i++) {
        uint8_t pred_id = (id_ + 256 - (1 << (i-1))) % 256;
        Node* p = findPredecessor(pred_id);
        p->updateFingerTable(this, i);
    }
}

void Node::printFingerTable() {
    std::cout << "----------Node Id:" << (int)id_ << "-------------\n";
    std::cout << "Successor: " << (int)fingerTable_.get(1)->id_
              << "  Predecessor: " << (int)predecessor_->id_ << "\n";
    fingerTable_.prettyPrint();
    std::cout << "--------------------------------\n\n";
}

void Node::printKeys() {
    std::cout << "Keys stored in Node " << (int)id_ << ": ";
    for (auto &pair : localKeys_) {
        std::cout << "(" << (int)pair.first << ",";
        if (pair.second.has_value())
            std::cout << (int)pair.second.value();
        else
            std::cout << "None";
        std::cout << ") ";
    }
    std::cout << "\n";
}

Node* Node::findSuccessorPath(uint8_t key, std::vector<uint8_t>& path) {
    path.push_back(this->id_);
    if (inInterval(key, this->id_, fingerTable_.get(1)->id_, true)) {
        return fingerTable_.get(1);
    } else {
        Node* n0 = closestPrecedingFinger(key);
        return n0->findSuccessorPath(key, path);
    }
}

uint8_t Node::lookup(uint8_t key, std::vector<uint8_t>& path) {
    Node* result = findSuccessorPath(key, path);
    return result->id_;
}

uint8_t Node::iterativeLookup(uint8_t key, std::vector<uint8_t>& path, std::optional<uint8_t>& outValue) {
    path.push_back(this->id_);
    Node* succ = this->findSuccessor(key);
    path.push_back(succ->id_);
    auto it = succ->localKeys_.find(key);
    if (it != succ->localKeys_.end()) {
        outValue = it->second;
    } else {
        outValue = std::nullopt;
    }
    return succ->id_;
}


void Node::leave() {
    Node* succ = fingerTable_.get(1);
    for (auto &pair : localKeys_) {
        succ->localKeys_[pair.first] = pair.second;
        std::cout << "migrate " << (int)pair.first << " from node " << (int)this->id_
                  << " to node " << (int)succ->id_ << "\n";
    }
    localKeys_.clear();

    predecessor_->fingerTable_.set(1, succ);
    
    succ->predecessor_ = predecessor_;

    for (int i = 1; i <= BITLENGTH; i++) {
        uint8_t pred_id = (id_ + 256 - (1 << (i - 1))) % 256;
        Node* p = findPredecessor(pred_id);
        p->fixFingerTableOnLeave(this, succ, i);
    }
    
    std::cout << "Node " << (int)this->id_ << " has left the network.\n";
}

void Node::fixFingerTableOnLeave(Node* leaving, Node* succ, int i) {
    if (fingerTable_.get(i) == leaving) {
        fingerTable_.set(i, succ);
        
        if (predecessor_ != leaving) {
            predecessor_->fixFingerTableOnLeave(leaving, succ, i);
        }
    }
}