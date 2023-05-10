#include <iostream>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <limits>
#include <omp.h>
#include <string>
#include <utility>
#include <unordered_map>

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

ostream& operator<< (ostream& os, const City& city)
{
  os << city.id;
  return os;
}

vector<City> cities; 
vector<City> visited;
vector<City> tsp_route;
float min_total_dist;
unordered_map<long, float> mst_table;

omp_lock_t mst_lock;
omp_lock_t min_total_lock;

string show_route ()
{
  string out = "[ ";
  for (int i = 0; i < tsp_route.size (); i++)
    out += to_string(tsp_route[i].id) + " ";

  out += "]";
  return out;
}

float dist (City a, City b)
{
  return sqrt (pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

float prim_mst (vector<City> cities)
{
  float total_dist = 0;
  vector<float> cost (cities.size (), numeric_limits<float>::infinity ());
  vector<char> in_mst (cities.size (), 0);

  cost[0] = 0;
  pair<City, int> min_city (cities[0], 0);

  for (int i = 0; i < cities.size () - 1; i++)
  {
    City new_city = min_city.first;
    in_mst[min_city.second] = 1;
    float next_min = numeric_limits<float>::infinity ();
    
    for (int j = 0; j < cities.size (); j++)
    {
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
    total_dist += next_min;
  }
  return total_dist;
}

float mst_lookup (vector<City> cities)
{
  long key = 0;
  for (int i = 0; i < cities.size (); i++)
  {
    long bit = 1 << cities[i].id;
    key |= bit;
  }

  omp_set_lock (&mst_lock);
  if (mst_table.count (key) == 0)
    mst_table[key] = prim_mst (cities);
  
  float out = mst_table[key];
  omp_unset_lock (&mst_lock); 

  return out;
}

void tsp_opt (vector<City> cities, float curr_total)
{
  int size = cities.size ();
  if (size == 0)
  {
    float total_dist = dist (visited[0], visited[visited.size ()-1]) + curr_total;
    if (total_dist < min_total_dist)
    {
      min_total_dist = total_dist;
      for (int i = 0; i < visited.size (); i++)
        tsp_route[i] = visited[i];
    }
    return;
  }

  if (curr_total + mst_lookup (cities) > min_total_dist)
    return;

  for (int i = 0; i < size; i++)
  {
    visited[size-1] = cities[i];
    vector<City> sub_cities;
    for (int k = 0; k < size; k++)
    {
      if (k != i)
        sub_cities.push_back (cities[k]);
    }

    tsp_opt (sub_cities, curr_total + dist (visited[size-1], visited[size]));
  }
}

void tsp_opt2 (vector<City> cities, float curr_total, City *visited)
{
  int size = cities.size ();
  float curr_min;

  if (size == 0)
  {
    float total_dist = dist (visited[0], visited[tsp_route.size()-1]) + curr_total;
    bool updated = false;

    omp_set_lock (&min_total_lock);
    if (total_dist < min_total_dist)
    {
      min_total_dist = total_dist;
      updated = true;
    }
    omp_unset_lock (&min_total_lock);

    if (updated)
    {
      for (int i = 0; i < tsp_route.size (); i++)
        tsp_route[i] = visited[i];
    }
    return;
  }

  omp_set_lock (&min_total_lock);
  curr_min = min_total_dist;
  omp_unset_lock (&min_total_lock);

  if (curr_total + mst_lookup (cities) > curr_min)
    return;

  for (int i = 0; i < size; i++)
  {
    visited[size-1] = cities[i];
    vector<City> sub_cities;
    for (int k = 0; k < size; k++)
    {
      if (k != i)
        sub_cities.push_back (cities[k]);
    }

    tsp_opt2 (sub_cities, curr_total + dist (visited[size-1], visited[size]), visited);
  }
}

void tsp_parallel ()
{
  int size = cities.size () - 1;
  #pragma omp parallel
  {
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


int main (int argc, char** argv)
{
  omp_init_lock (&mst_lock);
  omp_init_lock (&min_total_lock);

  srand (time(NULL));

  if (argc != 2)
  {
    cout << "Usage: tsp <number of cities>" << endl;
    exit (1);
  }

  int n = atoi (argv[1]);
  cities.resize(n); 
  visited.resize(n);
  tsp_route.resize(n);
  min_total_dist = numeric_limits<float>::infinity ();

  for (int i = 0; i < n; i++)
    cities[i] = City (i);

  visited[n-1] = cities[n-1];
  
  double start, duration;
  
  start = omp_get_wtime (); 
  tsp_opt (vector<City> (cities.begin (), cities.end () - 1), 0);
  duration = omp_get_wtime () - start; 
  
  printf ("C++ Opt      | Size %d | %.3f seconds | %.2f | %s\n", 
           n, duration, min_total_dist, show_route().c_str());
           
  min_total_dist = numeric_limits<float>::infinity ();
  tsp_route.clear ();
  tsp_route.resize (n);
  start = omp_get_wtime (); 
  tsp_parallel ();
  duration = omp_get_wtime () - start; 
  
  printf ("C++ parallel | Size %d | %.3f seconds | %.2f | %s\n", 
           n, duration, min_total_dist, show_route().c_str());
}
