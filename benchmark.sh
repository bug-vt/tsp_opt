
touch log.txt

START=5
END=10

for (( i=$START; i<=$END; i++ ))
do
  ./tsp.py 0 $i >> log.txt
done

START=6
END=11

for (( i=$START; i<=$END; i++ ))
do
  ./tsp.py 1 $i >> log.txt
done

START=7
END=12

for (( i=$START; i<=$END; i++ ))
do
  ./tsp.py 2 $i >> log.txt
done

START=8
END=13

for (( i=$START; i<=$END; i++ ))
do
  ./tsp.py 3 $i >> log.txt
done

START=10
END=20

for (( i=$START; i<=$END; i++ ))
do
  ./tsp.py 4 $i >> log.txt
done

START=10
END=24

for (( i=$START; i<=$END; i++ ))
do
  ./tsp.py 5 $i >> log.txt
done

START=15
END=25

for (( i=$START; i<=$END; i++ ))
do
  ./tsp.py 6 $i >> log.txt
done

START=15
END=25

for (( i=$START; i<=$END; i++ ))
do
  ./tsp.py 7 $i >> log.txt
done
