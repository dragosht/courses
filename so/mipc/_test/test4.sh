#Tema 2 SO - Test 4
#Se testeaza daca s-au implementat corect operatiile remove si clear
echo "Started test 4 ..."
cd ..
./server
sleep 0.5
./client a 8 a 0 a 17 a -1 a 10 a 9 a 20 a 16 a 19 a 21 s 100 p \
r 17 s 100 p r 0 s 100 p c s 100 p a 3 a 1 a 4 r 2 s 100 p \
| diff -uEb - _test/test4.in
if [ $? -eq 0 ] 
then
	echo "-- PASSED --"
else
	echo "-- FAILED --"
fi
./client e
sleep 0.5
echo
