#include <boost/fiber/all.hpp>
#include <functional>
#include <iostream>
#include <vector>

struct Graph {
    std::vector<std::vector<int>> adj;
};

void dfs_fiber_task(const Graph& gr, int start, int id) {
    std::vector<char> used(gr.adj.size(), false);

    std::function<void(int)> dfs = [&](int v) {
        used[v] = true;
        std::cout << "[task " << id << "] visit " << v << "\n";
        boost::this_fiber::yield();

        for (int to : gr.adj[v]) {
            if (!used[to]) dfs(to);
        }
    };

    dfs(start);
    std::cout << "[task " << id << "] done\n";
}

int main() {
    Graph gr{
        {
            {1, 2, 3},
            {4, 5},
            {5, 6},
            {6, 7},
            {8},
            {8, 9},
            {9, 10},
            {10, 11},
            {12},
            {12, 13},
            {13},
            {14},
            {15},
            {},
            {15},
            {}
        }
    };

    boost::fibers::fiber f1(dfs_fiber_task, std::cref(gr), 0, 1);
    boost::fibers::fiber f2(dfs_fiber_task, std::cref(gr), 2, 2);
    boost::fibers::fiber f3(dfs_fiber_task, std::cref(gr), 4, 3);

    f1.join();
    f2.join();
    f3.join();
    return 0;
}
