typedef int FIXEDNUM;

#define Fixed(x) (FIXEDNUM((double)65536 * double(x)))

template<class c> inline FIXEDNUM FixedCnvTo(const c& x) {
  return (FIXEDNUM)(x * c(65536));
}

template<class c> inline c FixedCnvFrom(const FIXEDNUM& x) {
  return c(x) / c(65536);
}

FIXEDNUM FixedDiv(FIXEDNUM x, FIXEDNUM y);
FIXEDNUM FixedMul(FIXEDNUM x, FIXEDNUM y);

