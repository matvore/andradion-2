#ifndef __CLASSES_H__
#define __CLASSES_H__

#include <vector>
#include <string>
#include <matrix.cpp>
#include "Andradion 2.h"

using namespace GraphLib;
using namespace SoundLib;
using namespace GenericClassLib;
using namespace std;

class CAmmo;
class CBazookaFire;
class CEnemy;
class CHealthPack;
class CLevelEnd;
class CMachineGunFire;
class CMedia;
class CPistolFire;
class CTrigger;
class CTurner;
class CWall;

class CBullet;
class CGuy;
class CPointable;
class CLocatable;
class IToken;

class IToken {
public:
	IToken(int type_) : type(type_) {}

	int GetType(void) const {return type;}
	void SetState(int state_) {state = state_;}
	int GetState(void) const {return state;}

	// draws the character
	virtual void Draw(void) = 0;
	
	// performs a frame of logic
	virtual void Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE]) = 0; 
	
protected:
	int state;
	const int type;
};

class CPointable {
public:
	void SetDirection(int direction_) {direction = direction_;}
	int GetDirection(void) const {return direction;}

protected:
	int direction;
};

class CLocatable {
public:
	void SetPosition(int x_,int y_) {x = x_; y = y_;}
	void GetPosition(int& x_,int& y_) const {y_ = y;x_ = x;}
	void SetX(int x_) {x = x_;}
	void SetY(int y_) {y = y_;}
	int GetX(void) const {return x;}
	int GetY(void) const {return y;}
	void Translate(int x_,int y_) {x += x_; y += y_;}
	void TranslateX(int x_) {x += x_;}
	void TranslateY(int y_) {y += y_;}

protected:
	int x;
	int y;
};

class CBullet : public IToken , public CLocatable, public CPointable
{
public:
	CBullet();
	~CBullet();

protected:
	int Collision(void);
};

class CBazookaFire : public IToken, public CLocatable, public CPointable {
public:
	CBazookaFire();
	~CBazookaFire();

	virtual void Draw(void);
	virtual void Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE]);

private:
	bool Collision(int token_index); // finds a token it collides with
};

class CMachineGunFire :public CBullet
{
public:
	CMachineGunFire();
	~CMachineGunFire();

	virtual void Draw(void);
	virtual void Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE]);

private:
	CLocatable lb; // coordinates of the line to draw later on
};

class CPistolFire : public CBullet
{
public:
	CPistolFire();
	~CPistolFire();

	virtual void Draw(void);
	virtual void Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE]);
};

class CGuy :
public IToken,
public CLocatable,
public CPointable
{
	friend bool gameloop(bool);
public:
	void GuyLogic();
	CGuy(int token_type);
	~CGuy();

	// moves to previous coordinates
	void BackUp(void) {x = prevx; y = prevy;}

	// translates with regard to prev? values
	void Move(int x_,int y_) {prevx = x; prevy = y; x+= x_; y+= y_;}

	void SubtractHealth(float health_);

protected:
	int prevx;
	int prevy;

	int frames_in_this_state;

	float health;

	CBazookaFire bf;
	CMachineGunFire mgf;
	CPistolFire pf;

	void TryToFire(CTrigger& weapon);

	void ForceOutOfWall(void);
};

class CTurner : public CGuy
{
	friend bool gameloop(bool);

public:
	CTurner();
	~CTurner();

	virtual void Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE]);
	virtual void Draw(void);

	void SetCurrentWeapon(int current_weapon_);

private:
	void AddHealth(void);

	int current_weapon;

	CTrigger **weapons;
};

class CEnemy : public CGuy
{
	friend bool gameloop(bool);

public:
	CEnemy(int model_); 
	~CEnemy();

	virtual void Draw(void);
	virtual void Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE]);

private:
	const int model; // index into the bitmap array of CMedia

	CTrigger *weapon;
};

class CMedia { // a singleton
	friend bool gameloop(bool);

public:
	~CMedia();
	
	static CMedia& Media(void) {static CMedia media; return media;} // returns the only instance

	CColorMap& Bitmap(int index) {assert(index >= 0 && index < NUM_BITMAPS); return *(bitmaps[index]);}
	
	SOB& Sound(int index) {assert(index >= 0 && index < NUM_SOUNDS); return sounds[index];}

	// load the level
	void LoadLevel(int index);

	// get level data
	CColorMap& UpperBitmapOfLevel(void) {assert(NULL != upper_bitmap); return *upper_bitmap;}
	CColorMap& LowerBitmapOfLevel(void) {assert(NULL != lower_bitmap); return *lower_bitmap;}
	std::vector<IToken *>& Tokens(void) {assert(NULL != lower_bitmap && NULL != upper_bitmap); return tokens;}
	const matrix<int>& WalkingData(void) const {assert(NULL != lower_bitmap && NULL != upper_bitmap); return walking_data;}

	const std::string& OutdoorMusic(void) const {return outdoor_music;}
	const std::string& IndoorMusic(void) const {return indoor_music;}

	int GetCurrentLevel(void) const {return current_level;}

protected:
	CMedia();
	void CMediaConstruction();  // loads the media
	void CMediaDestruction(); // unloads the media and current level
	
private:
	void DestroyTokens(bool destroy_turner);
	// global media data
	CColorMap** bitmaps;
	SOB* sounds;

	// level data
	CColorMap *lower_bitmap;
	CColorMap *upper_bitmap;
	std::vector<IToken *> tokens;
	matrix<int> walking_data;	

	std::string outdoor_music;
	std::string indoor_music;

	int current_level; // -1 if nothing is loaded
};

class CTrigger : public IToken {
public:
	CTrigger(int weapon_type_);
	~CTrigger();

	virtual void Draw(void);
	virtual void Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE]);

	float GetAmmo(void) const {return ammo;}

	void AddAmmo(void);
	void Fire(void);

	int GetWeaponType(void) const {return weapon_type;}

	bool CanFire(void) const {return bool(0 == state && 0 != ammo);}
	
private:
	const int weapon_type;

	// state is frames before next fire
	float ammo;
};

class CAmmo : public IToken, public CLocatable {
public:
	CAmmo(int weapon_type_);
	~CAmmo();

	virtual void Draw(void);
	virtual void Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE]);

	int GetAmmoType(void) const {return weapon_type;}

private:
	const int weapon_type;
};

class CHealthPack : public IToken, public CLocatable {
public:
	CHealthPack();
	~CHealthPack();

	virtual void Draw(void);
	virtual void Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE]);
};

class CLevelEnd : public IToken, public CLocatable { // state is the path ID #
public:
	CLevelEnd();
	~CLevelEnd();

	virtual void Draw(void);
	virtual void Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE]);
};

class CWall : public IToken {
public:
	CWall();
	~CWall();

	virtual void Draw(void);
	virtual void Logic(unsigned char keyb[KeybLib::KEYBOARD_BUFFER_SIZE]);
};

#endif
