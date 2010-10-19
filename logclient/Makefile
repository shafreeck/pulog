all:logtest test
logtest:logtest.cpp logclient.cpp logclient.h
	g++ -g -o logtest logtest.cpp logclient.cpp -lgearman
test:test.cpp
	g++ -g -o test test.cpp
clean:
	rm logtest
