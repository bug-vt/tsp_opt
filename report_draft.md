Introduction

Background

The first Traveling Salesman Problem (TSP) arose in 1856 by Hamilton, where the objective was to find "a path such that every vertex is visited a single time, no edge is visited twice, and the ending point is the same as the starting point" [1]. This is known as finding a Hamiltonian cycle. Now, TSP adds one additional constraint: find the lowest cost Hamiltonian cycle for the given weighted graph. In plain English, "What is the shortest route that starts and ends at the home city and lets the salesman visit all n cities exactly once?" 

The objective is simple and it is easy to come up with a brute-force solution. The brute force solution is to try all permutations of visiting n cities in order, then select the ordering that gives the optimal result. However, the brute force approach requires computing (n-1)! different ordering of cities, where search space become intractable even for the small graph.

Many algorithms, techniques, and optimizations have been introduced and attempted to solve or approximate the solution for TSP. The exact solution for 24978 cities was solved by David Applegate at el. in 2001, and later for 85900 cities was solved with Concorde TSP solver in 2006 [2]. On the other hand, the use of modern TSP heuristics can find a suboptimal solution for 1904711 cities within 0.1% optimum [3]. 

This project focuses on the selected algorithms and techniques introduced in Writing Efficient programs by Jon Bently [4] to solve small scale TSP (n=30). Parallelization using OpenMP was also explored for additional speed up. The book claim that use of compiler optimization, partial sum, and parallelization can speed up the computation, but only by constant factors. As a result, each optimization only allow for acceptable input size to grow by 1 or 2. More effective approach that book suggest is the pruning the search space using approximation algorithm and reducing number of recomputation using dynamic programming. Result of the combined technique reduce the runtime complexity of the TSP algorithm from $O(n!)$ to $O(n^2 2^n)$, reducing the factorial growth to expoential growth. 

Approach

All measurements was performed inside the rlogin machine. The specification of the rlogin machine is:
- Architecture:
- Logical CPU threads:
- Model:
- L1d cache:
- L2 cache:
- L3 cache:
- RAM:
- OS:

Brute force approach and all optimizations are written in C++ and compiled with g++ compiler version 8.5.0. Parallelizim was implemendted using OpenMP version 4.5. 

Optimizations were implemented and tested incrementally. Number of input cities were range from 8 to 30. Location of each cities were set to have random x-coordinate between 0 to 500 and random y-coordinate between 0 to 500. Note that some optimizations were not sufficient to compute certain input size and above without requiring years of computation time. Therefore, some measurements from optimizations that do not include prunning are estimation based on the grwoth pattern.

The brute force approach set the base line performance of the algorithm. All possible permutation of visiting order of cities can be generated using the following recursive algorithm.  
The tour cost is computed at the base case by following the parent pointer. Also, we set aside a variable to keep track of current minimum tour cost during execution. Once algorithm terminate (i.e. exhuasted all possible tour), the variable hold the minimum tour cost of the graph. 


The following subsections describe the serial and parallel optimizations used for speeding up the TSP. Note that optimizations were applied incrementally in order, where later optimization implicitly use all eailer optimizations. 

Compiler optimization flag

Compiler optimization truncates output assembly or machine code without changing the semantics of the original code. It performs dead code eliminations, constant folding, strength reduction, etc. O3 compiler flag was used for all measurements.

Approximation algorithm and Caching

The idea here to safely ignore computing some of the paths by making an approximation that subpath to be computed cannot be part of the solution (i.e. path with minimum tour cost). To do so, the algorithm should never make an over-approximated guess. Over-approximation can result in ignoring the potential solution with minimum tour cost. Therefore, approxmiation should be always less than the true cost of the path. For this reason, cost of Minimum Spanning Tree (MST) of the path can be used for the approximation.

Proof:
We know that TSP find a tour cycle that visit all vertices. After removing an edge from the TSP cycle, the solution become an spanning tree since all vertices are connected without any cycle. From this, we can see that there exists a at least one spanning tree that cost less than the TSP tour cycle. Then, the lowest cost spanning tree (i.e. Minimum Spanning Tree) must cost less than TSP tour cycle. Therefore, we conclude that it is safe to approxmiate cost of path using MST.

