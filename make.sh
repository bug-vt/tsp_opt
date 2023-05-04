g++ -fopenmp -O3 tsp.cpp -o tsp

RUN=25

for (( i=8; i<=$RUN; i++ ))
do
  ./tsp $i 
done
