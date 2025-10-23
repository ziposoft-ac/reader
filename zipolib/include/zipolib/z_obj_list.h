/*________________________________________________________________________

 z_obj_list_h

________________________________________________________________________*/

#ifndef z_obj_list_h
#define z_obj_list_h


#include "zipolib/zipo.h"



template <class ITEM , bool OWNER=true> class z_obj_list : public std::list<ITEM*>
{
protected:
public:

	typedef std::list<ITEM*> list;

	virtual ~z_obj_list()
	{
		if(OWNER)
		destroy();
	}

	/**
	 * Iterates through all items and calls callback
	 *
	 * Returning true from callback erases and deletes the pointer
	 * @param callback
	 */
	void filter_out(std::function<bool(ITEM*)> callback) {
		typename list::iterator iter = list::begin();
		typename list::iterator end  = list::end();

		while (iter != end)
		{
			ITEM * pItem = *iter;

			if (callback(pItem)) {
				iter = list::erase(iter);
				delete pItem;
			}
			else
				++iter;

		}


	}

	void destroy()
	{
		if (OWNER)
		{
            for(auto i : *this)
            {
                delete i;
            }
		}

		list::clear();
	}





};




#endif
