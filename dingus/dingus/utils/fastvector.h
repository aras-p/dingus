// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __FAST_VECTOR_H
#define __FAST_VECTOR_H

namespace dingus {

template< class _Ty, class _A = std::allocator<_Ty> >
class fastvector {
public:
	typedef typename std::vector<_Ty, _A>	vector_type;
public:
	typedef typename fastvector<_Ty, _A>	this_type;

	typedef typename vector_type::allocator_type allocator_type;
	typedef typename vector_type::size_type size_type;
	typedef typename vector_type::difference_type difference_type;
	typedef typename vector_type::reference reference;
	typedef typename vector_type::const_reference const_reference;
	typedef typename vector_type::value_type value_type;

	typedef typename vector_type::iterator iterator;
	typedef typename vector_type::const_iterator const_iterator;
	typedef typename vector_type::reverse_iterator reverse_iterator;
	typedef typename vector_type::const_reverse_iterator const_reverse_iterator;

private:
	vector_type	v;

public:
	explicit fastvector( const _A& alloc = _A() )
		: v(alloc) { }
	explicit fastvector( size_type num, const _Ty& val = _Ty(), const _A& alloc = _A() )
		: v(num,val,alloc) { }
	fastvector( const this_type& other )
		: v(other.v) { }
	fastvector( const_iterator first, const_iterator last, const _A& alloc = _A() )
		: v(first,last,alloc) { }
	~fastvector() { }

	this_type& operator=( const this_type& other ) { v = other.v; return *this; }
	void reserve( size_type num ) { v.reserve(num); }
	size_type capacity() const { return v.capacity(); }
	iterator begin() { return v.begin(); }
	const_iterator begin() const { return v.begin(); }
	iterator end() { return v.end(); }
	const_iterator end() const { return v.end(); }
	reverse_iterator rbegin() { return v.rbegin(); }
	const_reverse_iterator rbegin() const { return v.rbegin(); }
	reverse_iterator rend() { return v.rend(); }
	const_reverse_iterator rend() const { return v.rend(); }
	void resize( size_type num, const _Ty& val = _Ty() ) { v.resize(num,val); }
	size_type size() const { return v.size(); }
	size_type max_size() const { return v.max_size(); }
	bool empty() const { return v.empty(); }
	_A get_allocator() const { return v.get_allocator(); }
	const_reference at( size_type pos ) const { return v.at(pos); }
	reference at( size_type pos ) { return v.at(pos); }
	const_reference operator[]( size_type pos ) const { return v[pos]; }
	reference operator[]( size_type pos ) { return v[pos]; }
	reference front() { return v.front(); }
	const_reference front() const { return v.front(); }
	reference back() { return v.back(); }
	const_reference back() const { return v.back(); }
	void push_back( const _Ty& val ) { v.push_back(val); }
	void pop_back() { v.pop_back(); }
	void assign( const_iterator first, const_iterator last ) { v.assign(first,last); }
	void assign( size_type num, const _Ty& val = _Ty() ) { v.assign(num,val); }
	
	iterator insert( iterator pos, const _Ty& val = _Ty() ) { return v.insert(pos,val); }
	void insert( iterator pos, size_type num, const _Ty& val ) { v.insert(pos,num,val); }
	void insert( iterator pos, const_iterator first, const_iterator last ) { v.insert(pos,first,last); }

	/**
	 *  Places last in place of pos.
	 *  It's fast, but doesn't preserve the order.
	 */
	iterator erase( iterator pos ) {
		*pos = back();
		pop_back();
		assert( pos >= v.begin() && pos <= v.end() );
		return pos;
	}

	void remove( const _Ty& val ) {
		iterator last = end();
		for( iterator first = begin(); first != last; )
			if( *first == val )
				first = erase(first), --last;
			else
				++first;
	};
	
	iterator erase( iterator first, iterator last ) {
		return v.erase(first,last);
	}
	void clear() { v.clear(); }

	void swap( this_type& other ) { v.swap( other.v ); }
};

}; // namespace

#endif
