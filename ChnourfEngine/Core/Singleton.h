#pragma once

template <class T> class Singleton
{
public:
	inline static void Create()
	{
		if (!ourInstance)
		{
			ourInstance = new T;
		}
	}

	inline static void Destroy()
	{
		if (ourInstance)
		{
			delete ourInstance;
			ourInstance = nullptr;
		}
	}

	inline static T* GetInstance()
	{
		return ourInstance;
	}

	Singleton(Singleton const&) = delete;              // Don't Implement
	void operator=(Singleton const&) = delete;

protected:
	Singleton() {};
	static T* ourInstance;

};

template <class T> T* Singleton<T>::ourInstance = nullptr;