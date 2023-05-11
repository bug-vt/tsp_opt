#g++ -fopenmp tsp.cpp -o tsp
#
#touch c-log.txt
#
#START=8
#END=12
#
#for (( i=$START; i<=$END; i++ ))
#do
#  ./tsp 0 $i >> c-log.txt
#done
#
#g++ -fopenmp -O3 tsp.cpp -o tsp
#
#START=8
#END=13
#
#for (( i=$START; i<=$END; i++ ))
#do
#  ./tsp 0 $i >> c-log.txt
#done
#
#START=10
#END=30
#
#for (( i=$START; i<=$END; i++ ))
#do
#  ./tsp 1 $i >> c-log.txt
#done

g++ -fopenmp -O3 tsp.cpp -o tsp
START=10
END=30

for (( i=$START; i<=$END; i++ ))
do
  ./tsp 2 $i >> c-log.txt
done

