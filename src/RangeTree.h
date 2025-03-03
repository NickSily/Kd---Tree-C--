#pragma once

#include <vector>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <set>

template<typename T, size_t K>
class RangeTree {
private:
    // Primary node structure for the range tree
    struct Node {
        std::vector<T> point;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        std::unique_ptr<RangeTree<T, K-1>> next_level_tree; // For next dimension
        std::vector<std::vector<T>> canonical_subset; // Points in this subtree
        
        Node(const std::vector<T>& pt) : point(pt), left(nullptr), right(nullptr), next_level_tree(nullptr) {}
    };
    
    std::unique_ptr<Node> root;
    size_t dimension; // Current dimension this tree is sorted by
    
    // Helper methods
    std::unique_ptr<Node> buildTree(std::vector<std::vector<T>>& points, size_t begin, size_t end);
    void findSplitNode(const Node* node, T value, const Node** split_node) const;
    void collectPoints(const Node* node, std::vector<std::vector<T>>& result) const;
    void rangeSearchDim(const Node* node, const std::vector<T>& low, const std::vector<T>& high, 
                        std::vector<std::vector<T>>& result) const;
    bool isPointInRange(const std::vector<T>& point, const std::vector<T>& low, const std::vector<T>& high) const;
    
public:
    RangeTree(const std::vector<std::vector<T>>& points, size_t dim = 0);
    std::vector<std::vector<T>> rangeSearch(const std::vector<T>& low, const std::vector<T>& high) const;
    bool search(const std::vector<T>& point) const;
};

// Specialization for 1D Range Tree (base case for recursion)
template<typename T>
class RangeTree<T, 1> {
private:
    // Node structure for 1D range tree
    struct Node {
        std::vector<T> point;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        std::vector<std::vector<T>> canonical_subset;
        
        Node(const std::vector<T>& pt) : point(pt), left(nullptr), right(nullptr) {}
    };
    
    std::unique_ptr<Node> root;
    size_t dimension;
    
    // Helper methods
    std::unique_ptr<Node> buildTree(std::vector<std::vector<T>>& points, size_t begin, size_t end);
    void findSplitNode(const Node* node, T value, const Node** split_node) const;
    void collectPoints(const Node* node, std::vector<std::vector<T>>& result) const;
    void rangeSearchDim(const Node* node, const T& low, const T& high, 
                     std::vector<std::vector<T>>& result) const;
    bool isPointInRange(const std::vector<T>& point, const T& low, const T& high) const;
    
public:
    RangeTree(const std::vector<std::vector<T>>& points, size_t dim = 0);
    std::vector<std::vector<T>> rangeSearch(const T& low, const T& high) const;
    bool search(const std::vector<T>& point) const;
};

// Implementation for K-dimensional Range Tree

template<typename T, size_t K>
RangeTree<T, K>::RangeTree(const std::vector<std::vector<T>>& points, size_t dim) : dimension(dim) {
    if (points.empty()) return;
    
    // Validate input points
    for (const auto& point : points) {
        if (point.size() != K) {
            throw std::invalid_argument("Point dimension does not match tree dimension");
        }
    }
    
    // Make a copy of points for sorting
    std::vector<std::vector<T>> sorted_points = points;
    
    // Sort points by current dimension
    std::sort(sorted_points.begin(), sorted_points.end(), 
              [dim](const std::vector<T>& a, const std::vector<T>& b) {
                  return a[dim] < b[dim];
              });
    
    // Build the tree
    root = buildTree(sorted_points, 0, sorted_points.size());
}

template<typename T, size_t K>
std::unique_ptr<typename RangeTree<T, K>::Node> RangeTree<T, K>::buildTree(
    std::vector<std::vector<T>>& points, size_t begin, size_t end) {
    
    if (begin >= end) return nullptr;
    
    size_t mid = begin + (end - begin) / 2;
    auto node = std::make_unique<Node>(points[mid]);
    
    // Store canonical subset (all points in this subtree)
    for (size_t i = begin; i < end; ++i) {
        node->canonical_subset.push_back(points[i]);
    }
    
    // Build subtrees
    node->left = buildTree(points, begin, mid);
    node->right = buildTree(points, mid + 1, end);
    
    // Build next level tree for next dimension if not at last dimension
    if (dimension + 1 < K) {
        node->next_level_tree = std::make_unique<RangeTree<T, K-1>>(node->canonical_subset, dimension + 1);
    }
    
    return node;
}

template<typename T, size_t K>
void RangeTree<T, K>::findSplitNode(const Node* node, T value, const Node** split_node) const {
    if (!node) return;
    
    // If node's value matches search value exactly, this is the split node
    if (node->point[dimension] == value) {
        *split_node = node;
        return;
    }
    
    // Navigate to find the split node
    if (value < node->point[dimension]) {
        // Update split node candidate and go left
        *split_node = node;
        findSplitNode(node->left.get(), value, split_node);
    } else {
        // Go right
        findSplitNode(node->right.get(), value, split_node);
    }
}

template<typename T, size_t K>
void RangeTree<T, K>::collectPoints(const Node* node, std::vector<std::vector<T>>& result) const {
    if (!node) return;
    
    // Add all points from this node's canonical subset
    result.insert(result.end(), node->canonical_subset.begin(), node->canonical_subset.end());
}

template<typename T, size_t K>
bool RangeTree<T, K>::isPointInRange(
    const std::vector<T>& point, const std::vector<T>& low, const std::vector<T>& high) const {
    
    for (size_t i = 0; i < K; ++i) {
        if (point[i] < low[i] || point[i] > high[i]) {
            return false;
        }
    }
    return true;
}

