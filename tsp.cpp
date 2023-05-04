#include <iostream>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <limits>
#include <omp.h>
#include <string>

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

float dist (City a, City b)
{
  return sqrt (pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

void tsp_opt (vector<City> cities, float curr_total)
{
  if (curr_total > min_total_dist)
    return;

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

string show_route ()
{
  string out = "[ ";
  for (int i = 0; i < tsp_route.size (); i++)
    out += to_string(tsp_route[i].id) + " ";

  out += "]";
  return out;
}


int main (int argc, char** argv)
{
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
  
  printf ("Size %d | %.3f seconds | %.2f | %s\n", 
           n, duration, min_total_dist, show_route().c_str());
}
