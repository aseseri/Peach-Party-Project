#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
using namespace std;
//new inclusions
#include <iostream>
#include <sstream>
#include "Actor.h"

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
}

StudentWorld::~StudentWorld() {
    cleanUp();
}

int StudentWorld::init()    //  construct a representation of the world and store this in a StudentWorld object by loading the board data file and determining where each actor is
{
    // gets the Board Number
    int boardNumber = getBoardNumber();
    // creates a string stream for the board file
    ostringstream oss1;
    oss1 << assetPath() << "board0" << boardNumber << ".txt";
    string board_file = oss1.str();
    // Loads the board and detemines if the board is invalid
    Board::LoadResult result = bd.loadBoard(board_file);
    if (result == Board::load_fail_file_not_found)
        cerr << "Could not find " << oss1.str() << " data file\n";
    else if (result == Board::load_fail_bad_format)
        cerr << "Your board was improperly formatted\n";
    else if (result == Board::load_success)
        cerr << "Successfully loaded board\n";
    if (result != Board::load_success)
        return  GWSTATUS_BOARD_ERROR;

    // adds an actor for each corresponding character in the board file
    for (int x = 0; x < BOARD_HEIGHT; x++) {
        for (int y = 0; y < BOARD_WIDTH; y++) {
            Actor* temp;
            int c = x * SPRITE_HEIGHT;
            int r = y * SPRITE_WIDTH;
            Board::GridEntry ge = bd.getContentsOf(x, y); // x=5, y=10
            switch (ge) {
                case Board::empty:
                    break;
                case Board::boo:
                    temp = new CoinSquare(this, IID_BLUE_COIN_SQUARE, c, r, 3);
                    m_listOfActors.push_back(temp);
                    temp = new Boo(this, IID_BOO, c, r);
                    m_listOfActors.push_back(temp);
                    break;
                case Board::bowser:
                    temp = new CoinSquare(this, IID_BLUE_COIN_SQUARE, c, r, 3);
                    m_listOfActors.push_back(temp);
                    temp = new Bowser(this, IID_BOWSER, c, r);
                    m_listOfActors.push_back(temp);
                    break;
                case Board::player:
                    //has Peach & Yoshi and a blue coin square
                    temp = new CoinSquare(this,IID_BLUE_COIN_SQUARE, c, r, 3);
                    m_listOfActors.push_back(temp);
                    m_peachPtr = new Player(this,IID_PEACH, c, r, 1);
                    m_yoshiPtr = new Player(this,IID_YOSHI, c, r, 2);
                    break;
                case Board::blue_coin_square:
                    //blue coin square
                    temp = new CoinSquare(this,IID_BLUE_COIN_SQUARE, c, r, 3);
                    m_listOfActors.push_back(temp);
                    break;
                case Board::red_coin_square:
                    temp = new CoinSquare(this,IID_RED_COIN_SQUARE, c, r, -3);
                    m_listOfActors.push_back(temp);
                    break;
                case Board::star_square:
                    temp = new StarSquare(this, IID_STAR_SQUARE, c, r);
                    m_listOfActors.push_back(temp);
                    break;
                case Board::up_dir_square:
                    temp = new DirectionalSquare(this, IID_DIR_SQUARE, c, r, Actor::up, 0);
                    m_listOfActors.push_back(temp);
                    break;
                case Board::down_dir_square:
                    temp = new DirectionalSquare(this, IID_DIR_SQUARE, c, r, Actor::down, 0);
                    m_listOfActors.push_back(temp);
                    break;
                case Board::right_dir_square:
                    temp = new DirectionalSquare(this, IID_DIR_SQUARE, c, r, Actor::right, 0);
                    m_listOfActors.push_back(temp);
                    break;
                case Board::left_dir_square:
                    temp = new DirectionalSquare(this, IID_DIR_SQUARE, c, r, Actor::left, 180);
                    m_listOfActors.push_back(temp);
                    break;
                case Board::bank_square:
                    temp = new BankSquare(this, IID_BANK_SQUARE, c, r);
                    m_listOfActors.push_back(temp);
                    break;
                case Board::event_square:
                    temp = new EventSquare(this, IID_EVENT_SQUARE, c, r);
                    m_listOfActors.push_back(temp);
                    break;
            }
        }
    }
    m_moneyInBank = 0;  //sets money in the bank to 0

    startCountdownTimer(99);  // the game will last 99 seconds long
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()    // runs a single tick in the game (asks all actors to do something, adds new actors, removes invalid actors)
{
    // All active actors do something
    doSomethingAllActors();

    // Remove inactive actors
    removeInactiveActors();

    // Update Status Text at the top of the screen
    ostringstream oss2;
    oss2 << "P1 Roll: " << m_peachPtr->get_dice() << " Stars: " << m_peachPtr->get_stars()
        << " $$: " << m_peachPtr->get_coins() << (m_peachPtr->has_vortex() ? " VOR" : "") << " | Time: " << timeRemaining() << " | Bank: " <<
        m_moneyInBank << " | P2 Roll: " << m_yoshiPtr->get_dice() << " Stars: " << m_yoshiPtr->get_stars()
        << " $$: " << m_yoshiPtr->get_coins() << (m_yoshiPtr->has_vortex() ? " VOR" : "");
    setGameStatText(oss2.str());

    // Check if game is over
    if (timeRemaining() <= 0) {
        playSound(SOUND_GAME_FINISHED);
        // Determine a winner and display the winner on the screen
        Actor* winner = getWinner();
        if (winner == m_yoshiPtr) // yoshi won
        {
            setFinalScore(m_yoshiPtr->get_stars(), m_yoshiPtr->get_coins());
            return GWSTATUS_YOSHI_WON;
        }
        else // peach won
        {
            setFinalScore(m_peachPtr->get_stars(), m_peachPtr->get_coins());
            return GWSTATUS_PEACH_WON;
        }
        return GWSTATUS_NOT_IMPLEMENTED;
    }
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()    // deletes all dynamically allocated Actors
{
    while (m_listOfActors.begin() != m_listOfActors.end()) {    // iterates through the list of Actor pointers until there are none left
        delete* (m_listOfActors.begin());   // deletes the dynamically allocated Actors
        m_listOfActors.pop_front();     // removes deleted Actor pointers from the list
    }
    delete m_peachPtr;  // deletes the Player pointers
    delete m_yoshiPtr;
}

void StudentWorld::add_actor(Actor* new_player){    // adds an actor to the list
    m_listOfActors.push_back(new_player);
}

bool StudentWorld::is_there_a_square_at_location(int dest_x, int dest_y) // checks if there is a square at the location given by the parameters (in pixels)
{ 
    Board::GridEntry ge = bd.getContentsOf(dest_x / 16, dest_y / 16);

    switch (ge) {
    case Board::blue_coin_square:
        [[fallthrough]];
    case Board::red_coin_square:
        [[fallthrough]];
    case Board::up_dir_square:
        [[fallthrough]];
    case Board::down_dir_square:
        [[fallthrough]];
    case Board::left_dir_square:
        [[fallthrough]];
    case Board::right_dir_square:
        [[fallthrough]];
    case Board::event_square:
        [[fallthrough]];
    case Board::bank_square:
        [[fallthrough]];
    case Board::star_square:
        [[fallthrough]];
    case Board::bowser:
        [[fallthrough]];
    case Board::boo:
        [[fallthrough]];
    case Board::player:
        return true;
        break;
    default: //Board::empty
        break;
    }
    return false;
}

Actor* StudentWorld::get_square_at_location(double x, double y) { // returns the pointer to the Square that is at the location given by the parameter (in pixels)
    for (list<Actor*>::iterator p = m_listOfActors.begin(); p != m_listOfActors.end(); p++) {   // iterate through the list
        if ((*p)->is_a_square() && (*p)->getX() == x && (*p)->getY() == y) // find an Actor that is a square and at the given coordinates
            return *p;
    }
    return nullptr;
}

Actor* StudentWorld::get_enemy_at_location(double x, double y) { // returns the pointer to the Enemy that is at the location given by the parameter (in pixels)
    for (list<Actor*>::iterator p = m_listOfActors.begin(); p != m_listOfActors.end(); p++) {   //iterate through the list
        if ((*p)->can_be_hit_by_vortex()) { // finds the Actors that can be hit by vortexes ( Enemy )
            if (  actorsOverlap(x, y, (*p)->getX(), (*p)->getY())  )
                return *p;
        }
    }
    return nullptr;
}

Actor* StudentWorld::get_random_square() {  // gets a pointer to a random Square in the board
    list<Actor*> squareActors;  // create a list of only Square Actors
    for (list<Actor*>::iterator p = m_listOfActors.begin(); p != m_listOfActors.end(); p++) { 
        if ((*p)->is_a_square())
            squareActors.push_back(*p);
    }
    int size = squareActors.size();
    int randPosition = randInt(0, size - 1);    // randomly pick a Square in the list of Square Actors
    int i = 0;
    for (list<Actor*>::iterator p = squareActors.begin(); p != squareActors.end(); p++, i++) {
        if (i == randPosition)
            return *p;
    }
    return nullptr;
}

Player* StudentWorld::get_other_player(Player* p) const {   // given a pointer to a player, returns the other player
    if (p == m_peachPtr)
        return m_yoshiPtr;
    return m_peachPtr;
}

Actor* StudentWorld::getWinner() {  // determines the winner based on stars and coins
    Actor* temp;
    int pStars = m_peachPtr->Player::get_stars();
    int yStars = m_yoshiPtr->Player::get_stars();
    if (pStars > yStars)    // player with most stars wins
        temp = m_peachPtr;
    else if (yStars > pStars)
        temp = m_yoshiPtr;
    else {
        int pCoins = m_peachPtr->Player::get_coins();
        int yCoins = m_yoshiPtr->Player::get_coins();
        if (pCoins > yCoins)    // player with most coins wins if same number of stars
            temp = m_peachPtr;
        else if (yCoins > pCoins)
            temp = m_yoshiPtr;
        else {
            if (randInt(1, 2) == 1) // random winner if same number of stars and coins
                temp = m_peachPtr;
            else
                temp = m_yoshiPtr;
        }
    }
    return temp;
}

void StudentWorld::doSomethingAllActors() { // makes all Actors call their do_something() function
    for (list<Actor*>::iterator p = m_listOfActors.begin(); p != m_listOfActors.end(); p++) {   //iterates through the list of actors
        if ((*p)->is_active()) {
            (*p)->do_something();   // makes all active actors do_something()
        }
    }
    m_peachPtr->do_something();
    m_yoshiPtr->do_something();
}

void StudentWorld::removeInactiveActors() { // deletes and removes inactive Actors 
    for (list<Actor*>::iterator p = m_listOfActors.begin(); p != m_listOfActors.end();) {   //iterates through list
        if (!(*p)->is_active()) {
            delete* p;  // delete because Actors are dynamically allocated
            p = m_listOfActors.erase(p);    // removes Actor pointer from the list
        }
        else
            p++;
    }
}

bool StudentWorld::actorsOverlap(double vx, double vy, double ex, double ey) {  // determiens if a Vector and Enemy overlaps
    return (vx-16 <= ex) && (ex <= vx + 16) && (vy - 16 <= ey) && (ey <= vy + 16);  // a vector and enemy can overlap by simply one pixel
}