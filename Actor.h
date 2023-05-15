#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"

class Actor : public GraphObject {
public:
    //Note that there is no need to add a virtual destructor because the base class GraphObject already has one
    Actor(StudentWorld* sw, int imageID, int startX, int startY, int dir, double size, int depth);  //Actor's constructor
    virtual void do_something() = 0;    // do_something is a pure virtual function specified differenty in each derived class
    virtual bool is_a_square() const = 0;   // is_a_square is a pure virtual function specified differenty in each derived class
    virtual bool can_be_hit_by_vortex() const = 0; // can_be_hit_by_vortex is a pure virtual function specified differenty in each derived class
    virtual void hit_by_vortex() {}  // tell an game object it has been hit by a vortex
    bool is_active() const { return m_isActive; }   // is a game object still alive or should it be removed
    void set_inactive() { m_isActive = false; } // changes the status of an Actor to inalive/dead
    StudentWorld* get_ptr_to_student_world() const{ return m_sw; }  // Gets a pointer to the Actor's StudentWorld
    enum DIR {
        right = 0, left = 180, up = 90, down = 270
    };
private:
    bool m_isActive;
    StudentWorld* m_sw;
};

class Player : public Actor {
public:
    Player(StudentWorld* sw, int imageID, int startX, int startY, int player_number);   //Player's constructor
    virtual void do_something();    // dictates the movement of a Player by receiving the user's input to roll a dice and move, walk only on paths with squares, and allow the user to choose direction at a fork in the road
    void reset_walk_direction() { m_walkDirection = right; } // sets a Player's walking direction to the right direction by default 
    void reset_coins() { m_coins = 0; } // sets a Player's coins to 0
    void reset_stars() { m_stars = 0; } // sets a Player's stars to 0
    virtual bool is_a_square() const { return false; }  // Indicates that a Player is not a square
    bool can_be_hit_by_vortex() const { return false; } // Indicates that a Player is not impactable by a vortex
    int get_dice() const { return m_dice; }    // used to display stats on status line
    int get_stars() const { return m_stars; }   // used to display stats on status line
    int get_coins() const { return m_coins; }   // used to display stats on status line
    bool has_vortex() const { return m_hasVortex; } // used to display stats on status line
    bool is_walking() const { return m_isWalking; } // determines the state of the Player (walking or waiting)
    void force_walk_direction(DIR dir, int angle);  // sets the Player's walking direction respective sprite direction based on the parameters
    void adjust_stars(const int this_much) { m_stars += this_much; } // adds this_much to the Player's stars
    void adjust_coins(const int this_much) { m_coins += this_much; } // adds this_much to the Player's coins
    void swap_positions();  // with other player
    void swap_stars();  // with other player
    void swap_coins();  // with other player
    void equip_with_vortex_projectile() { m_hasVortex = true; } // indicates that a Player has a vortex
    void teleport_me_to_random_sq();    // changes the Player's position to a random Square on the board
    bool getJustLanded() const { return justLanded; }   // gets whether a Player has just landed on a square (or conversely has been waiting on a square for more than one tick)
    bool getJustMoved() const { return justMoved; }     // gets whether a Player has just moved onto a square (or conversely has been passing by the square for more than one tick)
    DIR getWalkingDirection() const { return m_walkDirection; } // gets the direction that the Player is walking in
    void setIgnoreFork(bool ignore) { m_ignoreFork = ignore; }  // indicates that the user must not choose a direction for the Player (even though there may be more than one path to choose from)
protected:
    int getPlayerNumber() const { return m_playerNumber; }  // gets the Player's number (1 for Peach and 2 for Yoshi)
private:
    bool canPlayerMoveForward();
    bool canWalkInDirection(DIR aDirection) const;
    void checkAndSetSpriteDirection();
    bool m_ignoreFork;
    bool m_hasVortex;
    bool justLanded;
    bool justMoved;
    int m_playerNumber;
    int m_dice;
    int m_stars;
    int m_coins;
    DIR m_walkDirection;
    int m_ticksToMove;
    bool m_isWalking;
};

class ActivatingObject : public Actor {
public:
    ActivatingObject(StudentWorld* sw, int imageID, int startX, int startY, int dir, double size, int depth);   // ActivatingObject's constructor
    virtual void do_something();    
};

