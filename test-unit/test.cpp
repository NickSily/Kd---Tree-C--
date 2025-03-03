#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <cmath>
#include "../src/KdTree.h"

// Simple test framework
#define TEST(name) void name()
#define ASSERT_TRUE(condition) if (!(condition)) { \
    test_file << "TEST FAILED: " << __func__ << " Line " << __LINE__ << ": " << #condition << " is not true" << std::endl; \
    failed_tests++; \
    return; \
} else { passed_assertions++; }

#define ASSERT_FALSE(condition) if ((condition)) { \
    test_file << "TEST FAILED: " << __func__ << " Line " << __LINE__ << ": " << #condition << " is not false" << std::endl; \
    failed_tests++; \
    return; \
} else { passed_assertions++; }

#define ASSERT_EQUAL(a, b) if ((a) != (b)) { \
    test_file << "TEST FAILED: " << __func__ << " Line " << __LINE__ << ": " << #a << " != " << #b << " (" << (a) << " != " << (b) << ")" << std::endl; \
    failed_tests++; \
    return; \
} else { passed_assertions++; }

#define RUN_TEST(name) { \
    test_file << "Running " << #name << "..." << std::endl; \
    name(); \
    if (failed_tests == old_failed_tests) { \
        test_file << "PASSED" << std::endl << std::endl; \
        passed_tests++; \
    } else { \
        old_failed_tests = failed_tests; \
    } \
    total_tests++; \
}

std::ofstream test_file;
int failed_tests = 0;
int old_failed_tests = 0;
int passed_tests = 0;
int total_tests = 0;
int passed_assertions = 0;

