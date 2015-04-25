#Tema 2 SO - Test 3
#Se testeaza daca a fost implementata operatia add
echo "Started test 3 ..."
cd ..
./server
sleep 0.5
./client a 3 a 1 a 4 a 5 s 100 p | diff -uEb - _test/test3.in
if [ $? -eq 0 ] 
then
	echo "-- PASSED --"
else
	echo "-- FAILED --"
fi
./client e
sleep 0.5
echo
