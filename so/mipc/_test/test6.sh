#Tema 2 SO - Test 6
#Test complex cu mai multi clienti in acelasi timp
echo "Started test 6 ..."
cd ..
./server
sleep 0.5
./client s 10 a 6 a 3 p s 5 a 10 p a 0 a 21 r 3 s 5 a 14 p a 7 p a 9 >/dev/null &
./client s 5 r 6 p a 15 a -2 a 30 p s 5 p r 30 p a 31 a 22 a 2 p s 5 r 22 a 31 a 13 p >/dev/null &
./client a 14 a 16 p s 5 r 0 a 4 p a 25 a -5 a 17 s 5 a 37 a 26 p s 5 p a -22 a 44 a 21 >/dev/null &
./client p | perl _test/tree.pl _test/test6_1.in
temp=$?
./client s 2000 p | perl _test/tree.pl _test/test6_2.in
if [ $? -eq 0 -a $temp -eq 0 ] 
then
	echo "-- PASSED --"
else
	echo "-- FAILED --"
fi
./client e
sleep 0.5
echo