The Prim's algorithm was used to compute the MST. The main idea behind the Prim's algorithm is to find the next unvisited vertex with minimum cost then visiting it. Priority queue is generally needed for managing what vertex should visit next. The time complexity of Prim's algorithm with prioirty queue is known as $O((n+m)\log n)$ (= $O(n^2 \log n)$ for complete graph). However, in case of a complete graph, the next unvisited vertex with minimum cost must be one of the neighbors of the most recently visited vertex. Therefore, a simple scan of neighbors is enough to determine what vertex should be visted next, which can be stored in a variable instead of queue. This simple modification allow MST to be computed in $O(n^2)$ time. 

Another catch to computing MST is that many path contain common MST. If two path contain same vertices, then same MST can be constructed. So, to avoid recomputing MST for each path, the idea here to cache the MST after the first encounter. Hash table was used for this purpose where set of vertices as a key and corresponding MST cost as a value. 

Efficiency: For a set of $n$ vertices, there are $2^n$ possible subsets. Therefore, there are at most $2^n$ unique MST need to computed. Assume that cached MST cost can be obtained in $O(1)$. Combining the approximation algorithm with caching, the total time compelxity of the TSP now reduce to $O(n^2 2^n)$ in average case.


Parallelization and Scheduling policy 

The idea here is to distribute computation of sub-path to multiple threads. For this reason, exploratory decomposition was used. Starting at the home city, there are $n-1$ sub-path that need to be explored. This naturally lead to partitioning search space of TSP into $n-1$ part that can be concurrently executed.

To maximize parallelism, few adjustment was needed to made from the serial implementation. First, instead of sharing computed MST among all thread, each thread get assigned with its own hash table for storing MST. Unlike serial implementation, now some MST get recomputed by multiple threads. As a result, total computation work is now become $O((n-1) \cdot (n-1)^2 2^{n-1})$ instead of $O(n^2 2^n)$. Second, global minimum tour cost is shared among all threads. The global minimum tour cost is access in two senarios. one senario is when checking whether the sub-path can be prune after comparing approximated cost with minimum tour cost. Since minimum tour cost only decrease, accessing out-of-date value is still safe from causing over-approximation hence do not harm the correctness even without locking. Although it result to a more conservative pruning, avoiding locking contention was observed to be more effective. The other senario is when minimum tour cost must be updated. Unlucky scheduling order of concurrent threads can accidently overwrite the potential minimum tour cost, so read and write must happen atomically. Lock must be used in this case, but only happen in the base case of the recusion after survived from pruning. Finally, dynamic scheduling policy was used. Due to prunning and caching, the workload of each partition is highly unblanced, making good for dynamic scheduling.

Parallelism: Let parallelsim $P = \frac{W}{S}$ where $W$ represent sum of total computation and $S$ represent the longest/critical path length after parallelization. First, we know that search space is partitioned into $n-1$ parts where each part takes $O((n-1)^2 2^{n-1})$ to compute optimal sub-path. From this, we find that total work is $O((n-1) \cdot (n-1)^2 2^{n-1})$. Next, let $T$ be the number of threads. Then each threads get assigned with $\frac{n-1}{T}$ partition of search space. There is one contraint that each threads cannot get assigned $< 1$ partition. So, the ciritical path length is $O(\frac{n-1}{T} \cdot (n-1)^2 2^{n-1})$ for $T \leq n-1$. Therefore, $P = O((n-1) \cdot (n-1)^2 2^{n-1}) / O(\frac{n-1}{T} \cdot (n-1)^2 2^{n-1}) = O(T)$ for $T \leq n-1$.


Results


Conclusion

[1] David Biron, The Traveling Salesman Problem: Deceptively Easy to State; Notoriously Hard to Solve, Liberty University, 2006

[2] Benila Virgin Jerald Xavier, A Massively Parallel Exact TSP Solver for small problem sizes, Texas State University, 2022

[3] https://www.frontiersin.org/articles/10.3389/frobt.2021.689908/full

[4] J.L. Bentley, Writing Efficient Programs, 1st ed. Prentice-Hall Software Series, 1982.