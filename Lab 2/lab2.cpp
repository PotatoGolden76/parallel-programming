#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

std::mutex g_mutex;
std::condition_variable g_cv;

bool g_ready = false;

std::vector<int> vector1 = {1, 2, 3, 4, 50};
std::vector<int> vector2 = {6, 7, 8, 9, 10};
std::vector<int> products(vector1.size());
int sum = 0;

void consumer() {
    for (size_t i = 0; i < vector1.size(); ++i) {
        std::unique_lock<std::mutex> ul(g_mutex);
        g_cv.wait(ul, []() { return g_ready; });
        sum += products[i];
        g_ready = false;
        ul.unlock();
        g_cv.notify_one();
    }
}

void producer() {
    for (size_t i = 0; i < vector1.size(); ++i) {
        std::unique_lock<std::mutex> ul(g_mutex);
        products[i] = vector1[i] * vector2[i];
        g_ready = true;
        ul.unlock();
        g_cv.notify_one();
        ul.lock();
        g_cv.wait(ul, []() { return !g_ready; });
    }
}

int main() {
    std::thread consumerThread(consumer);
    std::thread producerThread(producer);

    consumerThread.join();
    producerThread.join();

    std::cout << "Scalar product: " << sum << std::endl;

    return 0;
}
