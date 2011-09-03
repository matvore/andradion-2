/*
Copyright 2011 Matt DeVore

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/** Indicates the player was disconnected from the game.
 */
class NetSessionLost : public std::exception {
public:
  virtual const char *what() const throw() {
    return "Disconnected from network game";
  }
};

/** Indicates it was impossible to join a game in a particular room.
 */
class NetJoinFailure : public std::exception {
public:
  virtual const char *what() const throw() {
    return "Unable to join game";
  }
};

/** Indicates it was impossible to create and host a new game in a
 * particular room.
 */
class NetCreateFailure : public std::exception {
public:
  virtual const char *what() const throw() {
    return "Unable to create game";
  }
};

/** Allows the Net module to talk back to another part of the
 * application code in order to indicate what is happening in the
 * network game.
 */
class NetFeedback {
public:
  NetFeedback() throw() {}
  virtual ~NetFeedback() throw() {}

  /** Indicates that the host has changed the weather. This method
   * will not be called if this computer is the host.
   * @param new_state the new value of the weather state indicator.
   */
  virtual void SetWeatherState(unsigned int new_state) throw() = 0;

  /** Indicates that another player has fired the bazooka.
   * @param index the index of the enemy character.
   * @param x_hit the exact x-coordinate where the bazooka hit.
   * @param y_hit the exact y-coordinate where the bazooka hit.
   */
  virtual void EnemyFiresBazooka(unsigned int index,
                                 unsigned short x_hit,
                                 unsigned short y_hit) throw() = 0;

  /** Indicates that some enemy has fired the pistol.
   * @param index the index of the enemy firing the pistol.
   * @param direction the direction in which the pistol was fired.
   */
  virtual void EnemyFiresPistol(unsigned int index) throw() = 0;

  /** Indicates that some enemy has fired the machine gun.
   * The direction is not indicated by this method; it is assumed to
   * be the direction in which the enemy is known to be facing at the
   * moment.
   */
  virtual void EnemyFiresMachineGun(unsigned int index) throw() = 0;

  /** Indicates that some enemy has grabbed a power up.
   * @param index an index indicating which power up was picked up.
   */
  virtual void PickUpPowerUp(unsigned short index) throw() = 0;

  /** Indicates that the enemies are about to be recounted. A call to
   * this will be followed by any number of consecutive calls to
   * <tt>CreateEnemy(unsigned int)</tt>.
   */
  virtual void ClearEnemyArray() throw() = 0;

  /** Indicates that a new enemy was found. The index assigned to the
   * enemy is determined by the order in which the enemies were
   * created. For example, upon the first call to this function, the 
   * enemy created will have the index of 0. Upon the second call to
   * this function, the enemy created will have the index of 1, and so
   * on.
   * @param model the model that this enemy should appear as.
   */  
  virtual void CreateEnemy(unsigned int model) throw() = 0;
  
  virtual void SetEnemyWeapon(unsigned int index,
                              unsigned int weapon) throw() = 0;
  virtual void SetEnemyPosition(unsigned int index,
                                unsigned short x,
                                unsigned short y) throw() = 0;
  
  virtual void SetEnemyDirection(unsigned int index,
                                 unsigned int direction) throw() = 0;
  
  virtual void WalkEnemy(unsigned int index) throw() = 0;
  
  virtual void KillEnemy(unsigned int index, WORD *ammo)
    throw() = 0;
  virtual void HurtEnemy(unsigned int index) throw() = 0;
  virtual void HurtHero(unsigned int weapon_type) throw() = 0;

  virtual void PlayerLeaving(const char *name) throw() = 0;
  virtual void PlayerJoining(const char *name) throw() = 0;
};

/** Initializes the Net module. If the computer does not seem to have
 * DirectPlay installed, then this function does not throw an error but
 * rather acts as if DirectPlay exists and no protocols are installed.
 * @throw std::bad_alloc if an out-of-memory error occurs or if
 *  there is a "catastrophic failure" as indicated by the
 *  <em>Microsoft COM SDK Documentation</em>.
 * @post the Net module is initialized.
 */
