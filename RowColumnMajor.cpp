// ============================================================
//  Cache-friendly code: row-major vs column-major traversal
//  Demonstrates the impact of cache misses on performance
//
//  Expected output (approximate):
//  - Row-major (cache-friendly): ~50 ms
//  - Column-major (cache-unfriendly): ~250 ms
// ============================================================

#include <iostream>
#include <chrono>

static const int MATRIX_SIZE = 4096;        // Matrix dimension (NxN)
static const int MEASUREMENT_COUNT = 5;     // Number of measurements

// A flat 2D array stored in row-major order
// Element [row][col] is accessed by: data[row * MATRIX_SIZE + col]
static double gMatrix[MATRIX_SIZE][MATRIX_SIZE];

// Prevents the compiler from optimizing away the sum result (since it's not actualy used in this example)
static volatile double gSink;

// High-resolution timer which returns elapsed time (milliseconds)
static double measureTime(auto matrixAccessFunc)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    matrixAccessFunc();
    auto endTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(endTime - startTime).count();
}

// Version A: Row-major traversal
// Walks through memory in the order it was stored: [0][0], [0][1], [0][2], ...
static void traverseRowMajor()
{
    double sum = 0.0;
    for (int row = 0; row < MATRIX_SIZE; ++row)
    {
        for (int col = 0; col < MATRIX_SIZE; ++col)
        {
            sum += gMatrix[row][col];
        }
    }

    gSink = sum;
}

// Version B: Column-major traversal
// Walks through memory in the wrong order: [0][0], [1][0], [2][0], ...
static void traverseColumnMajor()
{
    double sum = 0.0;
    for (int col = 0; col < MATRIX_SIZE; ++col)
    {
        for (int row = 0; row < MATRIX_SIZE; ++row)
        {
            sum += gMatrix[row][col];
        }
    }

    gSink = sum;
}

int main()
{
    // Fill the matrix with a known pattern
    for (int i = 0; i < MATRIX_SIZE; ++i)
        for (int j = 0; j < MATRIX_SIZE; ++j)
            gMatrix[i][j] = static_cast<double>(i + j);

    std::cout << "Matrix size : " << MATRIX_SIZE << " x " << MATRIX_SIZE
        << " (" << (MATRIX_SIZE * MATRIX_SIZE * sizeof(double)) / (1024 * 1024)
        << " MB)\n\n";

    // Row-major (cache-friendly)
    double rowTotal = 0.0;
    for (int run = 0; run < MEASUREMENT_COUNT; ++run)
    {
        rowTotal += measureTime(traverseRowMajor);
    }

    double rowAvg = rowTotal / MEASUREMENT_COUNT;
    std::cout << "Row-major (cache-friendly): "
        << rowAvg << " ms (average over " << MEASUREMENT_COUNT << " runs)\n";

    // Column-major (cache-unfriendly)
    double colTotal = 0.0;
    for (int run = 0; run < MEASUREMENT_COUNT; ++run)
    {
        colTotal += measureTime(traverseColumnMajor);
    }

    double colAvg = colTotal / MEASUREMENT_COUNT;
    std::cout << "Column-major (cache-unfriendly): "
        << colAvg << " ms (average over " << MEASUREMENT_COUNT << " runs)\n";

    // Speedup summary
    std::cout << "\nSpeedup: " << (colAvg / rowAvg) << "x faster\n";

    system("PAUSE");
    return 0;
}