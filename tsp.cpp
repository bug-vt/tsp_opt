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

class City
{
  public:
    int id;
    int x;
    int y;

    City ()
    {
      this->id = -1;
      this->x = 0;
      this->y = 0;
    }

    City (int id)
    {
      this->id = id;
      this->x = rand() % 500;
      this->y = rand() % 500;
    }

    friend ostream& operator<< (ostream& os, const City& city); 
};



/*
ostream& operator<< (ostream& os, const City& city)
{
  os << city.id;
  return os;
}
*/

int num_city;
// cities to visit
//vector<City> cities; 
int city_id[MAXNUM_CITY];
int city_x[MAXNUM_CITY];
int city_y[MAXNUM_CITY];
// visited cities in order
//vector<City> visited;
int visited[MAXNUM_CITY];
// final/optimal tsp route
//vector<City> tsp_route;
int tsp_route[MAXNUM_CITY];
// current minimum tsp route distance
float min_total_dist;
// table for storing minimum spanning tree of each subroute
thread_local unordered_map<long, float> mst_table;

omp_lock_t min_update_lock;

void init_cities ()
{
  for (int i = 0; i < num_city; i++)
  {
    city_x[i] = rand() % 500;
    city_y[i] = rand() % 500;
  }
}

string show_route ()
{
  string out = "[ ";
  for (int i = 0; i < num_city; i++)
    out += to_string(tsp_route[i]) + " ";

  out += "]";
  return out;
}

inline float dist (int a, int b)
{
  return sqrt (pow(city_x[a] - city_x[b], 2) + pow(city_y[a] - city_y[b], 2));
}

/*
float prim_mst (vector<City> cities)
{
  // initialize
  float total_dist = 0;
  vector<float> cost (cities.size (), numeric_limits<float>::infinity ());
  vector<char> in_mst (cities.size (), 0);

  // pick source city
  // in this case, we pick the first city 
  cost[0] = 0;
  pair<City, int> min_city (cities[0], 0);

  // For each iteration, visit next city with minimum cost
  for (int i = 0; i < cities.size () - 1; i++)
  {
    City new_city = min_city.first;
    in_mst[min_city.second] = 1;
    float next_min = numeric_limits<float>::infinity ();
    
    // update the cost of the neighboring cities if needed
    // and find the next minimum cost city to visit.
    // note that since tsp is a complete graph, neighbors are all the other cities.
    for (int j = 0; j < cities.size (); j++)
    {
      // only consider unvisited neighbor
      if (!in_mst[j])
      {
        if (dist (new_city, cities[j]) < cost[j])
          cost[j] = dist (new_city, cities[j]);
        if (cost[j] < next_min)
        {
          min_city.first = cities[j];
          min_city.second = j;
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

float mst_lookup (vector<City> cities)
{
  // cities are encoded into n bit binary to represent key for the hash table 
  long key = 0;
  for (int i = 0; i < cities.size (); i++)
  {
    long bit = 1 << cities[i].id;
    key |= bit;
  }

  if (mst_table.count (key) == 0)
    mst_table[key] = prim_mst (cities);
  
  float out = mst_table[key];

  return out;
}
*/

int city_size (long city_mask)
{
  int size = 0;
  // bit 1 means city is present,
  // so add all the 1s from the bit mask
  for (int i = 0; i < num_city; i++)
  {
    size += city_mask & 1u;
    city_mask >>= 1;
  }
  return size;
}

// param:
// city_mask: bit mask that define group of cities that are considered in the current subtree.
// curr_total: total distance so far from the visited cities
void tsp_unopt (long city_mask, float curr_total)
{
  int size = city_size (city_mask);
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
  for (int i = 1; i < num_city; i++)
  {
    // exclude visited city from the city_mask
    long sub_city_mask = city_mask & ~(1u << i);
    if (sub_city_mask == city_mask)
      continue;

    visited[size-1] = i;

    tsp_unopt (sub_city_mask, curr_total + dist (visited[size-1], visited[size]));
  }
}

