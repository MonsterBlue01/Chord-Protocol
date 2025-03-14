#include "node.h"
#include <optional>

int main() {
    // 1. 先创建并加入原有的 6 个节点
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

    // 所有节点加入完成后，再统一打印各个节点的 finger table
    std::cout << "\nFinal finger tables:\n";
    n0.printFingerTable();
    n1.printFingerTable();
    n2.printFingerTable();
    n3.printFingerTable();
    n4.printFingerTable();
    n5.printFingerTable();

    // 测试插入操作（注意部分调用只给一个参数，默认 value = key）
    n0.insert(3, 3);
    n1.insert(200);      // 相当于 insert(200,200)
    n2.insert(123);      // insert(123,123)
    n3.insert(45, 3);
    n4.insert(99);       // insert(99,99)
    n2.insert(60, 10);
    n0.insert(50, 8);
    n3.insert(100, 5);
    n3.insert(101, 4);
    n3.insert(102, 6);
    n5.insert(240, 8);
    n5.insert(250, 10);

    // 插入操作过程中，若有 Key 迁移，会在 join() 时打印

    // 最后打印每个节点存储的键值对
    std::cout << "\nFinal keys stored in each node:\n";
    n0.printKeys();
    n1.printKeys();
    n2.printKeys();
    n3.printKeys();
    n4.printKeys();
    n5.printKeys();

    Node n6(100);
    n6.join(&n0); 

    // 4. 进行查找测试：对每个已插入 key，从节点 n0, n2, n6 分别进行查找
    std::vector<uint8_t> keys = {3, 200, 123, 45, 99, 60, 50, 100, 101, 102, 240, 250};
    Node* lookupNodes[] = { &n0, &n2, &n6 };

    for (Node* lookupNode : lookupNodes) {
        std::cout << "\n--------------------------------------\n";
        std::cout << "Lookup initiated at node " << (int)lookupNode->find(3) << "\n";
        for (uint8_t k : keys) {
            std::vector<uint8_t> path;
            std::optional<uint8_t> value;
            // 用迭代查找得到最终节点 ID, 并拿到其存储的 value（若有）
            uint8_t res = lookupNode->iterativeLookup(k, path, value);
        
            // 打印类似 “Look-up result of key 3 from node 0 with path [0, 30] value is 3”
            std::cout << "Look-up result of key " << (int)k 
                      << " from node " << (int)lookupNode->getId()  // 这里直接用 ID
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

    return 0;
}
