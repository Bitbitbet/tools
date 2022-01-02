#include <cstdio>
#include <cstdint>

void printb(uint8_t data) {
	putchar(data & 0b10000000 ? '1' : '0');
	putchar(data & 0b01000000 ? '1' : '0');
	putchar(data & 0b00100000 ? '1' : '0');
	putchar(data & 0b00010000 ? '1' : '0');
	putchar(data & 0b00001000 ? '1' : '0');
	putchar(data & 0b00000100 ? '1' : '0');
	putchar(data & 0b00000010 ? '1' : '0');
	putchar(data & 0b00000001 ? '1' : '0');
}
void printx(uint8_t data) {
	const static char qualifiers[] = {'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'A', 'B',
		'C', 'D', 'E', 'F'};
	putchar(qualifiers[data >> 4]);
	putchar(qualifiers[data & 0x0F]);
}

int main() {
	for(unsigned char c = 0; c <= 127; ++c) {
		printf("\t%d\t", static_cast<int>(c));
		printx(c);
		printf(" ");
		printb(c);
		if(c > 32) {
			printf(": %c", c);
		}
		printf("\n");
	}
	return 0;
}
