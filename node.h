#ifndef NODE_H
#define NODE_H

#include <stdint.h>
#include <map>
#include <set>
#include <vector>
#include <cstddef>

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
    Node(uint8_t id): id_(id), fingerTable_(id) {}
    void join(Node* node);
    uint8_t find(uint8_t key);
    void insert(uint8_t key);
    void remove(uint8_t key);

    friend class FingerTable;
    
private:
    uint64_t id_;
    FingerTable fingerTable_;
    std::map<uint8_t, uint8_t> localKeys_;
};

#endif
