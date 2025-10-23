#ifndef Z_SAFE_Q
#define Z_SAFE_Q
#include "zipolib/zipolib.h"

#include <condition_variable>


template <typename T>
class z_safe_queue
{
public:
	z_safe_queue()
	{
	    _wait_on_empty = true;
	}
	void wait_disable() {
	    //std::unique_lock<std::mutex> mlock(_mutex);
	    //ZTF;
	    _wait_on_empty = false;
	    _cond.notify_all();
	}
	void wait_enable()
	{
	    _wait_on_empty = true;
	}
	bool pop_wait(T& item)
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		//ZTF;

		while (_queue.empty() && (_wait_on_empty))
		{
			_cond.wait(mlock);
		}
		//ZTF;

		if (_queue.empty())
		{
		    return false;

		}

		item = _queue.front();
		_queue.pop();

		return true;
	}
	bool pop()
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		if (_queue.empty())
			return false;
		_queue.pop();
		return true;
	}
	bool pop(T& item)
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		if (_queue.empty())
			return false;
		item = _queue.front();
		_queue.pop();
		return true;
	}
	bool front(T& item)
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		if (_queue.empty())
			return false;
		item = _queue.front();
		return true;
	}


	bool push(T& item)
	{
	    /*
	     *
	    if (_quiting) // should this be at the top??
			return false;
	     */
		std::unique_lock<std::mutex> mlock(_mutex);
		//ZTF;

		_queue.push(item);

		_cond.notify_one();
	
		return true;
	}
	void clear()
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		while(_queue.empty()==false)
			_queue.pop();
	}
	size_t get_count()
	{
		return _queue.size();
	}
private:
	bool _wait_on_empty=true;

	std::queue<T> _queue;
	std::mutex _mutex;
	std::condition_variable _cond;
};
#if 0 
class QueueTest
{
public:

	QueueTest()
	{

		_counter = 0;
		_check = 0;
		_delay = 500;
		_show = false;
	}

	z_safe_queue<int> _q;
	int _delay;
	int _counter;
	int _check;
	bool _show;
	std::thread _produce;
	std::thread _consume;

	void produce() {
		bool running = true;
		while (running)
		{
			running = _q.push(_counter);
			if(_show)
			std::cout << "pushed=" << _counter << std::endl;
			_counter++;
			if (!running)
				break;
			std::this_thread::sleep_for(std::chrono::milliseconds{ _delay });
		}
		//	std::this_thread::sleep_for(std::chrono::milliseconds{ 500 });
	}
	void consume() {
		int val;
		while (_q.pop(val))
		{
			if (_show)
				std::cout << "popped=" << val << std::endl;
			if (val != _check)
			{
				_q.stop();
				std::cout << "Error!" << std::endl;
				std::cout << "_counter=" << _counter << std::endl;
				std::cout << "_check=" << _check << std::endl;
				std::cout << "val=" << val << std::endl;
				break;
			}

			_check++;
		}
		//	std::this_thread::sleep_for(std::chrono::milliseconds{ 500 });
	}
	z_status start()
	{
		_q.start();

		_produce = std::thread(&QueueTest::produce, this);
		_consume = std::thread(&QueueTest::consume, this);

		std::this_thread::sleep_for(std::chrono::milliseconds{ 2000 });
		_q.stop();
		_produce.join();
		_consume.join();
		std::cout << "_counter=" << _counter << std::endl;
		std::cout << "_check="<< _check << std::endl;


		return zs_ok;
	}

	



};

#endif
#endif