#include <iostream>
#include <vector>
#include <thread>
#include <sys/time.h>
#include "BS_thread_pool.hpp"

using namespace std;

struct testData {
    vector<vector<int>> A;
    vector<vector<int>> B;
    pair<int, int> aSize, bSize;
};

testData test1 = {
        {{8, 15, 24, 11, 40, 24, 18, 34, 1},{8, 15, 24, 11, 40, 24, 18, 34, 1}, {29, 33, 28, 10, 17, 10, 19, 14, 8}, {23, 28, 24, 40, 40, 18, 3, 15, 14}, {39, 19, 15, 28, 30, 12, 23, 2, 36}, {20, 4, 24, 21, 14, 34, 6, 17, 17}, {12, 35, 38, 15, 22, 27, 0, 1, 23}, {22, 40, 14, 26, 36, 27, 31, 0, 19}, {18, 33, 3, 13, 26, 24, 14, 38, 20}, {23, 25, 39, 3, 23, 24, 21, 20, 39}},
        {{21, 14, 38, 37, 40, 38, 24, 9, 13}, {31, 13, 10, 21, 5, 33, 36, 11, 30}, {11, 30, 9, 12, 21, 27, 27, 24, 10}, {14, 7, 38, 29, 6, 40, 13, 36, 26}, {19, 21, 10, 38, 25, 22, 7, 16, 39}, {21, 10, 1, 40, 24, 20, 29, 14, 10}, {31, 23, 30, 33, 36, 17, 19, 2, 27}, {6, 32, 7, 10, 36, 13, 23, 19, 23}, {37, 25, 27, 37, 19, 7, 9, 25, 7}},
        {10,         9},
        {9,         9}
};


testData currentTest = test1;
vector<vector<int>> result1(currentTest.aSize.first, vector<int>(currentTest.bSize.second));
vector<vector<int>> result2(currentTest.aSize.first, vector<int>(currentTest.bSize.second));
vector<vector<int>> result3(currentTest.aSize.first, vector<int>(currentTest.bSize.second));

int numThreads = 50;
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
    approach1();
//    printMat(result1);
}

void runSecond() {
    approach2();
//    printMat(result2);
}

void runThird() {
    approach3();
//    printMat(result3);
}

int main() {
    struct timeval start, end;
    long mtime, seconds, useconds;
    gettimeofday(&start, NULL);

    runFirst();
    runSecond();
    runThird();
    pool.wait_for_tasks();

    gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;

    mtime = (seconds) * 100 + useconds/100;

    cout << "First method, using threads:" << endl;
    printMat(result1);
    cout << endl;

    cout << "Second method, using threads:" << endl;
    printMat(result2);
    cout << endl;

    cout << "Third method, using threads:" << endl;
    printMat(result3);


    printf("Elapsed time: %ld milliseconds\n", mtime);
    return 0;
}
