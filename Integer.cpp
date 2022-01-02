#include "Integer.h"

#include <cstring>
#include <algorithm>

std::ostream &operator<<(std::ostream &stream, UInteger i) {
	for(size_t index = 0; index < i.length(); ++index) {
		stream.put(i[i.length() - index - 1] + '0');
	}
	return stream;
}
std::istream &operator>>(std::istream &stream, UInteger &i) {
	char result;
	while((result = stream.peek()), result == '\n' || result == ' ' || result == '\t') stream.ignore();

	if(result > '9' || result < '0' || !stream) {
		i.zero();
		stream.setstate(std::ios::failbit);
		return stream;
	}
	std::vector<UInteger::num_type> temp;
	while((result = stream.get()), result >= '0' && result <= '9') {
		temp.push_back(result - '0');
	}
	std::reverse(temp.begin(), temp.end());
	i.m_data = std::move(temp);
	return stream;
}

auto UInteger::m_compare(const UInteger &i) const -> StrongOrdering {
	if(length() < i.length()) return StrongOrdering::less;
	else if(length() > i.length()) return StrongOrdering::greater;
	size_t index = length();
	while(index != 0) {
		if(m_data[index - 1] != i.m_data[index - 1]) {
			return m_data[index - 1] < i.m_data[index - 1] ? StrongOrdering::less : StrongOrdering::greater;
		}
		--index;
	}
	return StrongOrdering::equal;
}

void UInteger::m_self_increase() {
	++m_data[0];
	size_type index = 0;
	while(m_data[index] > 9) {
		m_data[index] -= 10;
		if(m_data.size() > index + 1) {
			m_data[index + 1] += 1;
		} else {
			m_data.push_back(1);
		}
		++index;
	}
}
void UInteger::m_self_decrease() {
	--m_data[0];
	size_type index = 0;
	while(m_data[index] < 0) {
		m_data[index] += 10;
		if(index + 1 >= length()) {
			throw UIntegerOverflowException();
		}
		m_data[index + 1] -= 1;
		++index;
	}
	if(length() != 1 && m_data[length() - 1] == 0) {
		m_data.pop_back();
	}
}

void UInteger::m_add(const UInteger &integer) {
	if(integer.length() > length()) {
		m_data.resize(integer.length(), 0);
	}
	for(size_t index = 0; index < length(); ++index) {
		if(index < integer.length()) {
			m_data[index] += integer.m_data[index];
		} else if(m_data[index] < 10) {
			break;
		}
		if(m_data[index] > 9) {
			m_data[index] -= 10;
			if(index == length() - 1) {
				m_data.push_back(1);
				return;
			} else {
				m_data[index + 1] += 1;
			}
		}
	}
}

void UInteger::m_minus(const UInteger &integer) {
	StrongOrdering compare_result = m_compare(integer);
	if(compare_result == StrongOrdering::less) {
		throw UIntegerOverflowException();
	}
	if(compare_result == StrongOrdering::equal) {
		zero();
		return;
	}

	for(size_t index = 0; index < length(); ++index) {
		if(index < integer.length()) {
			m_data[index] -= integer.m_data[index];
		} else if(m_data[index] >= 0) {
			break;
		}
		if(m_data[index] < 0) {
			m_data[index] += 10;
			m_data[index + 1] -= 1;
		}
	}
	size_t zeros = 0;
	while(m_data[length() - zeros - 1] == 0) {
		++zeros;
	}
	m_data.resize(length() - zeros);
}

void UInteger::m_square(const UInteger &integer) {
	if(is_zero() || integer.is_zero()) {
		zero();
		return;
	}
	UInteger result;
	result.m_data.resize(length() + integer.length(), 0);

	for(size_t i = 0; i < length(); ++i) {
		for(size_t j = 0; j < integer.length(); ++j) {
			num_type n = m_data[i] * integer.m_data[j];
			result[i + j] += n % 10;
			result[i + j + 1] += n / 10;
			if(result[i + j] > 9) {
				result[i + j] -= 10;
				result[i + j + 1] += 1;
			}
			if(result[i + j + 1] > 9) {
				result[i + j + 1] -= 10;
				result[i + j + 2] += 1;
			}
		}
	}
	size_t zeros = 0;
	while(result.m_data[result.length() - zeros - 1] == 0) {
		++zeros;
	}
	result.m_data.resize(result.length() - zeros);

	*this = std::move(result);
}

void UInteger::m_divide(const UInteger &integer) {
	if(integer.is_zero()) {
		throw UIntegerDividedByZeroException();
	}
	if(is_zero()) {
		return;
	}
	StrongOrdering compare_result = m_compare(integer);
	if(compare_result == StrongOrdering::less) {
		zero();
		return;
	} else if(compare_result == StrongOrdering::equal) {
		one();
		return;
	}
	UInteger result;
	result.m_data.resize(length() + integer.length(), 0);

	for(size_t i = 0; i < length(); ++i) {
	}

	size_t zeros = 0;
	while(result.m_data[result.length() - zeros - 1] == 0) {
		++zeros;
	}
	result.m_data.resize(result.length() - zeros);

	*this = std::move(result);
}

void UInteger::initialize(const char *sstr) {
	size_t prefix_zero = 0;
	while(sstr[prefix_zero] == '0') {
		++prefix_zero;
	}
	const char *str = sstr + prefix_zero;
	size_t len = strlen(str);
	if(len == 0) {
		zero();
		return;
	}
	m_data.clear();
	m_data.resize(len, 0);
	for(size_t index = 0; index < len; ++index) {
		if(str[index] >= '0' && str[index] <= '9') {
			m_data[len - index - 1] = str[index] - '0';
		} else {
			struct NotANumberException : std::exception {
				const char *what() const noexcept override {
					return "Not A Number Exception";
				}
			};
			throw NotANumberException();
		}
	}
}


UInteger::UInteger() : m_data(1, 0) {
	
}
UInteger::UInteger(const char *sstr) {
	initialize(sstr);
}

UInteger::UInteger(const UInteger &integer_) : m_data(integer_.m_data) {}

UInteger::UInteger(UInteger &&rvalue_) {
	m_data.swap(rvalue_.m_data);
}

UInteger &UInteger::operator++() {
	m_self_increase();
	return *this;
}
UInteger UInteger::operator++(int) {
	UInteger i = *this;
	m_self_increase();
	return i;
}
UInteger &UInteger::operator--() {
	m_self_decrease();
	return *this;
}
UInteger UInteger::operator--(int) {
	UInteger i = *this;
	m_self_decrease();
	return i;
}

UInteger &UInteger::operator=(const char *sstr) & {
	initialize(sstr);
	return *this;
}

std::string UInteger::to_string() const {
	std::string result;
	result.reserve(length());
	for(size_type i = 0; i < length(); ++i) {
		result.push_back(m_data[length() - i - 1] + '0');
	}
	return result;
};
