#include <cstdio>
#include <cstdlib>
#include <random>
#include <ctime>

int main(int argc, char **argv) {
	long seed;
	if(argc > 1) {
		seed = strtol(argv[1], nullptr, 10);
	} else {
		seed = time(nullptr);
	}
	printf("Generate 100 UUIDs with seed %ld\n\n", seed);
	std::mt19937 engine(seed);
	char map[] {	'0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	auto generate = [&engine, map] () {
		char result[37];
		result[8] = '-';
		result[13] = '-';
		result[18] = '-';
		result[23] = '-';
		result[36] = 0;
		uint32_t a = engine();
		uint32_t b = engine();
		uint32_t c = engine();
		uint32_t d = engine();
		result[0] =  map[a >> 28];
		result[1] =  map[(a << 4) >> 28];
		result[2] =  map[(a << 8) >> 28];
		result[3] =  map[(a << 12) >> 28];
		result[4] =  map[(a << 16) >> 28];
		result[5] =  map[(a << 20) >> 28];
		result[6] =  map[(a << 24) >> 28];
		result[7] =  map[(a << 28) >> 28];
		result[9] =  map[b >> 28];
		result[10] = map[(b << 4) >> 28];
		result[11] = map[(b << 8) >> 28];
		result[12] = map[(b << 12) >> 28];
		result[14] = '4';
		result[15] = map[(b << 20) >> 28];
		result[16] = map[(b << 24) >> 28];
		result[17] = map[(b << 28) >> 28];
		result[19] = map[(c >> 30) + 8];
		result[20] = map[(c << 4) >> 28];
		result[21] = map[(c << 8) >> 28];
		result[22] = map[(c << 12) >> 28];
		result[24] = map[(c << 16) >> 28];
		result[25] = map[(c << 20) >> 28];
		result[26] = map[(c << 24) >> 28];
		result[27] = map[(c << 28) >> 28];
		result[28] = map[d >> 28];
		result[29] = map[(d << 4) >> 28];
		result[30] = map[(d << 8) >> 28];
		result[31] = map[(d << 12) >> 28];
		result[32] = map[(d << 16) >> 28];
		result[33] = map[(d << 20) >> 28];
		result[34] = map[(d << 24) >> 28];
		result[35] = map[(d << 28) >> 28];
		printf("%s\n", result);
	};
	for(int i = 0; i < 100; ++i) {
		generate();
	}
	return 0;
}
