
make -n VERBOSE=1 | bear parse-sh
make

clear
echo "----- RUNNING TESTS -----"
echo
../build/debug/bin/angel_tests
echo
exit 1
