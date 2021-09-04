#pragma once

// 2d vector for screen coordinates, maybe make it use integers since we cant have half pixels
class vec2_t {
public:
	float x, y;

public:
	// ctors
	vec2_t() = default;

	vec2_t(float xy) : x(xy), y(xy) {};
	vec2_t(float x, float y) : x(x), y(y) {};
	vec2_t(float x, float y, float z) : x(x), y(y) {};

	// at-accesors.
	__forceinline float& at( const size_t index ) {
		return ( ( float* )this )[ index ];
	}
	__forceinline float& at( const size_t index ) const {
		return ( ( float* )this )[ index ];
	}

	// index operators.
	__forceinline float& operator()( const size_t index ) {
		return at( index );
	}
	__forceinline const float& operator()( const size_t index ) const {
		return at( index );
	}
	__forceinline float& operator[]( const size_t index ) {
		return at( index );
	}
	__forceinline const float& operator[]( const size_t index ) const {
		return at( index );
	}

	// equality operators.
	__forceinline bool operator==( const vec2_t& v ) const {
		return v.x == x && v.y == y;
	}
	__forceinline bool operator!=( const vec2_t& v ) const {
		return v.x != x || v.y != y;
	}

	// copy assignment.
	__forceinline vec2_t& operator=( const vec2_t& v ) {
		x = v.x;
		y = v.y;
		return *this;
	}

	// negation-operator.
	__forceinline vec2_t operator-( ) const {
		return vec2_t( -x, -y );
	}

	// arithmetic operators.
	__forceinline vec2_t operator+( const vec2_t& v ) const {
		return {
			x + v.x,
			y + v.y
		};
	}
	__forceinline vec2_t operator-( const vec2_t& v ) const {
		return {
			x - v.x,
			y - v.y
		};
	}
	__forceinline vec2_t operator*( const vec2_t& v ) const {
		return {
			x * v.x,
			y * v.y
		};
	}
	__forceinline vec2_t operator/( const vec2_t& v ) const {
		return {
			x / v.x,
			y / v.y
		};
	}

	// compound assignment operators.
	__forceinline vec2_t& operator+=( const vec2_t& v ) {
		x += v.x;
		y += v.y;
		return *this;
	}
	__forceinline vec2_t& operator-=( const vec2_t& v ) {
		x -= v.x;
		y -= v.y;
		return *this;
	}
	__forceinline vec2_t& operator*=( const vec2_t& v ) {
		x *= v.x;
		y *= v.y;
		return *this;
	}
	__forceinline vec2_t& operator/=( const vec2_t& v ) {
		x /= v.x;
		y /= v.y;
		return *this;
	}

	// arithmetic operators w/ float
	__forceinline vec2_t operator+( float f ) const {
		return {
			x + f,
			y + f
		};
	}
	__forceinline vec2_t operator-( float f ) const {
		return {
			x - f,
			y - f
		};
	}
	__forceinline vec2_t operator*( float f ) const {
		return {
			x * f,
			y * f
		};
	}
	__forceinline vec2_t operator/( float f ) const {
		return {
			x / f,
			y / f
		};
	}

	// compound assignment operators w/ float
	__forceinline vec2_t& operator+=( float f ) {
		x += f;
		y += f;
		return *this;
	}
	__forceinline vec2_t& operator-=( float f ) {
		x -= f;
		y -= f;
		return *this;
	}
	__forceinline vec2_t& operator*=( float f ) {
		x *= f;
		y *= f;
		return *this;
	}
	__forceinline vec2_t& operator/=( float f ) {
		x /= f;
		y /= f;
		return *this;
	}

	// methods.
	__forceinline float length_sqr( ) const {
		return ( x * x + y * y );
	}

	__forceinline float length( ) const {
		return std::sqrt( length_sqr( ) );
	}

	__forceinline void clear( ) {
		x = y = 0.f;
	}

	__forceinline bool valid() const {
		return x > 1 && y > 1;
	}
};