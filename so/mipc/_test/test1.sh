#Tema 2 SO - Test 1
#Se testeaza daca s-a implementat comanda exit (./client e)
echo "Started test 1 ..."
cd .. ; ./server ; sleep 0.5; ./client e; sleep 0.5
ps -e | grep " server"
if [ $? -eq 0 ]
	then echo "-- FAILED --"
	else echo "-- PASSED --"
fi
echo
