make -C ./tests clean
make -C ./tests

clear
echo "----- RUNNING TESTS -----"
echo
./build/debug/bin/angel_tests
echo
exit 1
