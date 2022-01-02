#ifndef ___INTEGER_H___
#define ___INTEGER_H___

#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <vector>

class UIntegerOverflowException : std::exception {
public:
	const char *what() const noexcept override {
		return "UIntegerOverflowException";
	}
};

class UIntegerDividedByZeroException : std::exception {
public:
	const char *what() const noexcept override {
		return "UIntegerDividedByZeroException";
	}
};

class UInteger {
	friend std::ostream &operator<<(std::ostream &stream, UInteger i);
	friend std::istream &operator>>(std::istream &stream, UInteger &i);
	friend UInteger operator+(UInteger lfs, const UInteger &rfs) {lfs.m_add(rfs); return lfs;}
	friend UInteger operator-(UInteger lfs, const UInteger &rfs) {lfs.m_minus(rfs); return lfs;}
	friend UInteger operator*(UInteger lfs, const UInteger &rfs) {lfs.m_square(rfs); return lfs;}
	friend UInteger operator/(UInteger lfs, const UInteger &rfs) {lfs.m_divide(rfs); return lfs;}
	friend UInteger operator%(UInteger lfs, const UInteger &rfs) {lfs.m_mod(rfs); return lfs;}

	friend bool operator==(const UInteger &lfs, const UInteger &rfs) {return lfs.m_compare(rfs) == StrongOrdering::equal;}
	friend bool operator!=(const UInteger &lfs, const UInteger &rfs) {return lfs.m_compare(rfs) != StrongOrdering::equal;}
	friend bool operator<(const UInteger &lfs, const UInteger &rfs) {return lfs.m_compare(rfs) == StrongOrdering::less;}
	friend bool operator>(const UInteger &lfs, const UInteger &rfs) {return lfs.m_compare(rfs) == StrongOrdering::greater;}
	friend bool operator<=(const UInteger &lfs, const UInteger &rfs) {return lfs.m_compare(rfs) != StrongOrdering::greater;}
	friend bool operator>=(const UInteger &lfs, const UInteger &rfs) {return lfs.m_compare(rfs) != StrongOrdering::less;}
public:
	using num_type = int8_t;
	using size_type = uint32_t;
	UInteger();
	UInteger(const UInteger &integer_);
	UInteger(UInteger &&rvalue_);
	UInteger(char *s) : UInteger(const_cast<const char *>(s)) {}
	UInteger(const char *);
	UInteger(const std::string &s) : UInteger(s.c_str()) {}
	template<typename int_t> UInteger(const int_t &value_);

	~UInteger() = default;

	UInteger &operator+=(const UInteger &i) {m_add(i); return *this;}
	UInteger &operator-=(const UInteger &i) {m_minus(i); return *this;}
	UInteger &operator*=(const UInteger &i) {m_square(i); return *this;}
	UInteger &operator/=(const UInteger &i) {m_divide(i); return *this;}
	UInteger &operator%=(const UInteger &i) {m_mod(i); return *this;}

	num_type operator[](size_t index) const {return m_data[index];}

	UInteger &operator++();
	UInteger operator++(int);
	UInteger &operator--();
	UInteger operator--(int);

	UInteger &operator=(const UInteger &integer_) & = default;
	UInteger &operator=(UInteger &&rvalue_) & = default;
	UInteger &operator=(const char *) &;
	UInteger &operator=(const std::string &s) & {return operator=(s.c_str());}
	UInteger &operator=(char *s) & {return operator=(const_cast<const char *>(s));}
	template<typename int_t> UInteger &operator=(const int_t &) &;

	explicit operator bool() {return !is_zero();}

	bool is_zero() const {return m_data.size() == 1 && m_data[0] == 0;}
	bool is_one() const {return m_data.size() == 1 && m_data[0] == 1;}
	void zero() {m_data.resize(1); m_data[0] = 0;}
	void one() {m_data.resize(1); m_data[0] = 1;}

	size_type length() const {return m_data.size();}
	std::string to_string() const;

	const std::vector<num_type> &data() const {return m_data;}

	void swap(UInteger &i) {m_data.swap(i.m_data);}
private:
	enum class StrongOrdering : int8_t {
		less = -1, equal = 0, greater = 1
	};
	std::vector<num_type> m_data;

	StrongOrdering m_compare(const UInteger &) const;

	void m_self_increase();
	void m_self_decrease();

	void m_add(const UInteger &);
	void m_minus(const UInteger &);
	void m_square(const UInteger &);
	void m_divide(const UInteger &);
	void m_mod(const UInteger &);

	num_type &operator[](size_t index) {return m_data[index];}

	void initialize(const char *);
};


template<typename int_t> UInteger::UInteger(const int_t &value_) {
	std::string str = std::to_string(value_);
	m_data.reserve(str.size());
	for(auto iter = str.rbegin(); iter != str.rend(); ++iter) {
		m_data.push_back(*iter - '0');
	}
}

template<typename int_t>
UInteger &UInteger::operator=(const int_t &value_) & {
	std::string str = std::to_string(value_);
	m_data.resize(str.size());
	for(size_t i = 0; i < length(); ++i) {
		m_data[i] = str[length() - i - 1] - '0';
	}
	return *this;
}

namespace std {
	template<> inline void swap<UInteger>(UInteger &a, UInteger &b) {
		a.swap(b);
	}
}

#endif
