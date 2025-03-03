#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include "RangeTree.h"

// Function to test the range tree with timings
void testRangeTree(std::ofstream& output_file) {
    output_file << "Range Tree Test Results" << std::endl;
    output_file << "=======================" << std::endl << std::endl;
    
    // Create sample 2D points
    std::vector<std::vector<int>> points_2d = {
        {3, 6}, {17, 15}, {13, 15}, {6, 12}, {9, 1}, {2, 7}, {10, 19},
        {14, 11}, {8, 7}, {15, 5}, {4, 8}, {11, 9}, {16, 3}, {5, 14},
        {12, 2}, {7, 10}, {18, 13}, {1, 4}, {19, 16}, {20, 18}
    };
    
    output_file << "Building 2D Range Tree with " << points_2d.size() << " points..." << std::endl;
    
    // Time the construction of the 2D range tree
    auto start = std::chrono::high_resolution_clock::now();
    RangeTree<int, 2> tree_2d(points_2d);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> elapsed = end - start;
    output_file << "Construction time: " << elapsed.count() << " ms" << std::endl << std::endl;
    
    // Test 2D range queries
    output_file << "2D Range Queries:" << std::endl;
    output_file << "----------------" << std::endl;
    
    // Define some test ranges
    std::vector<std::pair<std::vector<int>, std::vector<int>>> test_ranges = {
        {{5, 5}, {15, 15}},  // Mid-range query
        {{0, 0}, {20, 20}},  // Full range query
        {{3, 6}, {3, 6}},    // Single point query
        {{4, 4}, {5, 5}},    // Small range query
        {{18, 18}, {25, 25}} // Range with few points
    };
    
    // Perform the range queries
    for (const auto& range : test_ranges) {
        const auto& low = range.first;
        const auto& high = range.second;
        
        output_file << "Query range: [(" << low[0] << "," << low[1] << "), (" 
                  << high[0] << "," << high[1] << ")]" << std::endl;
        
        start = std::chrono::high_resolution_clock::now();
        auto results = tree_2d.rangeSearch(low, high);
        end = std::chrono::high_resolution_clock::now();
        
        elapsed = end - start;
        
        output_file << "Found " << results.size() << " points in " << elapsed.count() << " ms:" << std::endl;
        for (const auto& point : results) {
            output_file << "  (" << point[0] << "," << point[1] << ")" << std::endl;
        }
        output_file << std::endl;
    }
    
    // Test point search
    output_file << "Point Search Tests:" << std::endl;
    output_file << "-----------------" << std::endl;
    
    std::vector<std::vector<int>> test_points = {
        {3, 6},   // Existing point
        {7, 8},   // Non-existing point
        {17, 15}, // Existing point
        {100, 100} // Far outside range
    };
    
    for (const auto& point : test_points) {
        output_file << "Searching for point (" << point[0] << "," << point[1] << "): ";
        
        start = std::chrono::high_resolution_clock::now();
        bool found = tree_2d.search(point);
        end = std::chrono::high_resolution_clock::now();
        
        elapsed = end - start;
        
        output_file << (found ? "Found" : "Not Found") << " in " << elapsed.count() << " ms" << std::endl;
    }
    output_file << std::endl;
    
    // Test with 3D points
    output_file << "3D Range Tree Test:" << std::endl;
    output_file << "----------------" << std::endl;
    
    std::vector<std::vector<int>> points_3d = {
        {3, 6, 2}, {17, 15, 9}, {13, 15, 5}, {6, 12, 1}, {9, 1, 7},
        {2, 7, 3}, {10, 19, 8}, {14, 11, 4}, {8, 7, 6}, {15, 5, 10}
    };
    
    output_file << "Building 3D Range Tree with " << points_3d.size() << " points..." << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    RangeTree<int, 3> tree_3d(points_3d);
    end = std::chrono::high_resolution_clock::now();
    
    elapsed = end - start;
    output_file << "Construction time: " << elapsed.count() << " ms" << std::endl << std::endl;
    
    // Test 3D range query
    std::vector<int> low_3d = {5, 5, 3};
    std::vector<int> high_3d = {15, 15, 8};
    
    output_file << "3D Query range: [(" << low_3d[0] << "," << low_3d[1] << "," << low_3d[2] << "), (" 
              << high_3d[0] << "," << high_3d[1] << "," << high_3d[2] << ")]" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    auto results_3d = tree_3d.rangeSearch(low_3d, high_3d);
    end = std::chrono::high_resolution_clock::now();
    
    elapsed = end - start;
    
    output_file << "Found " << results_3d.size() << " points in " << elapsed.count() << " ms:" << std::endl;
    for (const auto& point : results_3d) {
        output_file << "  (" << point[0] << "," << point[1] << "," << point[2] << ")" << std::endl;
    }
    output_file << std::endl;
    
    output_file << "Range Tree Test Complete" << std::endl;
}

int main() {
    // Open output file
    std::ofstream output_file("range_tree_results.txt");
    
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open output file." << std::endl;
        return 1;
    }
    
    // Run tests
    testRangeTree(output_file);
    
    // Close file
    output_file.close();
    
    std::cout << "Range Tree tests completed. Results written to range_tree_results.txt" << std::endl;
    
    return 0;
}