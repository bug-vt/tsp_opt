
g++ -fopenmp -O3 tsp.cpp -o tsp
echo "O3 flag on"

START=8
END=12

for (( i=$START; i<=$END; i++ ))
do
  ./tsp 0 $i 
done


START=10
END=30

for (( i=$START; i<=$END; i++ ))
do
  ./tsp 1 $i
done

START=10
END=30

for (( i=$START; i<=$END; i++ ))
do
  ./tsp 2 $i
done
