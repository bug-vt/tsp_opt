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

The brute force approach set the base line performance of the algorithm. The following describe the pseudocode.


The following subsections describe the 6 optimizations used for speeding up the TSP. Note that optimizations were applied incrementally in order, where later optimization implicitly use all eailer optimizations. 

Compiler optimization flag

Using partial sum

Prunning

Approximation algorithm

Dynamic programming

Parallelization

Scheduling policy


Results

Conclusion

[1] David Biron, The Traveling Salesman Problem: Deceptively Easy to State; Notoriously Hard to Solve, Liberty University, 2006

[2] Benila Virgin Jerald Xavier, A Massively Parallel Exact TSP Solver for small problem sizes, Texas State University, 2022

[3] https://www.frontiersin.org/articles/10.3389/frobt.2021.689908/full