#pragma once
#include <functional>
#include <vector>

template<typename Signature> class SlotInternal;

template<typename arg1>
class SlotInternal<void(arg1)>
{
protected:
	std::vector<std::function<void(arg1)>> myCallbacks;

	inline void ConnectInternal(std::function<void(arg1)> aFunction)
	{
		myCallbacks.push_back(aFunction);
	}
public:
	void operator()(arg1 x1) const
	{
		for (auto i = 0; i < myCallbacks.size(); ++i)
		{
			if (myCallbacks[i])
				myCallbacks[i](x1);
		}
	}
};

template<typename arg1, typename arg2>
class SlotInternal<void(arg1, arg2)>
{
protected:
	std::vector<std::function<void(arg1, arg2)>> myCallbacks;

	inline void ConnectInternal(std::function<void(arg1, arg2)> aFunction)
	{
		myCallbacks.push_back(aFunction);
	}
public:
	void operator()(arg1 x1, arg2 x2) const
	{
		for (auto i = 0; i < myCallbacks.size(); ++i)
		{
			if (myCallbacks[i])
				myCallbacks[i](x1, x2);
		}
	}
};

template<typename arg1, typename arg2, typename arg3>
class SlotInternal<void(arg1, arg2, arg3)>
{
protected:
	std::vector<std::function<void(arg1, arg2, arg3)>> myCallbacks;

	inline void ConnectInternal(std::function<void(arg1, arg2, arg3)> aFunction)
	{
		myCallbacks.push_back(aFunction);
	}
public:
	void operator()(arg1 x1, arg2 x2, arg3 x3) const
	{
		for (auto i = 0; i < myCallbacks.size(); ++i)
		{
			if (myCallbacks[i])
				myCallbacks[i](x1, x2, x3);
		}
	}
};

template<typename arg1, typename arg2, typename arg3, typename arg4>
class SlotInternal<void(arg1, arg2, arg3, arg4)>
{
protected:
	std::vector<std::function<void(arg1, arg2, arg3, arg4)>> myCallbacks;

	inline void ConnectInternal(std::function<void(arg1, arg2, arg3, arg4)> aFunction)
	{
		myCallbacks.push_back(aFunction);
	}
public:
	void operator()(arg1 x1, arg2 x2, arg3 x3, arg4 x4) const
	{
		for (auto i = 0; i < myCallbacks.size(); ++i)
		{
			if (myCallbacks[i])
				myCallbacks[i](x1, x2, x3, x4);
		}
	}
};


template <typename Signature>
class Slot : public SlotInternal<Signature>
{
public:
	inline void Connect(std::function<Signature> aFunction)
	{
		return ConnectInternal(aFunction);
	}
};