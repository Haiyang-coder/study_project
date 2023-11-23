#include "pch.h"
#include "SafeQueue.h"
template<class T>
inline CSafeQueue<T>::CSafeQueue()
{
}

template<class T>
CSafeQueue<T>::~CSafeQueue()
{
}

template<class T>
bool CSafeQueue<T>::PushBack(const T& data)
{
	return false;
}

template<class T>
bool CSafeQueue<T>::PopFront(T& data)
{
	return false;
}

template<class T>
size_t CSafeQueue<T>::Size()
{
	return size_t();
}

template<class T>
void CSafeQueue<T>::Clear()
{
}

template<class T>
void CSafeQueue<T>::threadMain()
{
}
