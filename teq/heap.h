#ifndef LIBTEQ_HEAP_HH
#define LIBTEQ_HEAP_HH

#include <memory>
#include <iostream>

namespace teq
{
	template <class T>
	struct heap
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
			T_ptr ptr = std::make_shared<T>(t);
			return add(ptr);
		}
		
		void gc()
		{
			for (auto it = m_heap.begin(); it != m_heap.end();) {
				if (it->unique()) {
					// std::cout << "Erasing..." << std::endl;
					it = m_heap.erase(it);
				} else {
					++it;
				}
			}	
		}
	};
} // namespace
#endif