/*
void tsp_opt (vector<City> cities, float curr_total)
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

  // stop searching when we already know the route cannot be minimum
  if (curr_total + mst_lookup (cities) > min_total_dist)
    return;

  for (int i = 0; i < size; i++)
  {
    visited[size-1] = cities[i];
    vector<City> sub_cities (size-1);
    int k = 0;
    for (int j = 0; j < size; j++)
    {
      if (j == i)
        continue;

      sub_cities[k] = (cities[j]);
      k++;
    }

    tsp_opt (sub_cities, curr_total + dist (visited[size-1], visited[size]));
  }
}

void tsp_opt2 (vector<City> cities, float curr_total, City *visited)
{
  int size = cities.size ();
  // base case: no more cities to visit
  if (size == 0)
  {
    // connect first and last cities to form cycle
    float total_dist = dist (visited[0], visited[tsp_route.size()-1]) + curr_total;
    bool updated = false;

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
      for (int i = 0; i < tsp_route.size (); i++)
        tsp_route[i] = visited[i];
    }
    return;
  }

  // stop searching when we already know the route cannot be minimum
  if (curr_total + mst_lookup (cities) > min_total_dist)
    return;

  for (int i = 0; i < size; i++)
  {
    visited[size-1] = cities[i];
    vector<City> sub_cities (size-1);
    int k = 0;
    for (int j = 0; j < size; j++)
    {
      if (j == i)
        continue;

      sub_cities[k] = (cities[j]);
      k++;
    }

    tsp_opt2 (sub_cities, curr_total + dist (visited[size-1], visited[size]), visited);
  }
}

void tsp_parallel ()
{
  // parallelize subtree under first city (subtree with second city as a root)
  int size = cities.size () - 1;
  #pragma omp parallel
  {
    // We fix last city inside the cities list as a starting point
    // so starting size is length of cities - 1
    City *visited = new City [cities.size ()];
    visited[size] = cities[size];

    #pragma omp for schedule (dynamic)
    for (int i = 0; i < size; i++)
    {
      visited[size-1] = cities[i];
      vector<City> sub_cities;
      for (int k = 0; k < size; k++)
      {
        if (k != i)
          sub_cities.push_back (cities[k]);
      }

      tsp_opt2 (sub_cities, dist (visited[size-1], visited[size]), visited);
    }
    delete (visited);
  }
}
*/


int main (int argc, char** argv)
{
  omp_init_lock (&min_update_lock);
  srand (time(NULL));

  if (argc != 3)
  {
    cout << "Usage: tsp <option> <number of cities>" << endl;
    exit (1);
  }

  int option = atoi (argv[1]);
  num_city = atoi (argv[2]);
  //cities.resize(n); 
  //visited.resize(n);
  //tsp_route.resize(n);
  min_total_dist = numeric_limits<float>::infinity ();

  init_cities ();

  long city_mask = 0;
  for (int i = 0; i < num_city; i++)
  {
    city_mask <<= 1;
    city_mask |= 1;
  }

  // select home city (visit city 0)
  visited[num_city-1] = 0;
  city_mask &= ~1;
  
  double start, duration;

  switch (option)
  {
    case 0:
      start = omp_get_wtime (); 
      tsp_unopt (city_mask, 0);
      duration = omp_get_wtime () - start; 
      
      printf ("C++ Unopt    | Size %d | %.3f seconds | %.2f | %s\n", 
               num_city, duration, min_total_dist, show_route().c_str());
      cout << temp << endl;
      break;

    case 1:
      start = omp_get_wtime (); 
      //tsp_opt (vector<City> (cities.begin (), cities.end () - 1), 0);
      duration = omp_get_wtime () - start; 
      
      printf ("C++ Opt      | Size %d | %.3f seconds | %.2f | %s\n", 
               num_city, duration, min_total_dist, show_route().c_str());
      break;

    case 2:
      for (int i = 1; i <= 32; i *= 2)
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
        //tsp_parallel ();
        duration = omp_get_wtime () - start; 
        
        printf ("C++ parallel %d | Size %d | %.3f seconds | %.2f | %s\n", 
                 nthread, num_city, duration, min_total_dist, show_route().c_str());
      }
  }
}
