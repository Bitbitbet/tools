#include <random>
#include <iostream>
#include <ctime>

int main() {
	auto seed = time(nullptr);
	std::mt19937 engine(seed);
	/* or: engine.seed(seed); */
	std::uniform_int_distribution<int> range(1, 100); // <int> is not necessary
	auto result = range(engine);
	std::cout << "generated: " << result << std::endl;

	return 0;
}
