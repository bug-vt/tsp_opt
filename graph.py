#!/usr/bin/env python3

import matplotlib.pyplot as plt

PATH = ""

# table for storing execution times.
table = {}
# holds serial execution time.
serial = 0

def appendTable (path, name):
  global serial
  f = open (path + name + ".txt", "r")
  lines = f.readlines ()
 
  num_threads = ""
  is_parallel = False
  for line in lines:
    raw = line.split ()
    # ignore filler
    if len (raw) == 0:
      continue
    elif "Unopt" in raw[1] or "O3" in raw[1] or "Opt" in raw[1]:
      opt = raw[4]
      table[opt] = []
      is_parallel = False 
    elif "Parallel" in raw[1]:
      is_parallel = True 
      num_threads = int(raw[2])
    # append table with measured execution time for given optimization
    else:
      if is_parallel:
        table[opt].append (float(raw[1]))
      else:
        serial = float(raw[1])

def createGraph (question, title, pa2=False):
  global serial
  plt.clf()

  # speedup table
  speedups = {}
  # horizontal values (number of threads)
  x = [1,2,4,8,16,32]

  # plot graph for execution time and create table for speedup
  for opt, row in table.items ():
    plt.plot (range(len(x)), row, 'o-', label="size %sx%s"%(opt,opt))
    speedups[opt] = [row[0] / row[i] for i in range (len(row))]
  # title, labels, scaling
  plt.title("OpenMP %s execution time \nfor Matrix-Matrix Multiply" % (title))
  plt.ylabel("Execution time (seconds)")
  plt.xlabel("Number of threads")
  plt.xticks(range(len(x)),x)
  plt.yscale ('log', base=2)
  plt.legend()
  # save to image file
  plt.savefig('%s_exec_time.png' % (question))
  
  # clear graph 
  plt.clf()
  # plot graph for speedup
  for opt, row in speedups.items ():
    if pa2:
      plt.plot (range(len(x)), row, 'o-', label=("Task" if opt=="2048" else "Loop"))
    else:
      plt.plot (range(len(x)), row, 'o-', label="size %sx%s"%(opt,opt))

  # title, labels, scaling
  if pa2:
    plt.title("OpenMP %s speed up \nfor 2048x2048 Matrix-Matrix Multiply" % (title))
  else:
    plt.title("OpenMP %s speed up \nfor Matrix-Matrix Multiply" % (title))

  plt.ylabel("Speed up ratio")
  plt.xlabel("Number of threads")
  plt.xticks(range(len(x)),x)
  plt.yscale ('log', base=2)
  plt.legend()
  # save to image file
  plt.savefig('%s_speedup.png' % (question))
  

def main ():
  global table
  appendTable (PATH, "c-log")
  createGraph ("serial", "TSP serial optimizations" )
  
  table = {}
  appendTable (PATH, "c-log_par")
  createGraph ("parallel", "TSP parallel optimization", pa2=True)


if __name__ == "__main__":
  main ()
