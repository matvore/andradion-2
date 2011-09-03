class CCharacter;

class CLevelEnd  
{
 public:
  void Setup(FIXEDNUM x_,FIXEDNUM y_,int reference_) {x = x_; y = y_; reference = reference_;}
  bool Collides(const CCharacter& ch);
  int Reference() const {return this->reference;}

 private:
  FIXEDNUM x;
  FIXEDNUM y;
  int reference;
};
