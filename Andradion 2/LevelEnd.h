// LevelEnd.h: interface for the CLevelEnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEVELEND_H__B8CC0666_D7EF_11D2_B6FE_0050040B0541__INCLUDED_)
#define AFX_LEVELEND_H__B8CC0666_D7EF_11D2_B6FE_0050040B0541__INCLUDED_

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

#endif // !defined(AFX_LEVELEND_H__B8CC0666_D7EF_11D2_B6FE_0050040B0541__INCLUDED_)
