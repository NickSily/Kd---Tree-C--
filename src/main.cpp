#include <iostream>
#include <vector>
#include "KdTree.h"

int main() {
    // Create a 2D KD-Tree for integers
    KdTree<int, 2> tree;
    
    // Insert some points
    tree.insert({3, 6});
    tree.insert({17, 15});
    tree.insert({13, 15});
    tree.insert({6, 12});
    tree.insert({9, 1});
    tree.insert({2, 7});
    tree.insert({10, 19});
    
    // Search for points
    std::cout << "Search for (3,6): " << (tree.search({3, 6}) ? "Found" : "Not Found") << std::endl;
    std::cout << "Search for (7,8): " << (tree.search({7, 8}) ? "Found" : "Not Found") << std::endl;
    
    // Find nearest neighbor
    std::vector<int> nn = tree.nearestNeighbor({7, 8});
    std::cout << "Nearest neighbor to (7,8): (" << nn[0] << "," << nn[1] << ")" << std::endl;
    
    // Range search
    std::vector<std::vector<int>> rangeResult = tree.rangeSearch({5, 5}, {15, 15});
    std::cout << "Points in range ([5,5], [15,15]):" << std::endl;
    for (const auto& point : rangeResult) {
        std::cout << "(" << point[0] << "," << point[1] << ")" << std::endl;
    }
    
    return 0;
}