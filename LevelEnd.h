class CLevelEnd {
public:
  CLevelEnd(FIXEDNUM x_, FIXEDNUM y_, int ref_)
    : x(x_), y(y_), ref(ref_) {}
  bool Collides(FIXEDNUM tx, FIXEDNUM ty) const;
  int Reference() const {return ref;}

private:
  FIXEDNUM x, y;
  int ref;
};
