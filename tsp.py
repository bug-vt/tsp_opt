#!/usr/bin/env python3

n_max = 4
cities = [x for x in range (n_max)]
visited = [None] * n_max

def dist (a, b):
  return sqrt ( (a.x - b.x)**2 + (a.y - b.y)**2 )


def tsp_unopt (cities):
  # base case
  if len (cities) == 0:
    print (visited)
    return 0
  for i in range (len (cities)):
    visited[len (cities) - 1] = cities[i] 
    tsp_unopt (cities[:i] + cities[i + 1:])
         

def main ():
  tsp_unopt (cities)


if __name__ == "__main__":
  main ()
