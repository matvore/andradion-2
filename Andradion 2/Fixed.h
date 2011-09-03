#if !defined(_FDF81160_2FA5_11d5_B6FE_0050040B0541_INCLUDED_)
#define      _FDF81160_2FA5_11d5_B6FE_0050040B0541_INCLUDED_

// here are some fixed point helper templates and functions
typedef int FIXEDNUM;

#define Fixed(x) (FIXEDNUM((double)65536 * double(x)))

/*template<double x>
struct Fixed
{
	static inline FIXEDNUM val() {return FIXEDNUM(x * (double)65536);}
};*/

template<class c>
inline FIXEDNUM FixedCnvTo(const c& x)
{
	return x * c(65536);
}

template<class c>
inline c FixedCnvFrom(const FIXEDNUM& x)
{
	return c(x) / c(65536);
}

FIXEDNUM FixedDiv(FIXEDNUM x,FIXEDNUM y);
FIXEDNUM FixedMul(FIXEDNUM x,FIXEDNUM y);

#endif