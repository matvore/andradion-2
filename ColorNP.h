class CColorNP : public CColor { // non-palettized color translation
  friend class CGraphics;

 public:
  unsigned short Color16b() const;
  unsigned long Color32b() const;
  virtual int Certify();
  CColorNP();
  CColorNP(BYTE r_,BYTE g_,BYTE b_,BYTE a_ = 0);

 private:
  unsigned long color;
};

