using std::pair;

class CCompactMap : public CCertifiable {
  friend class CGraphics;

  friend void PutCompactMapStep2of2NoLeftRightClip(BYTE *lpSurface,
						   long pitch,
						   CCompactMap& bob,
						   int start_y,
						   int end_y,
						   int start_x);

  friend void PutCompactMapStep2of2RightClipOnly(BYTE *lpSurface,
						 long pitch,
						 CCompactMap& bob,
						 int start_y,
						 int end_y,
						 int start_x,
						 int max_x);

  friend void PutCompactMapStep2of2LeftClipOnly(BYTE *lpSurface,
						long pitch,
						CCompactMap& bob,
						int start_y,
						int end_y,
						int start_x,
						int min_x);

  friend void PutCompactMapStep2of2LeftRightClip(BYTE *lpSurface,
						 long pitch,
						 CCompactMap& bob,
						 int start_y,
						 int end_y,
						 int start_x,
						 int min_x,
						 int max_x);
	
 private: CertParamB(string ,file_name         ,FileName        );
 private: CertParamA(bool   ,load_from_resource,LoadFromResource);
 private: CertParamB(string ,resource_type     ,ResourceType    );
 private: CertParamA(HMODULE,resource_module   ,ResourceModule  );
 private: CertParamA(WORD   ,resource_language ,ResourceLanguage);

 public:
  CCompactMap();
  ~CCompactMap();

  virtual int Certify();
  virtual void Uncertify();

  // uses direct draw clippers and Blt, clipper should already be
  // attatched to surface, otherwise, no clipping will happen!
  // pass:  target surface, target x, target y, true to blit
  // asynchronously
  void RenderStep1(IDirectDrawSurface2 *, int, int, bool); 

  // uses manual clipper and BltFast.
  // pass: target surface, target x and y, and clipper rectangle
  void RenderStep1(IDirectDrawSurface2 *, int, int, const RECT&);

  // this function needs the surface to be locked
  //  if the last parameter is null, then there will be
  //  no clipping
  void RenderStep2(void *surface, int pitch, int x, int y,
		   const RECT *simple_clipper_rect); 

  // this returns true if the compact map is just nothingness
  inline bool Empty() const;

  // if RenderStep1 is not necessary, then this function returns true
  inline bool NoStep1() const;

  // this function returns true if RenderStep2 is not necessary
  inline bool NoStep2() const;		

 private:

  // info on patterns . . .

  VCTR_DIRECTDRAWSURFACE patterns; // bitmap data of patterns
  VCTR_RECTANGLEVECTOR pattern_coors; // coordinates of patterns
		
  // info on blocks . . .

  VCTR_DWORD blocks; // colors of the blocks
  VCTR_RECTANGLEVECTOR block_areas; // coordinates of the blocks

  // info on left over . . .
  VCTR_ROW left_over;

  // clipping info on left over . . .
  pair<int,int> min_max_x_offsets;
  VCTR_MINMAXXROWOFFSET min_max_x_row_offsets;
};

inline bool CCompactMap::Empty() const {
  assert(Certified());

  return this->patterns.empty() && this->blocks.empty()
    && this->left_over.empty();
}

inline bool CCompactMap::NoStep1() const {
  assert(Certified());

  return this->patterns.empty() && this->blocks.empty();
}

inline bool CCompactMap::NoStep2() const {
  assert(Certified());

  return this->left_over.empty();
}

