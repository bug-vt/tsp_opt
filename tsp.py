#!/usr/bin/env python3

import random
import math

class City:
  def __init__ (self, name):
    self.name = name
    self.x = random.randint (0, 500)
    self.y = random.randint (0, 500)

  def __str__ (self):
    return str (self.name)

  def __repr__ (self):
    return str (self)


n_max = 4
cities = [City(i) for i in range (n_max)]
visited = [None] * n_max
tsp_route = [None] * n_max
min_total_dist = math.inf


def dist (a, b):
  return math.sqrt ( (a.x - b.x)**2 + (a.y - b.y)**2 )

def calc_total_dist (visited):
  global min_total_dist
  global tsp_route

  total_dist = 0
  for i in range (len (visited) - 1):
    total_dist += dist (visited[i], visited[i+1]) 

  if total_dist < min_total_dist:
    min_total_dist = total_dist
    tsp_route = [visited[i] for i in range (len (visited))]

def tsp_unopt (cities):
  # base case
  if len (cities) == 0:
    calc_total_dist (visited)
    return 0

  for i in range (len (cities)):
    visited[len (cities) - 1] = cities[i] 
    tsp_unopt (cities[:i] + cities[i + 1:])
         

def main ():
  tsp_unopt (cities)
  print (min_total_dist)
  print (tsp_route)

if __name__ == "__main__":
  main ()
