all : test1.exe test2.exe

test1.exe : test_scheduler1.cpp
	g++ test_scheduler1.cpp -std=c++11 -o test1.exe


test2.exe : test_scheduler2.cpp
	g++ test_scheduler2.cpp -std=c++11 -o test2.exe

clean : 
	rm test*.exe

