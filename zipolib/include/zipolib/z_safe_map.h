#ifndef Z_SAFE_MAP
#define Z_SAFE_MAP
#include "zipolib/z_map.h"

#include <condition_variable>
#include <chrono>
using namespace std::chrono_literals;

template <typename KEY,typename OBJ,bool OWNER=false>
class z_safe_map
{
    std::mutex _mutex __attribute__ ((aligned (16)));
    std::condition_variable _cond __attribute__ ((aligned (16)));
private:
    bool _quit;

    z_obj_map_k<KEY,OBJ,OWNER> _map;

public:
    z_safe_map()
	{
		_quit = false;
	}
    virtual ~z_safe_map() {
        // member _map gets auto deleted

    }
	void init()
	{
		_quit = false;
	}

    OBJ* getobj(const KEY& key)
    {
        std::unique_lock<std::mutex> mlock(_mutex);
        return _map.getobj(key);

    }
    OBJ* get_wait_for(const KEY& key,int seconds,bool remove)
    {
        // this does not remove or delete the object
        std::unique_lock<std::mutex> mlock(_mutex);
        OBJ* obj=0;
        while (!_quit)
        {
            if (remove) {
                _map.pop(key,obj);

            }
            else {
                obj=_map.getobj(key);

            }
            if(obj)
                return obj;
            if(_cond.wait_for(mlock,1000ms*seconds)==std::cv_status::timeout)
                return nullptr;
        }
        return nullptr;
    }
    OBJ* get_wait(const KEY& key)
    {
        // this does not remove or delete the object
        std::unique_lock<std::mutex> mlock(_mutex);
        OBJ* obj=0;
        while (!_quit)
        {
            obj=_map.getobj(key);
            if(obj)
                break;
            _cond.wait(mlock);
        }
        return obj;
    }
    OBJ* pop(const KEY& key)
    {
        std::unique_lock<std::mutex> mlock(_mutex);
        OBJ* existing=0;
        _map.pop(key,existing);
        return existing;
    }
    bool pop_wait(const KEY& key,OBJ*& obj)
    {
        std::unique_lock<std::mutex> mlock(_mutex);
        obj=0;
        while (!_quit)
        {
            bool found=_map.pop(key,obj);
            if(found)
                break;
            _cond.wait(mlock);
        }
        //ZTF;

        if (_quit)
            return false;
        if(obj)
            return true;
        return false;
    }
    bool replace(const KEY& key, OBJ* obj)
    {
        if (_quit) // should this be at the top??
            return false;
        std::unique_lock<std::mutex> mlock(_mutex);
        OBJ* existing=0;
        if(_map.pop(key,existing))
        {
            delete existing;
        }
        _map.add(key,obj);
        _cond.notify_one();
        return true;
    }

	void quit() {
		//std::unique_lock<std::mutex> mlock(_mutex);
		//ZTF;
		_quit = true;
		_cond.notify_all();
	}

	void clear()
	{
		std::unique_lock<std::mutex> mlock(_mutex);
        _map.delete_all();
	}
	size_t get_count()
	{
		return _map.size();
	}

};

#endif