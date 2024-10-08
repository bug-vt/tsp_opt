#include <iostream>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <limits>
#include <omp.h>
#include <string>
#include <utility>
#include <unordered_map>

#define MAXNUM_CITY 32

using namespace std;

// number of cities to visit
int num_city;
// cities to visit
int city_x[MAXNUM_CITY];
int city_y[MAXNUM_CITY];
// visited cities in order
int visited[MAXNUM_CITY];
// final/optimal tsp route
int tsp_route[MAXNUM_CITY];
// current minimum tsp route distance
float min_total_dist;
// table for storing minimum spanning tree of each subroute
thread_local unordered_map<long, float> mst_table;

omp_lock_t min_update_lock;

// initialize location of the cities.
void init_cities ()
{
  for (int i = 0; i < num_city; i++)
  {
    city_x[i] = rand() % 500;
    city_y[i] = rand() % 500;
  }
}

// display tsp route. 
string show_route ()
{
  string out = "[ ";
  for (int i = 0; i < num_city; i++)
    out += to_string(tsp_route[i]) + " ";

  out += "]";
  return out;
}

// calculate distance between city a and b.
inline float dist (int a, int b)
{
  return sqrt (pow(city_x[a] - city_x[b], 2) + pow(city_y[a] - city_y[b], 2));
}

// prim minimum spanning tree algorithm.
// Implemented without an queue since tsp is a complete graph,
// reducing complexity from O(n^2 log n) to O(n^2).
float prim_mst (int city_id[], int city_size)
{
  // initialize
  float total_dist = 0;
  float cost[city_size];
  for (int i = 0; i < city_size; i++)
    cost[i] = numeric_limits<float>::infinity ();

  // keep track of visited cities when building mst (0: unvisited, 1: visited).
  int in_mst[city_size] = {0};

  // pick source city
  // in this case, we pick the first city 
  cost[0] = 0;
  int min_city = city_id[0];
  int min_city_index = 0;

  // For each iteration, visit next city with minimum cost
  for (int i = 0; i < city_size - 1; i++)
  {
    int new_city = min_city;
    in_mst[min_city_index] = 1;
    float next_min = numeric_limits<float>::infinity ();
    
    // update the cost of the neighboring cities if needed
    // and find the next minimum cost city to visit.
    // note that since tsp is a complete graph, neighbors are all the other cities.
    for (int j = 0; j < city_size; j++)
    {
      // only consider unvisited neighbor
      if (!in_mst[j])
      {
        if (dist (new_city, city_id[j]) < cost[j])
          cost[j] = dist (new_city, city_id[j]);
        if (cost[j] < next_min)
        {
          min_city = city_id[j];
          min_city_index = j;
          next_min = cost[j];
        }
      }
    }
    // build MST each time we visit next city.
    // in this case, we just keep track of total cost
    total_dist += next_min;
  }
  return total_dist;
}


float mst_lookup (int city_id[], int city_size)
{
  // cities are encoded into n bit binary to represent key for the hash table 
  long key = 0;
  for (int i = 0; i < city_size; i++)
  {
    long bit = 1 << city_id[i];
    key |= bit;
  }

  if (mst_table.count (key) == 0)
    mst_table[key] = prim_mst (city_id, city_size);
  
  float out = mst_table[key];

  return out;
}


// Param:
// city_id: IDs of unvistied cities 
// city_size: number of unvisited cities
// curr_total: total distance so far from the visited cities
void tsp_unopt (int city_id[], int city_size, float curr_total)
{
  int size = city_size;
  // base case: no more cities to visit
  if (size == 0)
  {
    // connect first and last cities to form cycle
    float total_dist = dist (visited[0], visited[num_city-1]) + curr_total;
    // update minimum total distance if needed
    if (total_dist < min_total_dist)
    {
      min_total_dist = total_dist;
      for (int i = 0; i < num_city; i++)
        tsp_route[i] = visited[i];
    }
    return;
  }
  for (int i = 0; i < size; i++)
  {
    // visit next city
    visited[size-1] = city_id[i];
    int sub_city_id[size -1];
    int k = 0;
    // exclude visited city from the city_id
    for (int j = 0; j < size; j++)
    {
      if (j == i)
        continue;
      sub_city_id[k] = city_id[j];
      k++;
    }

    tsp_unopt (sub_city_id, size-1, curr_total + dist (visited[size-1], visited[size]));
  }
}


