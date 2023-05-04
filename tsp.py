#!/usr/bin/env python3

import random
import math
import sys
from timeit import default_timer as time
import pymp
import numpy

class City:
  def __init__ (self, index):
    self.index = index
    self.x = random.randint (0, 500)
    self.y = random.randint (0, 500)

  def __str__ (self):
    return str (self.index)

  def __repr__ (self):
    return str (self)


nthread = 4
n = int (sys.argv[1])
cities = [City(i) for i in range (n)]
visited = [city for city in cities]
tsp_route = [None] * n
min_total_dist = math.inf


def dist (a, b):
  return math.sqrt ( (a.x - b.x)**2 + (a.y - b.y)**2 )

def calc_total_dist (visited):
  global min_total_dist
  global tsp_route
 
  # connect first and last cities to form cycle
  total_dist = dist (visited[0], visited[n-1])
  # find total distance by visiting the cities in the given order
  for i in range (len (visited) - 1):
    total_dist += dist (visited[i], visited[i+1]) 
  
  # update minimum total distance if needed
  if total_dist < min_total_dist:
    min_total_dist = total_dist
    tsp_route = [visited[i] for i in range (len (visited))]

# ------------------------------------------------------------
# Unopt

def tsp_unopt (cities):
  # base case: no more cities to visit
  if len (cities) == 0:
    calc_total_dist (visited)
    return 0

  for i in range (len (cities)):
    visited[len (cities) - 1] = cities[i] 
    tsp_unopt (cities[:i] + cities[i + 1:])

# ------------------------------------------------------------
# Opt 2: accumulate distance while traversing permutation

def tsp_opt2 (cities, curr_total):
  global min_total_dist
  global tsp_route

  size = len (cities) 
  # base case: no more cities to visit
  if size == 0:
    # connect first and last cities to form cycle
    total_dist = dist (visited[0], visited[n-1]) + curr_total
    # update minimum total distance if needed
    if total_dist < min_total_dist:
      min_total_dist = total_dist
      tsp_route = [visited[i] for i in range (len (visited))]
    return 0

  for i in range (size):
    visited[size - 1] = cities[i] 
    tsp_opt2 (cities[:i] + cities[i + 1:], curr_total + dist(visited[size-1], visited[size]))

# ------------------------------------------------------------
# Opt 3: Prune search space

def tsp_opt3 (cities, curr_total):
  global min_total_dist
  global tsp_route

  # stop searching when we already know the route cannot be minimum
  if curr_total > min_total_dist:
    return 0

  size = len (cities) 
  # base case: no more cities to visit
  if size == 0:
    # connect first and last cities to form cycle
    total_dist = dist (visited[0], visited[n-1]) + curr_total
    # update minimum total distance if needed
    if total_dist < min_total_dist:
      min_total_dist = total_dist
      tsp_route = [visited[i] for i in range (len (visited))]
    return 0

  for i in range (size):
    visited[size - 1] = cities[i] 
    tsp_opt3 (cities[:i] + cities[i + 1:], curr_total + dist(visited[size-1], visited[size]))


# ------------------------------------------------------------
# Opt 4: Prune better using approximation algorithm

def prim_mst (cities):
  # initialize
  total_dist = 0
  cost = [math.inf for i in range (len (cities))]   # cost of unvisited cities
  in_mst = [False for i in range (len (cities))]    # keep track of visited cities

  # pick source city
  # in this case, we pick the first city 
  cost[0] = 0
  min_city = (cities[0], 0)

  # For each iteration, visit next city with minimum cost
  for i in range (len (cities) - 1):
    new_city = min_city[0]
    in_mst[min_city[1]] = True
    next_min = math.inf

    # update the cost of the neighboring cities if needed
    # and find the next minimum cost city to visit.
    # note that since tsp is a complete graph, neighbor is all the other cities.
    for j in range (len (cities)):
      # only consider unvisited neighbor
      if in_mst[j] == False:
        if dist (new_city, cities[j]) < cost[j]:
          cost[j] = dist (new_city, cities[j])
        if cost[j] < next_min:
          min_city = (cities[j], j)
          next_min = cost[j]
    
    # build MST each time we visit next city.
    # in this case, we just keep track of total cost
    total_dist += next_min

  return total_dist

def tsp_opt4 (cities, curr_total):
  global min_total_dist
  global tsp_route

  size = len (cities) 
  # base case: no more cities to visit
  if size == 0:
    # connect first and last cities to form cycle
    total_dist = dist (visited[0], visited[n-1]) + curr_total
    # update minimum total distance if needed
    if total_dist < min_total_dist:
      min_total_dist = total_dist
      tsp_route = [visited[i] for i in range (len (visited))]
    return 0

  # stop searching when we already know the route cannot be minimum
  if curr_total + prim_mst (cities) > min_total_dist:
    return 0

  for i in range (size):
    visited[size - 1] = cities[i] 
    tsp_opt4 (cities[:i] + cities[i + 1:], curr_total + dist(visited[size-1], visited[size]))

# ------------------------------------------------------------
# Opt 5: Cache MST to avoid recomputation

mst_table = {}

def mst_lookup (cities):
  global mst_table
  
  # cities are encoded into n bit binary to represent key for the hash table 
  key = 0
  for city in cities:
    bit = 1 << city.index
    key |= bit

  if key not in mst_table:
    mst_table[key] = prim_mst (cities)

  return mst_table[key]