void NetInitialize() throw(std::bad_alloc);

/** Closes the Net module.
 * @post the Net module is not initialized.
 */
void NetRelease() throw();

/** This should be called after you initialize the Net module but
 * before you create or join a game. It permits connection using a
 * particular network protocol. It requires that the given protocol
 * index be less than the protocol count, because it is a zero-based
 * index. It is reasonable to expect no exceptions will be thrown
 * because upon enumeration of all the network protocols, each one is
 * tested to see if it is available for use. If any protocol is
 * already initialized, then it is released before this one is
 * initialized.
 * @param index the index of the protocol to initialize.
 * @pre the Net module is initialized, and that
 *  <tt>index < NetProtocolCount()</tt>
 * @post <tt>NetProtocolInitialized()</tt> returns true.
 */
void NetInitializeProtocol(unsigned int index) throw();

/** Releases the currently in-use protocol. If no protocol is
 * initialized, then this function has no effect.
 * @post <tt>NetInGame()</tt> and
 *  <tt>NetProtocolInitialized()</tt> return false.
 */
void NetReleaseProtocol() throw();

/** Joins a game on the network already in progress. If any game is
 * already in progress, then it is left.
 * @param index the index of the room in which to play.
 * @param player_model a number indicating the model that the
 *  player using this computer will use.
 * @param player_name the name of the player using this computer.
 * @param fb allows the Net module to communicate events and data
 *  back to the caller of the function.
 * @return index of the level.
 * @throw NetJoinFailure if it was impossible to join the given game
 *  for any reason. For example, the game may not exist.
 * @pre <tt>NetProtocolInitialized()</tt> returns true.
 * @post <tt>NetInGame()</tt> returns true and <tt>NetIsHost()</tt>
 *  returns false.
 */
unsigned int NetJoinGame(unsigned int index,
                         unsigned int player_model,
                         const char *player_name,
                         std::auto_ptr<NetFeedback> fb)
  throw(NetJoinFailure);

/** Creates a game in a given room, making this computer the host. If
 * any game is already in progress, then that game is left.
 * @param index the index of the room in which to create the game.
 * @param sr the sync rate that will be used for the new
 *  game. The same sync rate will be in use by all players who join.
 * @param initial_weather_state this value will be communicated to all
 *  players who join before the first change in weather.
 * @param player_model the model that the player using this computer
 *  will use.
 * @param player_name the name of the player using this computer.
 * @param fb allows the Net module to communicate data and events back
 *  to the caller of this function.
 * @throw NetCreateFailure if the game could not be created. For
 *  example, the game may already be hosted in the given room.
 * @pre the Net module is initialized.
 * @post <tt>NetInGame()</tt> and <tt>NetIsHost()</tt> return true.
 */
void NetCreateGame(unsigned int index,
                   unsigned int sr,
                   unsigned int initial_weather_state,
                   unsigned int player_model,
                   const char *player_name,
                   std::auto_ptr<NetFeedback> fb)
  throw(NetCreateFailure);

/** Sets the level index and allows players to join the game. This function
 * only has an effect if <tt>NetIsHost()</tt> is true.
 * No players will join your game until this function is called.
 * @param index the index of the level on which this game will be played.
 * @pre This function has not been called since the game was created.
 */
void NetSetLevelIndex(unsigned int index) throw(NetSessionLost);

/** Leaves a currently open game. If a game is not in progress, then
 * nothing happens.
 * @pre the Net module is initialized.
 * @post <tt>NetInGame()</tt> returns false.
 */
void NetLeaveGame() throw();

/** Determines if a protocol is in use.
 * @return true iff a protocol is in use, meaning it has been
 *  initialized by <tt>NetInitializeProtocol(unsigned int)</tt>.
 * @pre the Net module is initialized.
 */
bool NetProtocolInitialized() throw();

/** Determines if we are playing a game.
 * @return true iff a game is in progress.
 */
bool NetInGame() throw();

