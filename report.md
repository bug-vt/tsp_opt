# Assignment P3 – Traveling Salesman Problem (TSP) Optimization
### Bug Lee (7225881)


## Overview
This project focuses on optimizing small-scale TSP (n=30). Despite the small size, naive solution using brute force become intractable, estimated to take more than billions of years of computation time. Therefore, the following optimizations were used to tackle the TSP. 
-  High level optimization was applied reduce the growth rate from factorial to exponential. Approximation algorithm based on prim's algorithm and software caching reduced runtime complexity from O(n!) to O($n^2 2^n$). This techniques were introduced in Writing Efficient Programs by Jon Bently [1]. To avoid blindly using the algorithm without understanding, deeper analysis on the algorithms were performed in this report. 
-  Multithreading using OpenMP was explored for constant factors of speed up.
-  Low level optimization was applied for noticiable speed up. Converted Array of Structure (AoS) to Structure of Array (SoA) for better hardware cache performance and removed shared variables to reduce false sharing.

In the end, small-scale TSP with input size 30 was solved in less than 100 seconds.


## Set up

All measurements were performed using the personal laptop. The specification of the laptop is:
- Architecture: x86-64
- Logical CPU threads: 8
- Model: 11th Gen Intel(R) Core(TM) i5-1135G7 @ 2.40GHz
- L1d cache: 192 KiB
- L1i cache: 128 KiB
- L2 cache: 5 MiB
- L3 cache: 8 MiB
- RAM: 16 GB
- OS: Ubuntu 20.04 LTS 

The naive implementation and all optimizations are written in C++ and compiled with g++ compiler version 9.4.0. O3 compiler optimization flag was used for all implementation. Parallelism was implemented using OpenMP version 4.5. 

The number of input cities ranged from 8 to 30. The location of each city was set to have a random x-coordinate between 0 to 500 and a random y-coordinate between 0 to 500. Random seed 0 was used to produce deterministic output.

The Optimizations were applied incrementally in order, whereas later optimization implicitly uses all earlier optimizations. 

## Naive implementation
The naive approach set the baseline performance of the algorithm. The naive solution is to try all permutations of visiting n cities in order, then select the ordering that gives the optimal result. 

```c++
void tsp_unopt (vector<City> cities, float curr_total)
{
  int size = cities.size ();
  // base case: no more cities to visit
  if (size == 0)
  {
    // connect first and last cities to form cycle
    float total_dist = dist (visited[0], visited[visited.size ()-1]) + curr_total;
    // update minimum total distance if needed
    if (total_dist < min_total_dist)
    {
      min_total_dist = total_dist;
      for (int i = 0; i < visited.size (); i++)
        tsp_route[i] = visited[i];
    }
    return;
  }

  for (int i = 0; i < size; i++)
  {
    // visit next city
    visited[size-1] = cities[i];
    vector<City> sub_cities (size-1);
    int k = 0;
    // exclude visited city from the cities
    for (int j = 0; j < size; j++)
    {
      if (j == i)
        continue;

      sub_cities[k] = (cities[j]);
      k++;
    }

    tsp_unopt (sub_cities, curr_total + dist (visited[size-1], visited[size]));
  }
}
```

However, the brute force approach requires computing (n-1)! different ordering of cities, where search space become intractable even for the small graph.

## Profile #1


## High Level: Approximation algorithm and Caching

The idea here is to safely ignore computing some of the paths by making an approximation that the subpath to be computed cannot be part of the solution (i.e. path with minimum tour cost). To do so, the algorithm should never make an over-approximated guess. Over-approximation can result in ignoring the potential solution with minimum tour cost. Therefore, the approximation should be always less than the true cost of the path. For this reason, the cost of the Minimum Spanning Tree (MST) of the path can be used for the approximation.

Proof:
We know that TSP finds a tour cycle that visits all vertices. After removing an edge from the TSP cycle, the solution becomes a spanning tree since all vertices are connected without any cycle. From this, we can see that there exists at least one spanning tree that cost less than the TSP tour cycle. Then, the lowest cost spanning tree (i.e. Minimum Spanning Tree) must cost less than the TSP tour cycle. Therefore, we conclude that it is safe to approximate the cost of a path using MST.

Prim's algorithm was used to compute the MST. The main idea behind Prim's algorithm is to find the next unvisited vertex with minimum cost and then visit it. A priority queue is generally needed for managing what vertex should visit next. The time complexity of Prim's algorithm with priority queue is known as $O((n+m)\log n)$ (= $O(n^2 \log n)$ for the complete graph). However, in the case of a complete graph, the next unvisited vertex with minimum cost must be one of the neighbors of the most recently visited vertex. Therefore, a simple scan of neighbors is enough to determine what vertex should be visited next, which can be stored in a variable instead of a queue. This simple modification allows MST to be computed in $O(n^2)$ time. 

