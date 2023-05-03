#!/usr/bin/env python3

import random
import math
import sys
from timeit import default_timer as time

class City:
  def __init__ (self, name):
    self.name = name
    self.x = random.randint (0, 500)
    self.y = random.randint (0, 500)

  def __str__ (self):
    return str (self.name)

  def __repr__ (self):
    return str (self)


n = int (sys.argv[1])
cities = [City(i) for i in range (n)]
visited = [None] * n
tsp_route = [None] * n
min_total_dist = math.inf


def dist (a, b):
  return math.sqrt ( (a.x - b.x)**2 + (a.y - b.y)**2 )

def calc_total_dist (visited):
  global min_total_dist
  global tsp_route
  
  # find total distance by visiting the cities in the given order
  total_dist = 0
  for i in range (len (visited) - 1):
    total_dist += dist (visited[i], visited[i+1]) 
  
  # update minimum total distance if needed
  if total_dist < min_total_dist:
    min_total_dist = total_dist
    tsp_route = [visited[i] for i in range (len (visited))]

def tsp_unopt (cities):
  # base case: no more cities to visit
  if len (cities) == 0:
    calc_total_dist (visited)
    return 0

  for i in range (len (cities)):
    visited[len (cities) - 1] = cities[i] 
    tsp_unopt (cities[:i] + cities[i + 1:])
         

def main ():
  
  start = time ()
  tsp_unopt (cities)
  duration = time () - start
  print ("Size %d : %.3f seconds" % (n, duration) )
  #print (min_total_dist)
  #print (tsp_route)

if __name__ == "__main__":
  main ()
