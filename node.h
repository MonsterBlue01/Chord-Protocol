#ifndef NODE_H
#define NODE_H

#include <stdint.h>
#include <map>
#include <set>
#include <vector>
#include <cstddef>
#include <iostream>
#include <optional>

#define BITLENGTH 8

// Forward declaration
class Node;

class FingerTable {
public:
    /**
     * @param nodeId: the id of node hosting the finger table.
     */
    FingerTable(uint8_t nodeId);
    
    void set(size_t index, Node* successor) {
        fingerTable_[index] = successor;
    }
    Node* get(size_t index) {
        return fingerTable_[index];
    }
    void prettyPrint();
    
private:
    uint8_t nodeId_;
    std::vector<Node*> fingerTable_;
};

class Node {
public:
    Node(uint8_t id): id_(id), fingerTable_(id), predecessor_(nullptr) {}

    void join(Node* node);
    uint8_t find(uint8_t key);
    void insert(uint8_t key);                      // 如果只给 key，则 value 默认为 key
    void insert(uint8_t key, uint8_t value);         // 给定 key 和 value
    void remove(uint8_t key);

    Node* findSuccessor(uint8_t key);
    Node* closestPrecedingFinger(uint8_t key);
    Node* findPredecessor(uint8_t key);

    void updateOthers();
    void updateFingerTable(Node* s, int i);

    static bool inInterval(uint8_t key, uint8_t start, uint8_t end, bool inclusiveEnd);

    // 新增：打印本节点存储的键值对
    void printKeys();
    
    // 已经实现的打印 finger table
    void printFingerTable();

    friend class FingerTable;

private:
    uint64_t id_;
    FingerTable fingerTable_;
    std::map<uint8_t, std::optional<uint8_t>> localKeys_;
    Node* predecessor_;
};
    

#endif
