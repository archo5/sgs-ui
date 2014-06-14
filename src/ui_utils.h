

#ifndef __SGS_UI_UTILS__
#define __SGS_UI_UTILS__


#include <sgs_cppbc.h>

#include <assert.h>
#include <vector>
#include <map>


#ifndef MAX
#  define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#  define MIN(a,b) ((a)<(b)?(a):(b))
#endif

template< class T > T TMAX( const T& a, const T& b ){ return a > b ? a : b; }
template< class T > T TMIN( const T& a, const T& b ){ return a < b ? a : b; }

template< class T > const T& VLASTOF( const std::vector<T>& x ){ return x[ x.size() - 1 ]; }
template< class T > T& VLASTOF( std::vector<T>& x ){ return x[ x.size() - 1 ]; }
template< class T> size_t VFIND( const std::vector<T>& x, const T& y )
{
	size_t i;
	for( i = 0; i < x.size(); ++i )
		if( x[i] == y )
			break;
	return i;
}
template< class T > void VREMOVEAT( std::vector<T>& x, size_t i ){ x.erase( x.begin() + i ); }
template< class T > void VSWAP( std::vector<T>& v, size_t i1, size_t i2 )
{
	T x = v[i1];
	v[i1] = v[i2];
	v[i2] = x;
}
template< class T > void VQSWAP( std::vector<T>& v, size_t i1, size_t i2 )
{
	char buf[ sizeof(T) ];
	memcpy( buf, &v[i1], sizeof(T) );
	memcpy( &v[i1], &v[i2], sizeof(T) );
	memcpy( &v[i2], buf, sizeof(T) );
}


struct IDGen
{
	IDGen( int32_t max_range = 0x7ffffff ) : m_maxRange( max_range )
	{
		Range r = { 0, m_maxRange };
		m_ranges.push_back( r );
	}

	int32_t GetID()
	{
		assert( m_ranges.size() );
		// pick the first range and process it..
		Range& r = m_ranges[ 0 ];
		uint32_t range = r.id1 - r.id0;
		uint32_t id = r.id0;
		if( !range )
			m_ranges.erase( m_ranges.begin() );
		else
			r.id0++;
		return id;
	}
	bool RequestID( int32_t id )
	{
		for( uint32_t i = 0; i < m_ranges.size(); ++i )
		{
			Range& r = m_ranges[ i ];
			if( r.id0 <= id && r.id1 >= id )
			{
				// remove this id
				if( r.id0 == id )
					if( r.id1 == id )
						m_ranges.erase( m_ranges.begin() + i );
					else
						r.id0++;
				else if( r.id1 == id )
					r.id1--;
				else // the rest of this range.. requires a split
				{
					Range r2 = { id + 1, r.id1 };
					r.id1 = id - 1;
					m_ranges.insert( m_ranges.begin() + ( i + 1 ), r2 );
				}
				return true;
			}
		}
		return false;
	}
	bool Used( int32_t id )
	{
		for( uint32_t i = 0; i < m_ranges.size(); ++i )
		{
			Range& r = m_ranges[ i ];
			if( r.id0 <= id && r.id1 >= id )
				return false;
		}
		return true;
	}
	void ReleaseID( int32_t id )
	{
		int32_t rplc = 0;
		for( ; rplc < (int32_t) m_ranges.size(); ++rplc )
			if( m_ranges[ rplc ].id0 > id )
				break;

		// add range
		Range rr = { id, id };
		m_ranges.insert( m_ranges.begin() + rplc, rr );

		// merge r!anges, from -1 to +1, relative
		int32_t rmin = MAX( rplc - 1, 0 );
		int32_t rmax = MIN( rplc + 1, m_maxRange );
		if( rmax >= (int32_t) m_ranges.size() )
			rmax = (int32_t) m_ranges.size() - 1;

		for( int32_t i = rmin; i < rmax; ++i )
		{
			if( m_ranges[ i ].id1 == m_ranges[ i + 1 ].id0 - 1 )
			{
				m_ranges[ i ].id1 = m_ranges[ i + 1 ].id1;
				m_ranges.erase( m_ranges.begin() + ( i + 1 ) );
				i--;
				rmax--;
			}
		}
	}
	void Clear()
	{
		m_ranges.resize( 0 );
		Range r = { 0, m_maxRange };
		m_ranges.push_back( r );
	}

protected:
	struct Range { int32_t id0, id1; };
	
	std::vector< Range > m_ranges;
	int32_t m_maxRange;
	
};



#endif // __SGS_UI_UTILS__

