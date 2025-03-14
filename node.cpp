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
        // 第一个节点，所有 finger 指向自己
        for (int i = 1; i <= BITLENGTH; i++) {
            fingerTable_.set(i, this);
        }
        predecessor_ = this;
        std::cout << "Node " << (int)id_ << " joined as the first node.\n";
    } else {
        std::cout << "Node " << (int)id_ << " is joining using Node " << (int)node->id_ << ".\n";
        // 初始化 finger[1]
        Node* succ = node->findSuccessor((this->id_ + 1) % 256);
        fingerTable_.set(1, succ);

        // 设置前驱，并更新后继的前驱
        predecessor_ = succ->predecessor_;
        succ->predecessor_ = this;

        // 初始化 finger[2..m]
        for (int i = 1; i < BITLENGTH; i++) {
            uint8_t start = (this->id_ + (1 << i)) % 256;
            if (inInterval(start, this->id_, fingerTable_.get(i)->id_, false))
                fingerTable_.set(i+1, fingerTable_.get(i));
            else {
                Node* found = node->findSuccessor(start);
                fingerTable_.set(i+1, found);
            }
        }
        
        // 迁移键（与原来代码一致）
        Node* successor = fingerTable_.get(1);
        std::vector<uint8_t> keysToMigrate;
        for (auto &pair : successor->localKeys_) {
            uint8_t key = pair.first;
            if (inInterval(key, predecessor_->id_, this->id_, true)) {
                keysToMigrate.push_back(key);
            }
        }
        for (auto key : keysToMigrate) {
            // 记录旧拥有者的 ID
            uint8_t oldOwnerId = successor->id_;
            // 迁移键到当前节点
            this->localKeys_[key] = successor->localKeys_[key];
            successor->localKeys_.erase(key);
            std::cout << "migrate " << (int)key << " from node " << (int)oldOwnerId
                      << " to node " << (int)this->id_ << "\n";
        }
        // 新节点构造好自己后，让其它节点更新它们的 finger table
        updateOthers();
    }
    fingerTable_.prettyPrint();
}

uint8_t Node::find(uint8_t key) {
    Node* n = findSuccessor(key);
    std::cout << "Key " << (int)key << " is located at Node " << (int)n->id_ << "\n";
    return n->id_;
}

// 如果只给 key，则默认 value 等于 key
void Node::insert(uint8_t key) {
    Node* successor = findSuccessor(key);
    successor->localKeys_[key] = std::nullopt; // 表示无值
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
    // 当 key 不在 n 与 n 的后继之间时
    while (!inInterval(key, n->id_, n->fingerTable_.get(1)->id_, true)) {
        n = n->closestPrecedingFinger(key);
    }
    return n;
}

void Node::updateFingerTable(Node* s, int i) {
    // 如果 s 的 id 落在 (n, finger[i]) 之间，则更新 finger[i]
    // 注意：inInterval 中我们采用半开区间（不包含下界，但包含上界或根据需求调整）
    if (inInterval(s->id_, this->id_, fingerTable_.get(i)->id_, false)) {
        fingerTable_.set(i, s);
        // 递归更新前驱，除非前驱就是自己
        if (predecessor_ != s) {
            predecessor_->updateFingerTable(s, i);
        }
    }
}

void Node::updateOthers() {
    for (int i = 1; i <= BITLENGTH; i++) {
        // 计算 (n - 2^(i-1)) mod 2^m
        uint8_t pred_id = (id_ + 256 - (1 << (i-1))) % 256;
        // 找到这个 pred_id 所对应的前驱节点 p
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
    // 将当前节点加入路径
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
    // 将当前节点（查找发起者）加入路径
    path.push_back(this->id_);
    // 调用 findSuccessor(key) 得到正确的后继
    Node* succ = this->findSuccessor(key);
    // 将后继节点加入路径
    path.push_back(succ->id_);
    // 查找该后继节点是否存有 key
    auto it = succ->localKeys_.find(key);
    if (it != succ->localKeys_.end()) {
        outValue = it->second;
    } else {
        outValue = std::nullopt;
    }
    return succ->id_;
}


void Node::leave() {
    // 首先将本节点所有的 key 迁移到后继节点
    Node* succ = fingerTable_.get(1);
    for (auto &pair : localKeys_) {
        succ->localKeys_[pair.first] = pair.second;
        std::cout << "migrate " << (int)pair.first << " from node " << (int)this->id_
                  << " to node " << (int)succ->id_ << "\n";
    }
    localKeys_.clear();

    // 更新前驱和后继的指针
    // 让本节点的前驱的后继指向本节点的后继
    predecessor_->fingerTable_.set(1, succ);
    // 同时更新后继的前驱为本节点的前驱
    succ->predecessor_ = predecessor_;

    // 通知其它节点更新 finger table（简单调用 updateOthers() ）
    updateOthers();
    std::cout << "Node " << (int)this->id_ << " has left the network.\n";
}