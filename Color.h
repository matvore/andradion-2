class CColor : public CCertifiable {
 public:
  // accessors which work with the entire structure
  void GetColor(BYTE& r_,BYTE& g_,BYTE& b_) const;
  void GetColor(BYTE& r_,BYTE& g_,BYTE& b_,BYTE& a_) const;
  COLORREF GetColor() const;
  void SetColor(BYTE r_,BYTE g_,BYTE b_,BYTE a_ = 0);
  void SetColor(COLORREF new_color);
		
  virtual int Certify() = 0;
  CColor();
  CColor(BYTE r_,BYTE g_,BYTE b_,BYTE a_ = 0);

 protected: CertParamA(BYTE,r,Red);
 protected: CertParamA(BYTE,g,Green);
 protected: CertParamA(BYTE,b,Blue);
 protected: CertParamA(BYTE,a,Alpha);
};
