#include <iostream>
#include <vector>
#include <thread>
#include "BS_thread_pool.hpp"

using namespace std;

struct testData {
    vector<vector<int>> A;
    vector<vector<int>> B;
    pair<int, int> aSize, bSize;
};

testData test1 = {
        {{1, 0, 1}, {2, 1, 1}, {0, 1, 1}, {1, 1, 2}},
        {{1, 2, 1}, {2, 3, 1}, {4, 2, 2}},
        {4,         3},
        {3,         3}
};

testData currentTest = test1;
vector<vector<int>> result1(currentTest.aSize.first, vector<int>(currentTest.bSize.second));
vector<vector<int>> result2(currentTest.aSize.first, vector<int>(currentTest.bSize.second));
vector<vector<int>> result3(currentTest.aSize.first, vector<int>(currentTest.bSize.second));

int numThreads = 1;
BS::thread_pool pool(numThreads);

int computeElement(pair<int, int> coords) {
    int res = 0;
    for (int i = 0; i < currentTest.aSize.second; i++) {
        res += currentTest.A[coords.first][i] * currentTest.B[i][coords.second];
    }
    return res;
}

int matToLin(pair<int, int> coords) {
    return (coords.first - 1) * currentTest.aSize.first + coords.second;
}

pair<int, int> linToMat(int pos) {
    return {pos / (currentTest.aSize.first - 1), pos % currentTest.bSize.second};
}

void printMat(const vector<vector<int>> &mat) {
    for (auto &v: mat) {
        for (int a: v) {
            cout << a << " ";
        }
        cout << endl;
    }
}

//Approach 1
void thread1(int start, int count) {
    pair<int, int> coords;
    for (int i = 0; i < count; i++) {
        coords = linToMat(start + i);
        result1[coords.first][coords.second] = computeElement(coords);
    }
}

void approach1() {
    int resultLinSize = currentTest.aSize.first * currentTest.bSize.second;
    int resPerThread = resultLinSize / numThreads;

    for (int i = 0; i < numThreads - 1; i++) {
        pool.submit(thread1, i * resPerThread, resPerThread);
    }
    pool.submit(thread1, (numThreads - 1) * resPerThread, resPerThread + resultLinSize % numThreads);
}

//Approach 2
pair<int, int> inverseLinToMat(int pos) {
    return {pos % currentTest.aSize.first, pos / currentTest.aSize.first};
}

void thread2(int start, int count) {
//    cout << start << endl;
    pair<int, int> coords;
    for (int i = 0; i < count; i++) {
        coords = inverseLinToMat(start + i);
//        cout << start << " " <<coords.first << " " << coords.second << " " << computeElement(coords) << endl;
        result2[coords.first][coords.second] = computeElement(coords);
    }
}

void approach2() {
    int resultLinSize = currentTest.aSize.first * currentTest.bSize.second;
    int resPerThread = resultLinSize / numThreads;

    for (int i = 0; i < numThreads - 1; i++) {
        pool.submit(thread2, i * resPerThread, resPerThread);
    }
    pool.submit(thread2, (numThreads - 1) * resPerThread, resPerThread + resultLinSize % numThreads);
}

//Approach 3
void thread3(int start, int count) {
    pair<int, int> coords;
    for (int i = start; i < count; i += numThreads) {
        coords = linToMat(i);
        result3[coords.first][coords.second] = computeElement(coords);
    }
}

void approach3() {
    int resultLinSize = currentTest.aSize.first * currentTest.bSize.second;
    for (int i = 0; i < numThreads - 1; i++) {
        pool.submit(thread3, i, resultLinSize);
    }
    pool.submit(thread3, numThreads - 1, resultLinSize);
}

void runFirst() {
//    cout << "First method, using threads:" << endl;
    approach1();
//    printMat(result1);
}

void runSecond() {
//    cout << "Second method, using threads:" << endl;
    approach2();
//    printMat(result2);
}

void runThird() {
//    cout << "Third method, using threads:" << endl;
    approach3();
//    printMat(result3);
}

int main() {
    runFirst();
    runSecond();
    runThird();
    pool.wait_for_tasks();
    printMat(result1);
    cout << endl;
    printMat(result2);
    cout << endl;
    printMat(result3);
    return 0;
}
