#ifndef __ARRAY_LIST_H__
#define __ARRAY_LIST_H__

namespace al_node {
	template <class T> struct Node {
		T *value;
		Node<T> *next, *perv;
	};
}

template <class T> class ArrayList {
private:
	al_node::Node<T> *head = nullptr, *tail = nullptr, *p = nullptr;
	int size = 0;
public:
	ArrayList();
	ArrayList(int n, const T *given_list);
	bool add(T &value);
	bool add(T &&value);
	bool add_front(T &value);
	bool add_front(T &&value);
	bool add_after(int pos, T &value);
	bool add_after(int pos, T &&value);
	bool remove(int n);
	int get_size();
};
#endif

#ifndef __ARRAY_LIST_CPP__
#define __ARRAY_LIST_CPP__

#include <cstdio>
#include <utility>

template <class T> ArrayList<T>::ArrayList() {
	
}

template <class T> bool ArrayList<T>::add(T &&value) {
	return add_after(get_size() - 1, value);
}

template <class T> bool ArrayList<T>::add_after(int pos, T &&value) {
	printf("%d %d\n", pos, get_size());
	if(pos >= get_size()) {
		return false;
	}
	if(get_size() == 0) {
		head = new al_node::Node<T>;
	}
	size ++;
	return true;
}

template <class T> bool ArrayList<T>::add(T &value) {
	return add(std::move(value));
}
template <class T> bool ArrayList<T>::add_after(int pos, T &value) {
	return add_after(pos, std::move(value));
}

template <class T> bool ArrayList<T>::remove(int n) {
	return true;
}

template <class T> int ArrayList<T>::get_size() {
	return size;
}

#endif