Another catch to computing MST is that many paths contain common MST. If two paths contain the same vertices, then the same MST can be constructed. So, to avoid recomputing MST for each path, the idea here is to cache the MST after the first encounter. The hash table was used for this purpose where a set of vertices as a key and the corresponding MST cost as a value. 

Efficiency: For a set of $n$ vertices, there are $2^n$ possible subsets. Therefore, there are at most $2^n$ unique MSTs that need to be computed. Assume that cached MST cost can be obtained in $O(1)$. Combining the approximation algorithm with caching, the total time complexity of the TSP now reduces to $O(n^2 2^n)$ in the average case.


Parallelization and Scheduling Policy 

The idea here is to distribute the computation of the sub-path to multiple threads. For this reason, exploratory decomposition was used. Starting at the home city, there are $n-1$ sub-path that need to be explored. This naturally leads to partitioning the search space of TSP into $n-1$ parts that can be concurrently executed.

To maximize parallelism, a few adjustments were made to the serial implementation. First, instead of sharing computed MST among all threads, each thread gets assigned its own hash table for storing MST. Unlike serial implementation, now some MSTs get recomputed by multiple threads. As a result, total computation work is now become $O((n-1) \cdot (n-1)^2 2^{n-1})$ instead of $O(n^2 2^n)$. Second, the global minimum tour cost is shared among all threads. The global minimum tour cost is accessed in two scenarios. One scenario is when checking whether the sub-path can be pruned after comparing approximated cost with the minimum tour cost. Since minimum tour cost only decreases, accessing out-of-date value is still safe from causing over-approximation hence do not harm the correctness even without locking. Although it result in a more conservative pruning, avoiding locking contention was observed to be more effective. The other scenario is when the minimum tour cost must be updated. Unlucky scheduling order of concurrent threads can accidentally overwrite the potential minimum tour cost, so read and write must happen atomically. The lock must be used in this case, which can be problematic as the number of thread increase. Finally, a dynamic scheduling policy was used. Due to pruning and caching, the workload of each partition is highly unbalanced, making it a good candidate for dynamic scheduling.


Parallelism: Let parallelism $P = \frac{W}{S}$ where $W$ represents the sum of total computation and $S$ represents the longest/critical path length after parallelization. First, we know that search space is partitioned into $n-1$ parts where each part takes $O((n-1)^2 2^{n-1})$ to compute the optimal sub-path. From this, we find that total work is $O((n-1) \cdot (n-1)^2 2^{n-1})$. Next, let $T$ be the number of threads. Then each thread gets assigned with $\frac{n-1}{T}$ partition of search space. There is one constraint that each thread cannot get assigned a $< 1$ partition. So, the ciritical path length is $O(\frac{n-1}{T} \cdot (n-1)^2 2^{n-1})$ for $T \leq n-1$. Therefore, $P = O((n-1) \cdot (n-1)^2 2^{n-1}) / O(\frac{n-1}{T} \cdot (n-1)^2 2^{n-1}) = O(T)$ for $T \leq n-1$. However, this is a loose upper bound since it does not account for the lock contention to update the global minimum tour cost.


Results

Serial optimizations

As predicted, compiler optimization showcased a constant factor of improvement over the plain brute force algorithm. However, it can be observed that constant factors of improvement have minimal impact on scaling the TSP. On the other hand, the approximation algorithm with caching has reduced the overall growth rate, making TSP more scalable than before. 


Parallel optimization

The result shows linear/sub-linear speedup for a small number of threads $T\leq 4$. However, unlike the theoretical prediction, the result shows poor parallelism after 4 threads. The main reason is due to the heavy lock contention to update the global minimum tour cost as it increases in the number of threads. 


Conclusion

This project has explored various algorithms and techniques to tackle the famous TSP problem. For serial optimization, compiler optimization, approximation algorithm, and caching were used. Compiler optimization made the program run faster by a constant factor whereas the approximation algorithm combined with caching have reduce the growth rate of TSP. With the growth rate change, the goal input size of 30 was now solvable in tractable time. For parallel optimization, the main technique was to minimize dependency and shared resources among threads without losing correctness. However, not all locks could have been removed, which caused lock contention among threads as the number of threads increased. Although the approach used in this project is far from state of art for solving TSP, techniques and optimization learned from the project were valuable and applicable to many real-world problems outside of solving TSP.




[1] David Biron, The Traveling Salesman Problem: Deceptively Easy to State; Notoriously Hard to Solve, Liberty University, 2006

[2] Benila Virgin Jerald Xavier, A Massively Parallel Exact TSP Solver for small problem sizes, Texas State University, 2022

[3] https://www.frontiersin.org/articles/10.3389/frobt.2021.689908/full

[4] J.L. Bentley, Writing Efficient Programs, 1st ed. Prentice-Hall Software Series, 1982.