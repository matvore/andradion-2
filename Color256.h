class CColor256 : public CColor {
 public:
  CColor256();
  CColor256(BYTE r_,BYTE g_,BYTE b_);

  virtual int Certify();
  BYTE Color() const; // must be certified to call

  static void ChangePalette(LPDIRECTDRAWPALETTE pal);
  static void ChangePalette(const PALETTEENTRY *pal,int entries);
  static void ClearPalette(); // clears palette data to free memory
  static HPALETTE GetGDIPalette(); // do not destroy the returned handle!

 private:
  // the color in 8-bit format (only valid if true == this->is_certified)
  BYTE col;
		
  // pointer to palette entries
  static HPALETTE gdi_palette;
};