def tsp_opt5 (cities, curr_total):
  global min_total_dist
  global tsp_route

  size = len (cities) 
  # base case: no more cities to visit
  if size == 0:
    # connect first and last cities to form cycle
    total_dist = dist (visited[0], visited[n-1]) + curr_total
    # update minimum total distance if needed
    if total_dist < min_total_dist:
      min_total_dist = total_dist
      tsp_route = [visited[i] for i in range (len (visited))]
    return 0

  # stop searching when we already know the route cannot be minimum
  if curr_total + mst_lookup (cities) > min_total_dist:
    return 0

  for i in range (size):
    visited[size - 1] = cities[i] 
    tsp_opt5 (cities[:i] + cities[i + 1:], curr_total + dist(visited[size-1], visited[size]))

# ------------------------------------------------------------
# Opt 6: Parallelizing with static scheduling

def tsp_recurse (cities, curr_total):
  global min_total_dist
  global tsp_route

  size = len (cities) 
  # base case: no more cities to visit
  if size == 0:
    # connect first and last cities to form cycle
    total_dist = dist (visited[0], visited[n-1]) + curr_total
    # update minimum total distance if needed
    if total_dist < min_total_dist[0]:
      min_total_dist[0] = total_dist
      for i in range (len (visited)):
        tsp_route[i] = visited[i].index
    return 0

  # stop searching when we already know the route cannot be minimum
  if curr_total + mst_lookup (cities) > min_total_dist[0]:
    return 0

  for i in range (size):
    visited[size - 1] = cities[i] 
    tsp_recurse (cities[:i] + cities[i + 1:], curr_total + dist(visited[size-1], visited[size]))

def tsp_opt6 ():
  # parallelize subtree under first city (subtree with second city as a root)
  # pymp.Parallel is implicitly firstprivate
  with pymp.Parallel(nthread) as p:
    # We fix last city inside the cities list as a starting point
    # so starting size is length of cities - 1
    size = len (cities) - 1
    for i in p.range (size):
      visited[size - 1] = cities[i] 
      tsp_recurse (cities[:i] + cities[i + 1:size], dist(visited[size-1], visited[size]))

# ------------------------------------------------------------
# Opt 7: Parallelizing with dynamic scheduling

def tsp_opt7 ():
  # parallelize subtree under first city (subtree with second city as a root)
  # pymp.Parallel is implicitly firstprivate
  with pymp.Parallel(nthread) as p:
    # We fix last city inside the cities list as a starting point
    # so starting size is length of cities - 1
    size = len (cities) - 1
    for i in p.xrange (size):
      visited[size - 1] = cities[i] 
      tsp_recurse (cities[:i] + cities[i + 1:size], dist(visited[size-1], visited[size]))

def main ():
  global min_total_dist
  global tsp_route 
  
#  min_total_dist = math.inf
#  start = time ()
#  tsp_unopt (cities)
#  duration = time () - start
#  print ("Unopt | Size %d | %.3f seconds | %d | %s" % (n, duration, min_total_dist, tsp_route) )
#
#  min_total_dist = math.inf
#  start = time ()
#  tsp_unopt (cities[:n-1])
#  duration = time () - start
#  print ("Opt 1 | Size %d | %.3f seconds | %d | %s" % (n, duration, min_total_dist, tsp_route) )
#
#  min_total_dist = math.inf
#  start = time ()
#  tsp_opt2 (cities[:n-1], 0)
#  duration = time () - start
#  print ("Opt 2 | Size %d | %.3f seconds | %d | %s" % (n, duration, min_total_dist, tsp_route) )
#
#  min_total_dist = math.inf
#  start = time ()
#  tsp_opt3 (cities[:n-1], 0)
#  duration = time () - start
#  print ("Opt 3 | Size %d | %.3f seconds | %d | %s" % (n, duration, min_total_dist, tsp_route) )
#
#  min_total_dist = math.inf
#  start = time ()
#  tsp_opt4 (cities[:n-1], 0)
#  duration = time () - start
#  print ("Opt 4 | Size %d | %.3f seconds | %d | %s" % (n, duration, min_total_dist, tsp_route) )
#
#  min_total_dist = math.inf
#  start = time ()
#  tsp_opt5 (cities[:n-1], 0)
#  duration = time () - start
#  print ("Opt 5 | Size %d | %.3f seconds | %d | %s" % (n, duration, min_total_dist, tsp_route) )
#  
#  # share minimum total distance between threads
#  min_total_dist = pymp.shared.array((1,))
#  min_total_dist[0] = math.inf
#  tsp_route = pymp.shared.array((n,), dtype='uint8')
#
#  start = time ()
#  tsp_opt6 ()
#  duration = time () - start
#  print ("Opt 6 | Size %d | %.3f seconds | %d | %s" % (n, duration, min_total_dist[0], tsp_route) )
#
  # share minimum total distance between threads
  min_total_dist = pymp.shared.array((1,))
  min_total_dist[0] = math.inf
  tsp_route = pymp.shared.array((n,), dtype='uint8')

  start = time ()
  tsp_opt7 ()
  duration = time () - start
  print ("Opt 7 | Size %d | %.3f seconds | %d | %s" % (n, duration, min_total_dist[0], tsp_route) )


if __name__ == "__main__":
  main ()
