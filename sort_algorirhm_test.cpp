#include <algorithm>
#include <cassert>
#include <chrono>
#include <iterator>
#include <vector>
#include <array>
#include <ctime>
#include <iostream>
#include <random>

using namespace std;

constexpr size_t LENGTH = 700;
constexpr size_t TEST_REPEAT_TIME = 2000;

template<typename iter_t>
void print_sequence(iter_t begin, iter_t end, ostream &output = cout) {
	using traits = iterator_traits<iter_t>;
	using size_type = size_t;
	size_type length = 0;
	if constexpr(is_same_v<typename traits::iterator_category, random_access_iterator_tag>) {
		length = end - begin;
	} else {
		for(iter_t t = begin; t != end; ++t, ++length);
	}

	if (length == 0) {
		output << "0 element\n";
		return;
	}
	else if(length == 1) {
		output << "1 element: {" << *begin << "}\n";
		return;
	}
	output << length << " elements : {";
	bool first = true;
	for(; begin != end; ++begin) {
		if(first)
			first = false;
		else
			output << ", ";
		output << *begin;
	}
	output << "}\n";
}

using number_t = int;
using index_t = std::vector<number_t>::size_type;

void std_sort(vector<number_t> &v) {
	std::sort(v.begin(), v.end());
}

void insertion_sort(vector<number_t> &v) {
	if(v.size() < 2) return;
	for(index_t i = 1; i < v.size(); ++i) {
		if(v[i] >= v[i - 1]) continue;
		for(index_t j = 0; j < i; ++j) {
			if (v[i] <= v[j]) {
				// insert v[i] to index j
				auto temp = v[i];
				for (auto k = i; k-- > j;) { // k ranges from i - 1 to j
					v[k + 1] = v[k];
				}
				v[j] = temp;
				break;
			}
		}
	}
}

void bubble_sort(vector<number_t> &v) {
	if(v.size() < 2) return;
	for(index_t i = v.size() - 1; i > 0; --i) {
		bool flag = false;
		for(index_t j = 0; j < i; ++j) {
			if(v[j] > v[j + 1]) {
				swap(v[j], v[j + 1]);
				flag = true;
			}
		}
		if(!flag) break;
	}
}

auto generate_unsorted_sequence() {
	static std::mt19937 engine(time(nullptr));
	uniform_int_distribution range(0, 99999);
	array<vector<number_t>, TEST_REPEAT_TIME> output;
	for(auto &sequence: output) {
		sequence.reserve(LENGTH);
		for(size_t i = 0; i < LENGTH; ++i)
			sequence.push_back(range(engine));
	}
	return output;
}

auto test(const char *name, auto functor, array<vector<number_t>, TEST_REPEAT_TIME> unsorted) {
	static chrono::system_clock clock;
	cout << name << ": " << flush;
	auto t1 = clock.now();

	for(auto &v: unsorted) {
		functor(v);
	}

	auto t2 = clock.now();
	auto milliseconds = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
	cout << milliseconds << " ms" << endl;

	return unsorted;
}

#define test_algorithm(name, function_name, unsorted) \
	if(test((name), (function_name), (unsorted)) == sorted) \
		cout << (name) << " Passed!\n"; \
	else \
		cout << (name) << " Not passed!\n"

int main() {
	auto unsorted = generate_unsorted_sequence();

	auto sorted = test("Standard Template Library Sort", std_sort, unsorted);
	test_algorithm("Bubble Sort", bubble_sort, unsorted);
	test_algorithm("Insertion Sort", insertion_sort, unsorted);
	return 0;
}
