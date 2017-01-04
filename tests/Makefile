all : test1.exe test2.exe test3.exe

test1.exe : test_scheduler1.cpp ../scheduler.h 
	g++ -pthread test_scheduler1.cpp -std=c++14 -o test1.exe


test2.exe : test_scheduler2.cpp ../scheduler.h 
	g++ -pthread test_scheduler2.cpp -std=c++14 -o test2.exe

test3.exe : test_scheduler3.cpp ../scheduler.h 
	g++ -pthread test_scheduler3.cpp -std=c++14 -o test3.exe

clean : 
	rm test*.exe

