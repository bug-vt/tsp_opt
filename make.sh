g++ -fopenmp -O3 tsp.cpp -o tsp

RUN=30

for (( i=15; i<=$RUN; i++ ))
do
  ./tsp $i 
done