template<typename T, size_t K>
void RangeTree<T, K>::rangeSearchDim(
    const Node* node, const std::vector<T>& low, const std::vector<T>& high, 
    std::vector<std::vector<T>>& result) const {
    
    if (!node) return;
    
    // If current node's value is outside the range in the current dimension
    if (node->point[dimension] < low[dimension]) {
        rangeSearchDim(node->right.get(), low, high, result);
        return;
    }
    
    if (node->point[dimension] > high[dimension]) {
        rangeSearchDim(node->left.get(), low, high, result);
        return;
    }
    
    // Check if the current point is in the range for all dimensions
    if (isPointInRange(node->point, low, high)) {
        result.push_back(node->point);
    }
    
    // Check both subtrees
    rangeSearchDim(node->left.get(), low, high, result);
    rangeSearchDim(node->right.get(), low, high, result);
}

template<typename T, size_t K>
std::vector<std::vector<T>> RangeTree<T, K>::rangeSearch(
    const std::vector<T>& low, const std::vector<T>& high) const {
    
    if (low.size() != K || high.size() != K) {
        throw std::invalid_argument("Range dimensions do not match tree dimension");
    }
    
    std::vector<std::vector<T>> result;
    if (!root) return result;
    
    // Find results for current dimension
    rangeSearchDim(root.get(), low, high, result);
    
    return result;
}

template<typename T, size_t K>
bool RangeTree<T, K>::search(const std::vector<T>& point) const {
    if (point.size() != K) {
        throw std::invalid_argument("Point dimension does not match tree dimension");
    }
    
    // Create a range query where low = high = point
    return !rangeSearch(point, point).empty();
}

// Implementation for 1D Range Tree

template<typename T>
RangeTree<T, 1>::RangeTree(const std::vector<std::vector<T>>& points, size_t dim) : dimension(dim) {
    if (points.empty()) return;
    
    // Validate input points
    for (const auto& point : points) {
        if (point.size() != 1) {
            throw std::invalid_argument("Point dimension does not match tree dimension");
        }
    }
    
    // Make a copy of points for sorting
    std::vector<std::vector<T>> sorted_points = points;
    
    // Sort points by the single dimension
    std::sort(sorted_points.begin(), sorted_points.end(), 
              [](const std::vector<T>& a, const std::vector<T>& b) {
                  return a[0] < b[0];
              });
    
    // Build the tree
    root = buildTree(sorted_points, 0, sorted_points.size());
}

template<typename T>
std::unique_ptr<typename RangeTree<T, 1>::Node> RangeTree<T, 1>::buildTree(
    std::vector<std::vector<T>>& points, size_t begin, size_t end) {
    
    if (begin >= end) return nullptr;
    
    size_t mid = begin + (end - begin) / 2;
    auto node = std::make_unique<Node>(points[mid]);
    
    // Store canonical subset (all points in this subtree)
    for (size_t i = begin; i < end; ++i) {
        node->canonical_subset.push_back(points[i]);
    }
    
    // Build subtrees
    node->left = buildTree(points, begin, mid);
    node->right = buildTree(points, mid + 1, end);
    
    return node;
}

template<typename T>
void RangeTree<T, 1>::findSplitNode(const Node* node, T value, const Node** split_node) const {
    if (!node) return;
    
    // If node's value matches search value exactly, this is the split node
    if (node->point[0] == value) {
        *split_node = node;
        return;
    }
    
    // Navigate to find the split node
    if (value < node->point[0]) {
        // Update split node candidate and go left
        *split_node = node;
        findSplitNode(node->left.get(), value, split_node);
    } else {
        // Go right
        findSplitNode(node->right.get(), value, split_node);
    }
}

template<typename T>
void RangeTree<T, 1>::collectPoints(const Node* node, std::vector<std::vector<T>>& result) const {
    if (!node) return;
    
    // Add all points from this node's canonical subset
    result.insert(result.end(), node->canonical_subset.begin(), node->canonical_subset.end());
}

template<typename T>
bool RangeTree<T, 1>::isPointInRange(const std::vector<T>& point, const T& low, const T& high) const {
    return point[0] >= low && point[0] <= high;
}

template<typename T>
void RangeTree<T, 1>::rangeSearchDim(
    const Node* node, const T& low, const T& high, 
    std::vector<std::vector<T>>& result) const {
    
    if (!node) return;
    
    // If current node's value is outside the range
    if (node->point[0] < low) {
        rangeSearchDim(node->right.get(), low, high, result);
        return;
    }
    
    if (node->point[0] > high) {
        rangeSearchDim(node->left.get(), low, high, result);
        return;
    }
    
    // Current node's point is in range
    result.push_back(node->point);
    
    // Check both subtrees
    rangeSearchDim(node->left.get(), low, high, result);
    rangeSearchDim(node->right.get(), low, high, result);
}

template<typename T>
std::vector<std::vector<T>> RangeTree<T, 1>::rangeSearch(const T& low, const T& high) const {
    std::vector<std::vector<T>> result;
    if (!root) return result;
    
    // Find results for 1D range
    rangeSearchDim(root.get(), low, high, result);
    
    return result;
}

template<typename T>
bool RangeTree<T, 1>::search(const std::vector<T>& point) const {
    if (point.size() != 1) {
        throw std::invalid_argument("Point dimension does not match tree dimension");
    }
    
    // Use range query with low = high = point's value
    return !rangeSearch(point[0], point[0]).empty();
}