void tsp_opt (int city_id[], int city_size, float curr_total)
{
  int size = city_size;
  // base case: no more cities to visit
  if (size == 0)
  {
    // connect first and last cities to form cycle
    float total_dist = dist (visited[0], visited[num_city-1]) + curr_total;
    // update minimum total distance if needed
    if (total_dist < min_total_dist)
    {
      min_total_dist = total_dist;
      for (int i = 0; i < num_city; i++)
        tsp_route[i] = visited[i];
    }
    return;
  }

  // stop searching when we already know the route cannot be minimum
  if (curr_total + mst_lookup (city_id, size) > min_total_dist)
    return;

  for (int i = 0; i < size; i++)
  {
    // visit next city
    visited[size-1] = city_id[i];
    int sub_city_id[size -1];
    int k = 0;
    // exclude visited city from the city_id
    for (int j = 0; j < size; j++)
    {
      if (j == i)
        continue;
      sub_city_id[k] = city_id[j];
      k++;
    }

    tsp_opt (sub_city_id, size-1, curr_total + dist (visited[size-1], visited[size]));
  }
}


void tsp_opt2 (int city_id[], int city_size, float curr_total, int visited[])
{
  int size = city_size;
  // base case: no more cities to visit
  if (size == 0)
  {
    // connect first and last cities to form cycle
    float total_dist = dist (visited[0], visited[num_city-1]) + curr_total;
    bool updated = false;
    
    // lock to avoid race condition on shared variable min_total_dist
    omp_set_lock (&min_update_lock);
    // update minimum total distance if needed
    if (total_dist < min_total_dist)
    {
      min_total_dist = total_dist;
      updated = true;
    }
    omp_unset_lock (&min_update_lock);

    if (updated)
    {
      for (int i = 0; i < num_city; i++)
        tsp_route[i] = visited[i];
    }
    return;
  }

  // stop searching when we already know the route cannot be minimum
  if (curr_total + mst_lookup (city_id, size) > min_total_dist)
    return;

  for (int i = 0; i < size; i++)
  {
    // visit next city
    visited[size-1] = city_id[i];
    int sub_city_id[size -1];
    int k = 0;
    // exclude visited city from the city_id
    for (int j = 0; j < size; j++)
    {
      if (j == i)
        continue;
      sub_city_id[k] = city_id[j];
      k++;
    }

    tsp_opt2 (sub_city_id, size-1, curr_total + dist (visited[size-1], visited[size]), visited);
  }
}

void tsp_parallel (int city_id[])
{
  // parallelize subtree under first city (create subtrees where second cities are the roots)
  int size = num_city - 1;
  #pragma omp parallel
  {
    // We fix last city inside the cities list as a starting point
    // so starting size is length of cities - 1.
    // Also, create local visited array per thread.
    int visited[num_city];
    visited[size] = size;

    #pragma omp for schedule (dynamic)
    for (int i = 0; i < size; i++)
    {
      visited[size-1] = city_id[i];
      int sub_city_id[size-1];
      int k = 0;
      // exclude visited city from the city_id
      for (int j = 0; j < size; j++)
      {
        if (j == i)
          continue;
        sub_city_id[k] = city_id[j];
        k++;
      }

      tsp_opt2 (sub_city_id, size-1, dist (visited[size-1], visited[size]), visited);
    }
  }
}



int main (int argc, char** argv)
{
  omp_init_lock (&min_update_lock);
  //srand (time(NULL));
  srand (0);

  if (argc != 3)
  {
    cout << "Usage: tsp <option> <number of cities>" << endl;
    exit (1);
  }

  int option = atoi (argv[1]); // unopt, opt, parallel
  num_city = atoi (argv[2]);  
  min_total_dist = numeric_limits<float>::infinity ();

  init_cities ();
  
  // select home city (visit last city)
  visited[num_city-1] = num_city-1;

  // city ids of unvistied cities (all cities except last city)
  int city_id [num_city-1];
  for (int i = 0; i < num_city - 1; i++)
    city_id[i] = i;
  
  double start, duration;

  switch (option)
  {
    case 0:
      start = omp_get_wtime (); 
      tsp_unopt (city_id, num_city-1, 0);
      duration = omp_get_wtime () - start; 
      
      printf ("C++ Unopt    | Size %d | %.3f seconds | %.2f | %s\n", 
               num_city, duration, min_total_dist, show_route().c_str());
      break;

    case 1:
      start = omp_get_wtime (); 
      tsp_opt (city_id, num_city-1, 0);
      duration = omp_get_wtime () - start; 
      
      printf ("C++ Opt      | Size %d | %.3f seconds | %.2f | %s\n", 
               num_city, duration, min_total_dist, show_route().c_str());
      break;

    case 2:
      for (int i = 4; i <= 4; i *= 2)
      {
        int nthread = -1;
        omp_set_num_threads (i);
        // save number of threads
        #pragma omp parallel
        { 
          #pragma omp single
          nthread = omp_get_num_threads ();
        }
        min_total_dist = numeric_limits<float>::infinity ();
        
        start = omp_get_wtime (); 
        tsp_parallel (city_id);
        duration = omp_get_wtime () - start; 
        
        printf ("C++ parallel %d | Size %d | %.3f seconds | %.2f | %s\n", 
                 nthread, num_city, duration, min_total_dist, show_route().c_str());
      }
  }
}