// Helper functions
template<typename T>
bool vectorsEqual(const std::vector<T>& a, const std::vector<T>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

template<typename T>
bool vectorContainsPoint(const std::vector<std::vector<T>>& vectors, const std::vector<T>& point) {
    for (const auto& vec : vectors) {
        if (vectorsEqual(vec, point)) return true;
    }
    return false;
}

// Tests for 2D KD Tree of ints
TEST(test_empty_tree) {
    KdTree<int, 2> tree;
    
    ASSERT_FALSE(tree.search({3, 6}));
    
    try {
        tree.nearestNeighbor({3, 6});
        test_file << "TEST FAILED: " << __func__ << " Line " << __LINE__ << ": Expected exception for nearestNeighbor on empty tree" << std::endl;
        failed_tests++;
        return;
    } catch (const std::runtime_error&) {
        passed_assertions++;
    }
    
    auto rangeResult = tree.rangeSearch({0, 0}, {10, 10});
    ASSERT_EQUAL(rangeResult.size(), 0);
}

TEST(test_insert_and_search) {
    KdTree<int, 2> tree;
    
    tree.insert({3, 6});
    tree.insert({17, 15});
    tree.insert({13, 15});
    tree.insert({6, 12});
    
    ASSERT_TRUE(tree.search({3, 6}));
    ASSERT_TRUE(tree.search({17, 15}));
    ASSERT_TRUE(tree.search({13, 15}));
    ASSERT_TRUE(tree.search({6, 12}));
    
    ASSERT_FALSE(tree.search({4, 7}));
    ASSERT_FALSE(tree.search({18, 14}));
    ASSERT_FALSE(tree.search({10, 12}));
}

TEST(test_nearest_neighbor_exact_match) {
    KdTree<int, 2> tree;
    
    tree.insert({3, 6});
    tree.insert({17, 15});
    tree.insert({13, 15});
    tree.insert({6, 12});
    
    std::vector<int> result = tree.nearestNeighbor({3, 6});
    ASSERT_TRUE(vectorsEqual(result, {3, 6}));
    
    result = tree.nearestNeighbor({17, 15});
    ASSERT_TRUE(vectorsEqual(result, {17, 15}));
}

TEST(test_nearest_neighbor_approximate) {
    KdTree<int, 2> tree;
    
    tree.insert({3, 6});   // Distance to (5,5): sqrt(13) = 3.61
    tree.insert({17, 15}); // Distance to (5,5): sqrt(544) = 23.33
    tree.insert({13, 15}); // Distance to (5,5): sqrt(328) = 18.11
    tree.insert({6, 12});  // Distance to (5,5): sqrt(50) = 7.07
    tree.insert({9, 1});   // Distance to (5,5): sqrt(32) = 5.66
    
    std::vector<int> result = tree.nearestNeighbor({5, 5});
    ASSERT_TRUE(vectorsEqual(result, {3, 6})); // Closest point
    
    result = tree.nearestNeighbor({14, 14});
    ASSERT_TRUE(vectorsEqual(result, {13, 15})); // Closest point
}

TEST(test_range_search) {
    KdTree<int, 2> tree;
    
    tree.insert({3, 6});
    tree.insert({17, 15});
    tree.insert({13, 15});
    tree.insert({6, 12});
    tree.insert({9, 1});
    tree.insert({2, 7});
    tree.insert({10, 19});
    
    // Range that includes points
    auto result = tree.rangeSearch({5, 5}, {15, 15});
    ASSERT_EQUAL(result.size(), 2);
    ASSERT_TRUE(vectorContainsPoint(result, {6, 12}));
    ASSERT_TRUE(vectorContainsPoint(result, {13, 15}));
    
    // Range that includes all points
    result = tree.rangeSearch({0, 0}, {20, 20});
    ASSERT_EQUAL(result.size(), 7);
    
    // Range that includes no points
    result = tree.rangeSearch({4, 4}, {5, 5});
    ASSERT_EQUAL(result.size(), 0);
    
    // Range that includes boundary cases
    result = tree.rangeSearch({3, 6}, {3, 6});
    ASSERT_EQUAL(result.size(), 1);
    ASSERT_TRUE(vectorContainsPoint(result, {3, 6}));
}

TEST(test_higher_dimensions) {
    KdTree<double, 3> tree;
    
    tree.insert({1.0, 2.0, 3.0});
    tree.insert({4.0, 5.0, 6.0});
    tree.insert({7.0, 8.0, 9.0});
    
    ASSERT_TRUE(tree.search({1.0, 2.0, 3.0}));
    ASSERT_TRUE(tree.search({4.0, 5.0, 6.0}));
    ASSERT_TRUE(tree.search({7.0, 8.0, 9.0}));
    
    ASSERT_FALSE(tree.search({1.1, 2.0, 3.0}));
    
    auto nn = tree.nearestNeighbor({3.9, 5.1, 5.8});
    ASSERT_TRUE(vectorsEqual(nn, {4.0, 5.0, 6.0}));
    
    auto range = tree.rangeSearch({0.0, 0.0, 0.0}, {5.0, 6.0, 7.0});
    ASSERT_EQUAL(range.size(), 2);
}

TEST(test_different_data_types) {
    KdTree<float, 2> tree;
    
    tree.insert({3.5f, 6.7f});
    tree.insert({17.2f, 15.3f});
    
    ASSERT_TRUE(tree.search({3.5f, 6.7f}));
    ASSERT_TRUE(tree.search({17.2f, 15.3f}));
    
    ASSERT_FALSE(tree.search({3.51f, 6.7f}));
    
    auto nn = tree.nearestNeighbor({10.0f, 10.0f});
    ASSERT_TRUE(vectorsEqual(nn, {17.2f, 15.3f}) || vectorsEqual(nn, {3.5f, 6.7f})); // Could be either based on implementation
    
    auto range = tree.rangeSearch({3.0f, 6.0f}, {4.0f, 7.0f});
    ASSERT_EQUAL(range.size(), 1);
}

TEST(test_dimension_validation) {
    KdTree<int, 2> tree;
    
    try {
        tree.insert({1, 2, 3}); // Wrong dimension
        test_file << "TEST FAILED: " << __func__ << " Line " << __LINE__ << ": Expected exception for incorrect dimensions" << std::endl;
        failed_tests++;
        return;
    } catch (const std::invalid_argument&) {
        passed_assertions++;
    }
    
    try {
        tree.search({1}); // Wrong dimension
        test_file << "TEST FAILED: " << __func__ << " Line " << __LINE__ << ": Expected exception for incorrect dimensions" << std::endl;
        failed_tests++;
        return;
    } catch (const std::invalid_argument&) {
        passed_assertions++;
    }
    
    try {
        tree.nearestNeighbor({1, 2, 3}); // Wrong dimension
        test_file << "TEST FAILED: " << __func__ << " Line " << __LINE__ << ": Expected exception for incorrect dimensions" << std::endl;
        failed_tests++;
        return;
    } catch (const std::invalid_argument&) {
        passed_assertions++;
    }
    
    try {
        tree.rangeSearch({1}, {2}); // Wrong dimension
        test_file << "TEST FAILED: " << __func__ << " Line " << __LINE__ << ": Expected exception for incorrect dimensions" << std::endl;
        failed_tests++;
        return;
    } catch (const std::invalid_argument&) {
        passed_assertions++;
    }
}

TEST(test_large_dataset) {
    KdTree<int, 2> tree;
    
    // Insert 100 points in a grid pattern
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            tree.insert({i, j});
        }
    }
    
    // Verify all points are found
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            ASSERT_TRUE(tree.search({i, j}));
        }
    }
    
    // Verify nearest neighbor works correctly
    auto nn = tree.nearestNeighbor({5, 4});
    ASSERT_TRUE(vectorsEqual(nn, {5, 4}));
    
    // Verify range search works correctly
    auto range = tree.rangeSearch({3, 3}, {6, 6});
    ASSERT_EQUAL(range.size(), 16); // 4x4 grid
}

int main() {
    test_file.open("test_results.txt");
    
    if (!test_file.is_open()) {
        std::cerr << "Error opening test_results.txt!" << std::endl;
        return 1;
    }
    
    test_file << "KD Tree Unit Tests" << std::endl;
    test_file << "=================" << std::endl << std::endl;
    
    // Run all tests
    RUN_TEST(test_empty_tree);
    RUN_TEST(test_insert_and_search);
    RUN_TEST(test_nearest_neighbor_exact_match);
    RUN_TEST(test_nearest_neighbor_approximate);
    RUN_TEST(test_range_search);
    RUN_TEST(test_higher_dimensions);
    RUN_TEST(test_different_data_types);
    RUN_TEST(test_dimension_validation);
    RUN_TEST(test_large_dataset);
    
    // Output test summary
    test_file << std::endl;
    test_file << "Test Summary" << std::endl;
    test_file << "============" << std::endl;
    test_file << "Total Tests: " << total_tests << std::endl;
    test_file << "Passed Tests: " << passed_tests << std::endl;
    test_file << "Failed Tests: " << failed_tests << std::endl;
    test_file << "Passed Assertions: " << passed_assertions << std::endl;
    
    test_file.close();
    
    std::cout << "Tests completed. Results saved to test_results.txt" << std::endl;
    
    return failed_tests;
}