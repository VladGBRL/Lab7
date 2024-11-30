#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
int current_type = -1; 
int active_threads = 0; 
std::queue<int> request_queue; 

void accessResource(int thread_id, int type) {
    std::unique_lock<std::mutex> lock(mtx);

    request_queue.push(type);

    cv.wait(lock, [&] {
        return (request_queue.front() == type && (current_type == type || current_type == -1));
        });

    current_type = type;
    active_threads++;
    request_queue.pop(); 

    std::cout << "Thread " << thread_id << " (type " << (type == 0 ? "alb" : "negru")
        << ") acceseaza resursa.\n";
    lock.unlock();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    lock.lock();
    std::cout << "Thread " << thread_id << " (type " << (type == 0 ? "alb" : "negru")
        << ") elibereaza resursa.\n";
    active_threads--;

    if (active_threads == 0) {
        current_type = -1;
    }

    cv.notify_all();
}

void threadFunction(int thread_id, int type) {
    accessResource(thread_id, type);
}

int main() {
    const int num_threads = 10;

    std::thread threads[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        threads[i] = std::thread(threadFunction, i, i % 2);
    }

    for (int i = 0; i < num_threads; ++i) {
        threads[i].join();
    }

    return 0;
}
