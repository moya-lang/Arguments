default, all:
	mkdir -p Build
	g++ --std=c++17 -Wall Example1.cpp -o Build/example1
	g++ --std=c++17 -Wall Example2.cpp -o Build/example2
	g++ --std=c++17 -Wall Example3.cpp -o Build/example3

clean:
	$(RM) -rf Build
