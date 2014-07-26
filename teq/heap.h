#ifndef LIBTEQ_HEAP_HH
#define LIBTEQ_HEAP_HH

#include <memory>
#include <iostream>

namespace teq
{
	struct heap_base
	{
		virtual void gc() = 0;
	};

	template <class T>
	struct heap : public heap_base
	{
		typedef std::shared_ptr<T> T_ptr;
		
		std::list<T_ptr> m_heap;
		
		T_ptr add(T_ptr ptr)
		{
			m_heap.push_back(ptr);
			return ptr;
		}
		
		T_ptr add_new(T &&t)
		{
			return add(std::make_shared<T>(t));
		}
		
		void gc()
		{
			m_heap.remove_if(std::mem_fun_ref(&T_ptr::unique));
		}
	};
} // namespace
#endif
