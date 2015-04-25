#Tema 2 SO - Test 5
#Test simplu cu mai multi clienti in acelasi timp
echo "Started test 5 ..."
cd ..
./server
sleep 0.5
./client a 6 a 3 s 500 a 10 s 2000 c &
./client s 200 a 1 a 3 a 7 s 1300 r 6 &
./client s 1000 a 8 a 11 a 12 s 1000 p s 1000 p a 1 s 200 p \
| diff -uEb - _test/test5.in
if [ $? -eq 0 ] 
then
	echo "-- PASSED --"
else
	echo "-- FAILED --"
fi
./client e
sleep 0.5
echo
