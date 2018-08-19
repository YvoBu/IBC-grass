all : gamma
	$(MAKE) -C src

gamma : gamma.cpp
	$(CXX) -g -Wall -std=c++14 $< -o $@

clean : 
	rm -f gamma
	$(MAKE) -C src clean
