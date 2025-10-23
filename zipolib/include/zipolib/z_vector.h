/*________________________________________________________________________

 z_stl_vector_h

________________________________________________________________________*/

#ifndef z_stl_vector_h
#define z_stl_vector_h


#include "zipolib/zipo.h"



template <class ITEM_CLASS , bool OWNER=true> class z_obj_vector : public std::vector<ITEM_CLASS*>
{
protected:
public:

	typedef std::vector<ITEM_CLASS*> vect;

	virtual ~z_obj_vector()
	{
		if(OWNER)
		destroy();
	}


	virtual void* get_next(size_t &i)
	{
		void* v = get(i);
		i++;
		return v;
	}
	virtual z_status remove(size_t i)
	{
		vect::erase(vect::begin() + i);
		return zs_ok;
	}

	void add_void(void* v)
	{
		ITEM_CLASS* i = reinterpret_cast<ITEM_CLASS*>(v);
		add(i);
	}

	void add(ITEM_CLASS* item)
	{
		vect::push_back(item);
		return;
	}
	void destroy()
	{
		if (OWNER)
		{
			size_t i;
			for (i = 0; i<vect::size(); i++)
			{
				ITEM_CLASS* item = get(i);
				if (item)
					delete item;
			}
		}

		vect::clear();
	}



	ITEM_CLASS* get(size_t i)  const
	{
		if (i >= vect::size())
			return 0;
		return (*this)[i];
	}


	void* get_void(size_t i)
	{
		return (void*)get(i);
	}
	virtual z_obj_vector<ITEM_CLASS, OWNER> & operator << (ITEM_CLASS *x)
	{
		vect::push_back(x);
		return *this;
	};


};


template <class ITEM_CLASS > class z_obj_vector_encap
{
protected:
	std::vector<ITEM_CLASS*>  _vector;
public:

	typedef typename std::vector<ITEM_CLASS*>::iterator iter;

	virtual ~z_obj_vector_encap()
	{
		destroy();
	}

	typename  std::vector<ITEM_CLASS*>::iterator begin()
	{
		return _vector.begin();
	}
	iter end()
	{
		return _vector.end();
	}
	virtual void* get_next(size_t &i)
	{
		void* v=get(i);
		i++;
		return v;
	}
	virtual z_status remove(size_t i)
	{
		_vector.erase(_vector.begin()+i);
		return zs_ok;
	}

 	void add_void(void* v)
	{
		ITEM_CLASS* i=reinterpret_cast<ITEM_CLASS*>(v);
		add(i);
	}
	void add(ITEM_CLASS* item)
	{
		_vector.push_back(item);
		return ;
	}
	void destroy()
	{ 
		size_t i;
		for(i=0;i<size();i++)
		{
			ITEM_CLASS* item=get(i);
			if(item)
				delete item;
		}
		_vector.clear(); 
	}

	void clear()
	{ 
		_vector.clear(); 
	}

 	size_t size()	const
	{ 
		return _vector.size(); 
	}
	
	ITEM_CLASS* operator [](size_t i)  	const
	{
		if(i>=size())
			return 0;
		return _vector[i];
	}
	ITEM_CLASS* get(size_t i)  const
	{
		if(i>=size())
			return 0;
		return _vector[i];
	}


 	void* get_void(size_t i) 
	{
		return (void*)get(i);
	}
    virtual z_obj_vector<ITEM_CLASS> & operator << (ITEM_CLASS *x)
    {
		_vector.push_back(x);
		return *this;
    };


};


#endif
