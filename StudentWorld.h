#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include <string>
#include "GameWorld.h"
//new inclusions
#include "Board.h"
#include <list>

class Actor;
class Player;

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetPath); // StudentWorld's constructor
	~StudentWorld();	// StudentWorld's destructor
	virtual int init();
	virtual int move();
	virtual void cleanUp();

	// Add a new player to the StudentWorld
	void add_player(Actor* new_player);

	// Add a new actor to the StudentWorld (not for players, but all other game objects)
	void add_actor(Actor* actor);

	// Determine if there is a square at the specified location. Used to determine if a Actor
	// like a player, enemy or vortex can move onto the specified location
	bool is_there_a_square_at_location(int dest_x, int dest_y);

	// Get a pointer to the square at the specified location (pass in pixels)
	Actor* get_square_at_location(double x, double y);

	// Get a pointer to the first enemy at the specified location (pass in pixels)
	Actor* get_enemy_at_location(double x, double y);

	// get # of coins in the bank
	int get_bank_coins() const { return m_moneyInBank; }

	// add coins to the bank (when player passes over a bank square)
	void deposit_bank_coins(int coins) { m_moneyInBank += coins; }

	// reset # of coins in the bank to zero
	void reset_bank_coins() { m_moneyInBank = 0; }

	// Get a random location on the board that contains a square
	Actor* get_random_square();

	// Given a player object pointer, returns a pointer to the other player object. Used for swapping
	// actions.
	Player* get_other_player(Player* p) const;

	// Get a pointer to a Player (enter 0 for peach or 1 for yoshi)
	Player* getPlayer(int playerNumber) const { return playerNumber == 0 ? m_peachPtr : m_yoshiPtr; }
private:
	bool actorsOverlap(double vx, double vy, double ex, double ey);	// determines if an Enemy overlaps with a vortex
	void doSomethingAllActors();	// makes all actors call their do_something() function
	void removeInactiveActors();	// deletes all inactive actors
	Actor* getWinner();	// gets the winner from Peach or Yoshi

	// Data Members
	Board bd;
	int m_moneyInBank;
	std::list<Actor*> m_listOfActors;
	Player* m_peachPtr;
	Player* m_yoshiPtr;
};

#endif // STUDENTWORLD_H_