/** Determines if we are playing a game and we are the host of that
 * game.
 * @return true iff we are the host of a game in progress.
 */
bool NetIsHost() throw();

/** Determines the number of protocols available to connect with.
 * @pre the Net module is initialized.
 * @return the number of protocols available to connect with.
 */
unsigned int NetProtocolCount() throw();

/** Supplies a label that can be used to identify the protocol of the
 * given index.
 * @pre The Net module is initialized and
 *  <tt>protocol_index < NetProtocolCount()</tt>
 * @param protocol_index the index of the protocol whose label is
 *  needed.
 * @return a C-string that identifies the given protocol index.
 */
const char *NetProtocolName(unsigned int protocol_index) throw();

/** Performs one frame of logic for all the remote players according to
 * the information gathered about them. Also processes
 * messages that are sent by other players. If no game is in progress,
 * then this function has no effect.
 * @throw NetSessionLost if we were disconnected from the game.
 * @throw std::bad_alloc if there was a lack of sufficient system
 *  memory.
 */
void NetLogic() throw(NetSessionLost, std::bad_alloc); 

/** Indicates to the other players in the game that the player fired
 * the bazooka. This function has no effect if a game is not in
 * progress.
 * @param x the x-coordinate at which the bazooka hits.
 * @param y the y-coordinate at which the bazooka hits.
 */
void NetFireBazooka(unsigned short x, unsigned short y) throw();

/** Indicates to the other players in the game whether or not the
 * player is firing the machine gun for this frame. This function
 * should be called every frame.
 */
void NetFireMachineGun(bool firing) throw();

/** Indicates to the other players in the game what weapon the player
 * is holding. This function should be called every frame and has no
 * effect if a game is not in progress.
 */
void NetSetWeapon(unsigned int type) throw();

/** Indicates to the other players in the game that the player has
 * died and left some ammo in his place. This function has no effect
 * if a game is not in progress.
 * @param ammo the ammo the player had as he died.
 */
void NetDied(WORD *ammo) throw();

/** Indicates to the other players what position the player is in this
 * frame. This function should be called every frame and has no effect
 * if a game is not in progress.
 * @param new_x the x-coordinate of the player for this frame.
 * @param new_y the y-coordinate of the player for this frame.
 * @param new_dir the direction the player is facing for the current
 *  frame. 
 */
void NetSetPosition(unsigned short new_x, unsigned short new_y,
                    unsigned int new_dir) throw();

/** Indicates to the other players that we have just played a sound
 * indicating we are hurt. Using this function it is possible for the
 * other players to know when this player is hurt. This function has
 * no effect if a game is not in progress.
 */
void NetAdmitHit() throw();

/** Indicates to another player that you have hit him with a pistol
 * bullet or the machine gun. The other player should respond with a
 * <tt>NetAdmitHit()</tt> call, which will notify all players of his
 * pain. This function has no effect if a game is not in progress.
 * @param player the index of the player that was hit.
 * @param weapon_type a weapon index indicating either pistol or
 *  machine gun.
 */
void NetHit(unsigned int player,
            unsigned int weapon_type) throw();

/** Indicates to the other players that you have picked up a power
 * up. This function has no effect if a game is not in progress.
 * @param powerup_index the index of the power up that was picked up.
 */
void NetPickUpPowerUp(unsigned short powerup_index) throw();

/** Indicates to the other players that you have fired a pistol in
 * some direction. The pistol that results on the screens of other
 * players is only a visual indicator. In order to cause another
 * player to be hurt, call <tt>NetHit(unsigned int, unsigned
 * int)</tt>. If a game is not in progress, then this function has no
 * effect.
 * @param dir indicates which direction the pistol was fired
 *  in.
 */
void NetFirePistol(unsigned int dir) throw();

/** If you are the host, indicates to the other players that the
 * weather has changed. If you are not the host, or if a game is not
 * in progress, then this function has no effect.
 * @param new_weather a number indicating the new weather state.
 */
void NetChangeWeather(unsigned int new_weather) throw();
