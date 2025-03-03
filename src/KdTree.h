#pragma once

#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <memory>
#include <stdexcept>

template<typename T, size_t K>
class KdTree {
private:
    struct Node {
        std::vector<T> point;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;

        Node(const std::vector<T>& pt) : point(pt), left(nullptr), right(nullptr) {}
    };

    std::unique_ptr<Node> root;
    size_t dimensions;

    std::unique_ptr<Node> insertRec(std::unique_ptr<Node> node, const std::vector<T>& point, int depth);
    bool searchRec(const Node* node, const std::vector<T>& point, int depth) const;
    void nearestNeighborRec(const Node* node, const std::vector<T>& query, int depth, 
                            std::vector<T>& best, T& bestDist) const;
    void rangeSearchRec(const Node* node, const std::vector<T>& min, const std::vector<T>& max, 
                         int depth, std::vector<std::vector<T>>& result) const;
    T distance(const std::vector<T>& a, const std::vector<T>& b) const;

public:
    KdTree() : root(nullptr), dimensions(K) {}

    void insert(const std::vector<T>& point);
    bool search(const std::vector<T>& point) const;
    std::vector<T> nearestNeighbor(const std::vector<T>& query) const;
    std::vector<std::vector<T>> rangeSearch(const std::vector<T>& min, const std::vector<T>& max) const;
};

// Implementation

template<typename T, size_t K>
void KdTree<T, K>::insert(const std::vector<T>& point) {
    if (point.size() != dimensions) {
        throw std::invalid_argument("Point dimension does not match tree dimension");
    }
    root = insertRec(std::move(root), point, 0);
}

template<typename T, size_t K>
std::unique_ptr<typename KdTree<T, K>::Node> KdTree<T, K>::insertRec(
    std::unique_ptr<Node> node, const std::vector<T>& point, int depth) {
    
    if (node == nullptr) {
        return std::make_unique<Node>(point);
    }

    // Calculate current dimension
    int cd = depth % dimensions;

    if (point[cd] < node->point[cd]) {
        node->left = insertRec(std::move(node->left), point, depth + 1);
    } else {
        node->right = insertRec(std::move(node->right), point, depth + 1);
    }

    return node;
}

template<typename T, size_t K>
bool KdTree<T, K>::search(const std::vector<T>& point) const {
    if (point.size() != dimensions) {
        throw std::invalid_argument("Point dimension does not match tree dimension");
    }
    return searchRec(root.get(), point, 0);
}

template<typename T, size_t K>
bool KdTree<T, K>::searchRec(const Node* node, const std::vector<T>& point, int depth) const {
    if (node == nullptr) {
        return false;
    }

    // Check if current node contains the point
    bool equal = true;
    for (size_t i = 0; i < dimensions; ++i) {
        if (node->point[i] != point[i]) {
            equal = false;
            break;
        }
    }
    
    if (equal) {
        return true;
    }

    // Calculate current dimension
    int cd = depth % dimensions;

    if (point[cd] < node->point[cd]) {
        return searchRec(node->left.get(), point, depth + 1);
    } else {
        return searchRec(node->right.get(), point, depth + 1);
    }
}

template<typename T, size_t K>
T KdTree<T, K>::distance(const std::vector<T>& a, const std::vector<T>& b) const {
    T sum = 0;
    for (size_t i = 0; i < dimensions; ++i) {
        T diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

template<typename T, size_t K>
std::vector<T> KdTree<T, K>::nearestNeighbor(const std::vector<T>& query) const {
    if (query.size() != dimensions) {
        throw std::invalid_argument("Query dimension does not match tree dimension");
    }
    
    if (root == nullptr) {
        throw std::runtime_error("Tree is empty");
    }

    std::vector<T> best = root->point;
    T bestDist = distance(query, best);
    
    nearestNeighborRec(root.get(), query, 0, best, bestDist);
    
    return best;
}

template<typename T, size_t K>
void KdTree<T, K>::nearestNeighborRec(const Node* node, const std::vector<T>& query, 
                                     int depth, std::vector<T>& best, T& bestDist) const {
    if (node == nullptr) {
        return;
    }

    // Calculate current dimension
    int cd = depth % dimensions;

    // Calculate distance to current node
    T dist = distance(query, node->point);
    
    // Update best if current node is closer
    if (dist < bestDist) {
        bestDist = dist;
        best = node->point;
    }

    // Determine which subtree to search first
    std::unique_ptr<Node> const* firstBranch;
    std::unique_ptr<Node> const* secondBranch;
    
    if (query[cd] < node->point[cd]) {
        firstBranch = &node->left;
        secondBranch = &node->right;
    } else {
        firstBranch = &node->right;
        secondBranch = &node->left;
    }

    // Search the first branch
    nearestNeighborRec(firstBranch->get(), query, depth + 1, best, bestDist);

    // Calculate distance to splitting plane
    T planeDist = std::abs(query[cd] - node->point[cd]);
    
    // Search the second branch if it could contain a closer point
    if (planeDist < bestDist) {
        nearestNeighborRec(secondBranch->get(), query, depth + 1, best, bestDist);
    }
}

template<typename T, size_t K>
std::vector<std::vector<T>> KdTree<T, K>::rangeSearch(
    const std::vector<T>& min, const std::vector<T>& max) const {
    
    if (min.size() != dimensions || max.size() != dimensions) {
        throw std::invalid_argument("Range dimensions do not match tree dimension");
    }
    
    std::vector<std::vector<T>> result;
    rangeSearchRec(root.get(), min, max, 0, result);
    return result;
}

template<typename T, size_t K>
void KdTree<T, K>::rangeSearchRec(const Node* node, const std::vector<T>& min, 
                                const std::vector<T>& max, int depth, 
                                std::vector<std::vector<T>>& result) const {
    if (node == nullptr) {
        return;
    }

    // Check if current node is within range
    bool inRange = true;
    for (size_t i = 0; i < dimensions; ++i) {
        if (node->point[i] < min[i] || node->point[i] > max[i]) {
            inRange = false;
            break;
        }
    }
    
    if (inRange) {
        result.push_back(node->point);
    }

    // Calculate current dimension
    int cd = depth % dimensions;

    // Check if left subtree could contain points in range
    if (node->left && min[cd] <= node->point[cd]) {
        rangeSearchRec(node->left.get(), min, max, depth + 1, result);
    }

    // Check if right subtree could contain points in range
    if (node->right && node->point[cd] <= max[cd]) {
        rangeSearchRec(node->right.get(), min, max, depth + 1, result);
    }
}