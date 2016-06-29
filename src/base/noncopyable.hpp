/*
 * author:charlee
 */
#ifndef _NONCOPYABLE_HPP_
#define _NONCOPYABLE_HPP_

class noncopyable
{
protected:
	noncopyable() {}
	~noncopyable() {}

private: 
	noncopyable( const noncopyable& );
	noncopyable& operator=( const noncopyable& );
};

#endif