class ActivateOnPlayer : public ActivatingObject {
public:
    ActivateOnPlayer(StudentWorld* sw, int imageID, int startX, int startY,
        int dir, double size, int depth, bool activate_when_go_lands);
protected:
    bool checkJustLanded(Player* player) const;
    bool checkJustMoved(Player* player) const;
private:
    bool m_activateWhenGoLands;
};

class Vortex : public ActivatingObject {
public:
    Vortex(StudentWorld* sw, int imageID, int startX, int startY, DIR dir);
    virtual bool is_a_square() const { return false; }
    virtual bool can_be_hit_by_vortex() const { return false; }
    //std::vector<Actor*> do_i_activate();
    virtual void do_something();
private:
    void moveVortex();
    Actor* vortexOverlapsWithAnEnemy() const;
    DIR m_movingDirection;
};

class StarSquare : public ActivateOnPlayer {
public:
    StarSquare(StudentWorld* sw, int imageID, int startX, int startY);
    virtual bool is_a_square() const { return true; }
    virtual bool can_be_hit_by_vortex() const { return false; }
    virtual void do_something();
};

class CoinSquare : public ActivateOnPlayer {
public:
    CoinSquare(StudentWorld* sw, int imageID, int startX, int startY, int adjust_coins_by);
    virtual bool is_a_square() const { return true; }
    virtual bool can_be_hit_by_vortex() const { return false; }
    virtual void do_something();
private:
    int m_adjustCoinsBy;
};

class DirectionalSquare : public ActivateOnPlayer {
public:
    DirectionalSquare(StudentWorld* sw, int imageID, int startX, int startY, DIR dir, int angle);
    virtual bool is_a_square() const { return true; }
    virtual bool can_be_hit_by_vortex() const { return false; }
    virtual void do_something();
private:
    DIR m_facingDirection;
    int m_angle;
};

class BankSquare : public ActivateOnPlayer {
public:
    BankSquare(StudentWorld* sw, int imageID, int startX, int startY);
    virtual bool is_a_square() const {return true;}
    virtual bool can_be_hit_by_vortex() const { return false; }
    virtual void do_something();
};

class EventSquare : public ActivateOnPlayer {
public:
    EventSquare(StudentWorld* sw, int imageID, int startX, int startY);
    virtual bool is_a_square() const { return true; }
    virtual bool can_be_hit_by_vortex() const { return false; }
    virtual void do_something();
};

class Enemy : public ActivateOnPlayer {
public:
    Enemy(StudentWorld* sw, int imageID, int startX, int startY,
        int dir, double size, int depth, bool activate_when_go_lands, int num_sq_to_move, int number_of_ticks_to_pause);
    virtual void do_something();
    virtual bool is_a_square() const { return false; }
    virtual bool can_be_hit_by_vortex() const { return true; }
    virtual void hit_by_vortex();  // called when enemy is hit by a vortex projectile (called by vortex projectile)
protected:
    void randWalkDirection();
    void checkAndSetSpriteDirection();
    bool canWalkInDirection(int aDirection) const;
    bool canEnemyMoveForward() const;
    void enemyOnSquareWithPlayer();
    void atATurningPoint();
    bool atAFork();
    void walkForward();
    virtual void doToPlayer(Player* player) = 0;
    virtual void dropSquare() { ; }
private:
    bool justLanded;
    bool justMoved;

    int m_num_sq_to_move;
    int m_number_of_ticks_to_pause;
    int m_ticks_to_move;
    int m_walkDirection;
    bool m_is_walking;
    bool m_ignoreFork;
};

class DroppingSquare : public ActivateOnPlayer {
public:
    DroppingSquare(StudentWorld* sw, int imageID, int startX, int startY);
    virtual bool is_a_square() const { return true; }
    virtual bool can_be_hit_by_vortex() const { return false; }
    virtual void do_something();
};

class Bowser : public Enemy {
public:
    Bowser(StudentWorld* sw, int imageID, int startX, int startY);
    //virtual void do_something();
    virtual void doToPlayer(Player* player);
    virtual void dropSquare();
};

class Boo : public Enemy {
public:
    Boo(StudentWorld* sw, int imageID, int startX, int startY);
    virtual void doToPlayer(Player* player);
};

#endif // ACTOR_H_