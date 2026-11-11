#include <functional>
#include <future>
#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <algorithm>
#include "BlockingQueue.h"



template <typename Actual, typename Expect>
bool is_equal(Actual&& A, Expect&& B) {
    return A == B;
}

bool push_pop_test() {
    BlockingQueue bq;
    bq.push(1);
    bq.push(2);
    auto out1 = bq.pop();
    auto out2 = bq.pop();
    bq.close();
    auto out3 = bq.pop();

    if (!is_equal(out1, 1)) return false;
    if (!is_equal(out2, 2)) return false;
    if (!is_equal(out3, std::nullopt)) return false;

    return true;
}

bool close_pop_test() {
    BlockingQueue bq;

    bq.push(1);
    bq.close();
    auto out1 = bq.pop();
    auto out2 = bq.pop();

    if (!is_equal(out1, 1)) return false;
    if (!is_equal(out2, std::nullopt)) return false;

    return true;
}

bool close_push_test() {
    BlockingQueue bq;

    bq.close();
    auto out = bq.push(1);

    if (!is_equal(out, false)) return false;

    return true;
}

bool close_empty_test() {
    BlockingQueue bq;
    bq.close();

    auto out1 = bq.push(1);
    auto out2 = bq.pop();

    if (!is_equal(out1, false)) return false;
    if (!is_equal(out2, std::nullopt)) return false;

    return true;
}

bool concurrency_test() {
    BlockingQueue bq;
    const int THREADS_COUNT = 10;
    const int SIZE = 100'000;

    auto lambda = [&bq](std::promise<std::vector<int>> prom) {
        std::vector<int> got;
        auto cur = bq.pop();

        while (cur != std::nullopt) {
            got.push_back(cur.value());
            cur = bq.pop();
        }

        prom.set_value(std::move(got));
    };


    std::vector<std::promise<std::vector<int>>> prom(THREADS_COUNT);
    std::vector<std::future<std::vector<int>>> fut;
    for (int i = 0; i < THREADS_COUNT; ++i) {
        fut.push_back(prom[i].get_future());
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < THREADS_COUNT; i++) {
        threads.emplace_back(lambda, std::move(prom[i]));
    }
    std::vector<int> expect;
    for (int i = 0; i < SIZE; ++i) {
        expect.push_back(i);
        bq.push(i);
    }
    bq.close();

    for (int i = 0; i < THREADS_COUNT; i++) {
        threads[i].join();
    }

    std::vector<std::vector<int>> received(THREADS_COUNT);

    for (int i = 0; i < THREADS_COUNT; ++i) {
        received[i] = fut[i].get();
    }

    std::vector<int> actual;
    for (const auto& vec : received) {
        for (const auto& c : vec) {
            actual.push_back(c);
        }
    }

    sort(actual.begin(), actual.end());


    if (!is_equal(actual, expect)) return false;

    return true;
}

signed main() {

    std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"push_pop", push_pop_test},
        {"close_pop", close_pop_test},
        {"close_push", close_push_test},
        {"close_empty", close_empty_test},
        {"concurrency", concurrency_test}
    };

    for (auto& [name, test] : tests) {
        if (test()) {
            std::cout << "PASS " << name << "\n";
        }
        else {
            std::cout << "FAIL " << name << "\n";
        }
    }


    return 0;
}