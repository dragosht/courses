#Tema 2 SO - Test 2
#Se testeaza daca a fost implementata comanda sleep in client
echo "Started test 2 ..."
cd ..
./server
sleep 0.5
(time -p ./client s 1000) 2> ./_test/_sleep.tmp
cd _test
perl sleep.pl 1 < _sleep.tmp
if [ $? -eq 0 ] 
	then echo "-- PASSED --"
	else echo "-- FAILED --"
fi
rm -f _sleep.tmp
cd ..
./client e
sleep 0.5
echo
