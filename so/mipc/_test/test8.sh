#Tema 2 SO - Test 8
#Se testeaza daca s-au folosit anumite functii
echo "Started test 8 ..."
nm ../server | grep semop && nm ../client | grep semop && nm ../server | grep shmget && nm ../server | grep msgget
if [ $? -eq 0 ]
	then echo "-- PASSED --"
	else echo "-- FAILED --"
fi
echo
