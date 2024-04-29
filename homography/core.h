#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <random>
#include <numeric>
#include <algorithm>

namespace af
{
	template <typename T> __forceinline bool fpvalid( const T d, const bool bDenormalNormal = false/*non-zero number with magnitude smaller than the smallest 'normal' number*/ )
	{
		const int n = ::fpclassify(d);
		if( FP_ZERO == n )
			return true;
		if( FP_NORMAL == n )
			return true;
		if( FP_SUBNORMAL == n )
			return bDenormalNormal;
		// could be NAN or INFINITE
		return false;
	}

	// T - in data type
	// R - floored out data type
	template <typename T, typename R> constexpr __forceinline R posfloor( const T d )
	{
		// Floored integer
		return R( d );
	}

	// T - test data type
	template <typename T> constexpr __forceinline T minval( const T a, const T b )
	{
		return a < b ? a : b;
	}
	// T - test data type
	template <typename T> constexpr __forceinline T maxval( const T a, const T b )
	{
		return a > b ? a : b;
	}
}
