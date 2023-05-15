#include "Actor.h"
#include "StudentWorld.h"

// Actor Implementation
Actor::Actor(StudentWorld* sw, int imageID, int startX, int startY, int dir, double size, int depth)	// Actor's constructor
	:GraphObject(imageID, startX, startY, dir, depth,size), m_isActive(true), m_sw(sw)
{}
//------------------------------------------
// Player Implementation
Player::Player(StudentWorld* sw, int imageID, int startX, int startY, int player_number)	// Player's constructor
	:Actor(sw, imageID, startX, startY, right,1,0)
{
	m_coins = 0;
	m_stars = 0;
	m_dice = 0;
	m_isWalking = false;
	m_ticksToMove = 0;
	m_walkDirection = right;
	m_playerNumber = player_number;
	justLanded = false;
	justMoved = false;
	m_ignoreFork = true;
	m_hasVortex = false;
}

void Player::do_something() {
	// gets and locally stores the StudentWorld Pointer
	StudentWorld* swPtr = get_ptr_to_student_world();
	// Waiting to Roll State
	if (!m_isWalking) {
		justLanded = false;	// Indicates that the Player is no longer new to Square and has landed/been waiting for at least one tick

		// Ensures that a Player is not in between Squares while Waiting
		// Fixes the Player's position to a Square because swapping done by the EventSquare while one of the players is walking results in a Player waiting between two squares
		if (getX() % 16 != 0 || getY() % 16 != 0) {
			moveTo((getX()+8) / 16 * 16, (getY()+8) / 16 * 16);
		}

		int action = swPtr->getAction(m_playerNumber);		// Gets the user's input ( to roll the dice, use a vortex, choose a direction )
		if (action == ACTION_ROLL) {	// the user wants to roll the dice, randomly assigns a dice roll and moves the Player to a walking status
			m_dice = randInt(1, 10);	
			m_ticksToMove = m_dice * 8;
			m_isWalking = true;
		}
		if (action == ACTION_FIRE && m_hasVortex) {	// the user wants to use a vortex (if the Player has one) and adds the new Actor to StudentWorld
			Vortex* myVortex = new Vortex(swPtr, IID_VORTEX, getX(), getY(),m_walkDirection);
			swPtr->add_actor(myVortex);
			swPtr->playSound(SOUND_PLAYER_FIRE);
			m_hasVortex = false;
		}
		else {
			return;	
		}
	}

	// Walking State
	if (m_isWalking) {
		// Checks what directions a Player can Move in 
		int count = 0;
		bool canMoveRight = false, canMoveLeft = false, canMoveUp = false, canMoveDown = false;
		// Counts Possible pathways that a Player can move to 
		if (getX()%16==0 && getY()%16==0){
			if (left != m_walkDirection && canWalkInDirection(right)) {
				canMoveRight = true;
				count++;
			}
			if (right != m_walkDirection && canWalkInDirection(left)) {
				canMoveLeft = true;
				count++;
			}
			if (down != m_walkDirection && canWalkInDirection(up)) {
				canMoveUp = true;
				count++;
			}
			if (up != m_walkDirection && canWalkInDirection(down)) {
				canMoveDown = true;
				count++;
			}
		}

		// At a Fork/Intersection (when there are more than one paths that a Player may move to next)
		if (count > 1 && m_ignoreFork == false) {		
			// Gets user's pressed key
			int action = swPtr->getAction(m_playerNumber);
			if (canMoveRight && action == ACTION_RIGHT) {	// user chooses right
				m_walkDirection = right;
				setDirection(0);
				m_ignoreFork = true;
			}
			else if (canMoveLeft && action == ACTION_LEFT) {	// user chooses left
				m_walkDirection = left;
				setDirection(180);
				m_ignoreFork = true;
			}
			else if (canMoveUp && action == ACTION_UP) {	// user chooses up
				m_walkDirection = up;
				setDirection(0);
				m_ignoreFork = true;
			}
			else if (canMoveDown && action == ACTION_DOWN) {	// user chooses down
				m_walkDirection = down;
				setDirection(0);
				m_ignoreFork = true;
			}
			else
				return;
		}
		// the user may not choose a direction if there is only one valid pathway that the Player can move in
		if(count <= 1){	
			m_ignoreFork = false;
		}
		

		//cannot Move Forward so Change Direction to a Perpendicular Direction (At a corner)
		if (!canPlayerMoveForward()) {
			if (m_walkDirection == right || m_walkDirection == left) {
				m_walkDirection = up;	// preference for up
				if (!canPlayerMoveForward())
					m_walkDirection = down;
			}
			else {
				m_walkDirection = right;	// preference for right
				if (!canPlayerMoveForward())
					m_walkDirection = left;
			}
			checkAndSetSpriteDirection();	// set the sprite direction to match the walking direction correspondingly
		}
		// Walk two pixels forward in the walking direction
		int oldX = getX(), oldY = getY();
		switch (m_walkDirection) {
		case up:
			moveTo(oldX, oldY + 2);
			break;
		case down:
			moveTo(oldX, oldY - 2);
			break;
		case right:
			moveTo(oldX + 2, oldY);
			break;
		case left:
			moveTo(oldX - 2, oldY);
			break;
		}

		// checks if Player is on a new square while walking
		if (swPtr->get_square_at_location(oldX, oldY) != swPtr->get_square_at_location(getX(), getY())) {
			justMoved = true;
		}
		else {
			justMoved = false;
		}
			
		m_ticksToMove--;	// decrements ticks left to move which was determined by the dice roll
		if (m_ticksToMove <= 0) {	// sets status to walking once there are no more ticks left from the dice roll
			m_isWalking = false;	
			justLanded = true;
			justMoved = false;
		}
	}
}

void Player::force_walk_direction(DIR dir, int angle) {
	m_walkDirection = dir;
	setDirection(angle);
}

void Player::swap_positions() {
	StudentWorld* sw = get_ptr_to_student_world();
	Player* other = sw->get_other_player(this);
	// Swap Positions
	int myX = getX(), myY = getY();
	moveTo( other->getX(), other->getY());
	other->moveTo(myX, myY);

	// Swap Walking Directions
	DIR tempDir = m_walkDirection;
	m_walkDirection = other->getWalkingDirection();
	other->force_walk_direction(tempDir, getDirection());

	// Assign sprite direction correspondingly
	checkAndSetSpriteDirection();
}

void Player::swap_stars() {
	StudentWorld* sw = get_ptr_to_student_world();
	Player* peach = sw->getPlayer(0);
	Player* yoshi = sw->getPlayer(1);
	int tempStars = peach->get_stars();
	peach->reset_stars();
	peach->adjust_stars(yoshi->get_stars());
	yoshi->reset_stars();
	yoshi->adjust_stars(tempStars);
}

void Player::swap_coins() {
	StudentWorld* sw = get_ptr_to_student_world();
	Player* peach = sw->getPlayer(0);
	Player* yoshi = sw->getPlayer(1);
	int tempCoins = peach->get_coins();
	peach->reset_coins();
	peach->adjust_coins(yoshi->get_coins());
	yoshi->reset_coins();
	yoshi->adjust_coins(tempCoins);
}

void Player::teleport_me_to_random_sq() {
	StudentWorld* sw = get_ptr_to_student_world();
	Actor* square = sw->get_random_square();
	// Moves to Random Square
	if(square != nullptr)
		moveTo(square->getX(), square->getY());

	// Chooses Random Direction
	if (!canPlayerMoveForward()) {		//Avatar has invalid direction (from being teleported)
		while (!canPlayerMoveForward()) {
			switch (randInt(1, 4)) {
			case 1:
				m_walkDirection = right;
				break;
			case 2:
				m_walkDirection = left;
				break;
			case 3:
				m_walkDirection = up;
				break;
			default:
				m_walkDirection = down;
				break;
			}
		}
		checkAndSetSpriteDirection();	// sets sprite direction correspondingly
	}
}

bool Player::canWalkInDirection(DIR aDirection) const{
	StudentWorld* swPtr = get_ptr_to_student_world();
	int xnew, ynew;
	getPositionInThisDirection(aDirection, 16, xnew, ynew);

	//testing for out of bounds
	if (xnew < 0 || xnew >= 256 || ynew < 0 || ynew >= 256)
		return false;

	return swPtr->is_there_a_square_at_location(xnew, ynew);	// if there is a square in the next position, then the player may walk there
}

bool Player::canPlayerMoveForward() {
	StudentWorld* swPtr = get_ptr_to_student_world();

	// looks at the next position in the walking direction
	int xnew, ynew;
	if (m_walkDirection == left || m_walkDirection == down)
		getPositionInThisDirection(m_walkDirection, 1, xnew, ynew);
	else
		getPositionInThisDirection(m_walkDirection, 16, xnew, ynew);

	//testing for out of bounds
	if (xnew < 0 || xnew >= 256 || ynew < 0 || ynew >= 256)
		return false;

	return swPtr->is_there_a_square_at_location(xnew, ynew);	// if there is a square in the next position, then the player may walk there
}

void Player::checkAndSetSpriteDirection() {	// sets the sprite direction according to the walking direction
	if (m_walkDirection == left)
		setDirection(left);
	else
		setDirection(right);
}
//------------------------------------------
// ActivatingObject Implementation
ActivatingObject::ActivatingObject(StudentWorld* sw, int imageID, int startX, int startY,
	int dir, double size, int depth): Actor(sw, imageID, startX, startY, dir, size, depth)	// ActivatingObject constructor
{}

void ActivatingObject::do_something(){}
//------------------------------------------
// ActivateOnPlayer Implementation
ActivateOnPlayer::ActivateOnPlayer(StudentWorld* sw, int imageID, int startX, int startY,int dir, 
	double size, int depth, bool activate_when_go_lands)	// ActivateOnPlayer constructor
	:ActivatingObject(sw, imageID, startX, startY, dir, size, depth), 
	m_activateWhenGoLands(activate_when_go_lands)
{}

bool ActivateOnPlayer::checkJustLanded(Player* player) const {	// determine if a Player is waiting on square for the first tick only
	if (player->getJustLanded() && player->getX() == getX() && player->getY() == getY())
		return true;
	return false;
}

bool ActivateOnPlayer::checkJustMoved(Player* player) const {	// determine if a Player is walking onto a square for the first tick only
	if (player->getJustMoved() && player->getX() == getX() && player->getY() == getY())
		return true;
	return false;
}
//------------------------------------------
// Vortex Implementation
Vortex::Vortex(StudentWorld* sw, int imageID, int startX, int startY, DIR dir)
	: ActivatingObject(sw,imageID, startX,startY,dir, 1, 0), m_movingDirection(dir)		//Vortex Constructor
{}

void Vortex::do_something() {
	StudentWorld* sw = get_ptr_to_student_world();
	if (!is_active())	//if vortex is not active, do nothing
		return;
	moveVortex();	// moves vortex in walking direction

	Actor* hitEnemy = vortexOverlapsWithAnEnemy();	// checks if vortex overlaps with a Bowser or Boo (even by one pixel)
	if (hitEnemy != nullptr) {
		hitEnemy->hit_by_vortex();	// tells Bowser/Boo that it was hit
		set_inactive();	// sets vortex to inactive after being used
		sw->playSound(SOUND_HIT_BY_VORTEX);
	}
}

void Vortex::moveVortex() {	//moves vortex in the walking direction of the player until the vortex hits an enemy or goes off the screen
	int oldX = getX(), oldY = getY();
	switch (m_movingDirection) {
	case up:
		moveTo(oldX, oldY + 2);
		break;
	case down:
		moveTo(oldX, oldY - 2);
		break;
	case right:
		moveTo(oldX + 2, oldY);
		break;
	case left:
		moveTo(oldX - 2, oldY);
		break;
	}

	int newX = getX(), newY = getY();
	if (newX < 0 || newX >= VIEW_WIDTH || newY < 0 || newY >= VIEW_HEIGHT)	// vortex goes off the screen
		set_inactive();
}

Actor* Vortex::vortexOverlapsWithAnEnemy() const {	// returns the enemy that is overlapping with the vortex (nullptr if none)
	StudentWorld* sw = get_ptr_to_student_world();
	Actor* enemy = sw->get_enemy_at_location(getX(), getY());
	return enemy;
}
//------------------------------------------
// StarSquare Implementation
StarSquare::StarSquare(StudentWorld* sw, int imageID, int startX, int startY)	// StarSquare constructor
	: ActivateOnPlayer(sw, imageID, startX, startY, right,1,1,false)
{}

void StarSquare::do_something() {
	StudentWorld* sw = get_ptr_to_student_world();
	for (int i = 0; i < 2; i++) {
		Player* myPlayer = sw->getPlayer(i);
		if (checkJustLanded(myPlayer) || checkJustMoved(myPlayer)) {	// the Player landed or passes onto a StarSquare
			if (myPlayer->get_coins() >= 20){	// Player replaces 20 coins for a star (if the player has at least 20 coins
				myPlayer->adjust_coins(-20);
				myPlayer->adjust_stars(1);
				sw->playSound(SOUND_GIVE_STAR);
			}
		}
	}
}
//------------------------------------------
// CoinSquare Implementation
CoinSquare::CoinSquare(StudentWorld* sw, int imageID, int startX, int startY, int adjust_coins_by) //CoinSquare constructor
	:ActivateOnPlayer(sw,imageID, startX,startY, right,1,1, true), m_adjustCoinsBy(adjust_coins_by)
{}

void CoinSquare::do_something() {
	StudentWorld* sw = get_ptr_to_student_world();
	for (int i = 0; i < 2; i++) {
		Player* myPlayer = sw->getPlayer(i);
		if (checkJustLanded(myPlayer)) {	// checks if a player landed on the CoinSquare
			if (m_adjustCoinsBy > 0) {
				myPlayer->adjust_coins(m_adjustCoinsBy);	// gives 3 coins to the player (blue coinsquare)
				sw->playSound(SOUND_GIVE_COIN);
			}
			else {		// removes 3 coins from the player (red coinsquare)
				if (myPlayer->get_coins() >= 3) {
					myPlayer->adjust_coins(m_adjustCoinsBy);	
					sw->playSound(SOUND_TAKE_COIN);
				}
				else
					myPlayer->reset_coins();
				sw->playSound(SOUND_TAKE_COIN);
			}
		}
	}
}
//------------------------------------------
// DirectionalSquare Implementation
DirectionalSquare::DirectionalSquare(StudentWorld* sw, int imageID, int startX, int startY,
	DIR dir, int angle)	// DirectionalSquare constructor
	: ActivateOnPlayer(sw, imageID, startX, startY, dir,1,1, false)
{
	m_angle = angle;
	m_facingDirection = dir;
}

void DirectionalSquare::do_something() {
	StudentWorld* sw = get_ptr_to_student_world();
	for (int i = 0; i < 2; i++) {
		Player* myPlayer = sw->getPlayer(i);
		if (checkJustLanded(myPlayer) || checkJustMoved(myPlayer)) {	// player lands or passes onto a Directional Square
			myPlayer->force_walk_direction(m_facingDirection, m_angle);	// player moves in the directional square's direction
			myPlayer->setIgnoreFork(true);	// player does not choose direction if the directional square is a pathway at a fork
		}
	}
}
//------------------------------------------
//BankSquare Implementation
BankSquare::BankSquare(StudentWorld* sw, int imageID, int startX, int startY)	//BankSquare constructor
	:ActivateOnPlayer(sw, imageID, startX, startY, right, 1, 1, false)
{}

void BankSquare::do_something() {
	StudentWorld* sw = get_ptr_to_student_world();
	for (int i = 0; i < 2; i++) {
		Player* myPlayer = sw->getPlayer(i);
		if (checkJustLanded(myPlayer)) {		//player lands on BankSquare
			int coinsInBank = sw->get_bank_coins();
			myPlayer->adjust_coins(coinsInBank);	//give Player coins in bank and empty the bank
			sw->reset_bank_coins();
			sw->playSound(SOUND_WITHDRAW_BANK);
		}
		else if (checkJustMoved(myPlayer)) {	// player passes on BankSquare
			int numCoins = myPlayer->get_coins();
			if (numCoins >= 5) {
				sw->deposit_bank_coins(5);		//deduct 5 coins from the Player (if they have that many) and give to the bank
				myPlayer->adjust_coins(-5);
			}
			else {
				sw->deposit_bank_coins(numCoins);
				myPlayer->reset_coins();
			}	
			sw->playSound(SOUND_DEPOSIT_BANK);
		}
	}
}
//------------------------------------------
// EventSquare Implementation
EventSquare::EventSquare(StudentWorld* sw, int imageID, int startX, int startY)	//EventSquare constructor
	: ActivateOnPlayer(sw, imageID, startX, startY, right, 1, 1, true)
{}

void EventSquare::do_something() {
	StudentWorld* sw = get_ptr_to_student_world();
	for (int i = 0; i < 2; i++) {
		Player* myPlayer = sw->getPlayer(i);
		if (checkJustLanded(myPlayer)) {	// player lands on EventSquare
			int random = randInt(1, 3);	// 33% chance for each option
			switch (random) {
			case 1:	// Teleported to Random Square
			  myPlayer->teleport_me_to_random_sq();
				sw->playSound( SOUND_PLAYER_TELEPORT );
				myPlayer->setIgnoreFork(true);
				break;
			case 2:	// Swap Players
				myPlayer->swap_coins();
				myPlayer->swap_stars();
				myPlayer->swap_positions();
				sw->playSound(SOUND_PLAYER_TELEPORT);
				myPlayer->setIgnoreFork(true);
				break;
			default:	// Give the player a Vortex Projectile
				myPlayer->equip_with_vortex_projectile();
				sw->playSound(SOUND_GIVE_VORTEX);
				break;
			}
		}
	}
}
//------------------------------------------
// Enemy Implementation
Enemy::Enemy(StudentWorld* sw, int imageID, int startX, int startY,int dir, double size, 
	int depth, bool activate_when_go_lands, int num_sq_to_move, int number_of_ticks_to_pause)	//Enemy Constructor
	:ActivateOnPlayer(sw, imageID, startX, startY, dir, size,depth,activate_when_go_lands),
	m_num_sq_to_move(num_sq_to_move), m_number_of_ticks_to_pause(number_of_ticks_to_pause)
{
	m_ticks_to_move = 0;
	m_is_walking = false;
	m_walkDirection = right;
	m_ignoreFork = true;
	justLanded = false;
	justMoved = false;
}

void Enemy::do_something() {
	// Waiting Status
	if (!m_is_walking) {
		enemyOnSquareWithPlayer();	// enemy and player on same square
		justLanded = false;
		m_number_of_ticks_to_pause--;
		if (m_number_of_ticks_to_pause <= 0) {
			m_num_sq_to_move = randInt(1, 10);	// Enemy mimics a dice roll after waiting 180 ticks to pause
			m_ticks_to_move = m_num_sq_to_move * 8;
			// Pick a new Random Direction
			randWalkDirection();
			m_is_walking = true;
		}
	}
	// Walking Status
	if (m_is_walking) {
		atAFork();
		//cannot Move Forward and Change Direction Perpendicular (At a corner)
		atATurningPoint();

		// Walk two pixels forward
		walkForward();

		m_ticks_to_move--;
		if (m_ticks_to_move <= 0) {
			m_is_walking = false;
			m_number_of_ticks_to_pause = 180;
			justLanded = true;
			justMoved = false;
			dropSquare();
		}
	}
}

void Enemy::hit_by_vortex() {
	StudentWorld* sw = get_ptr_to_student_world();
	Actor* square = sw->get_random_square();
	moveTo(square->getX(), square->getY());	// Enemy moves to random square if hit by a vortex
	m_walkDirection = right;	// walk and sprite direction set to default
	setDirection(0);
	m_is_walking = false;
	m_number_of_ticks_to_pause = 180;	// enemy is in a waiting state
}

void Enemy::randWalkDirection() {
		switch (randInt(1, 4)) {	// chooses a random walking direction
		case 1:
			m_walkDirection = right;
			break;
		case 2:
			m_walkDirection = left;
			break;
		case 3:
			m_walkDirection = up;
			break;
		default:
			m_walkDirection = down;
			break;
		}
	checkAndSetSpriteDirection();	//sets sprite direction accordingly
}

void Enemy::checkAndSetSpriteDirection() {	//sets the sprite direction based on walking direction
	if (m_walkDirection == left)
		setDirection(left);
	else
		setDirection(right);
}

bool Enemy::canWalkInDirection(int aDirection) const {	//checks if an enemy can walk in the given direction
	StudentWorld* swPtr = get_ptr_to_student_world();
	int xnew, ynew;
	getPositionInThisDirection(aDirection, 16, xnew, ynew);

	//testing for out of bounds
	if (xnew < 0 || xnew >= 256 || ynew < 0 || ynew >= 256)
		return false;

	return swPtr->is_there_a_square_at_location(xnew, ynew);	// if there is a square at next position, then enemy can walk there
}

bool Enemy::canEnemyMoveForward() const{	// checks if enemy can move forward in its walking direction
	StudentWorld* swPtr = get_ptr_to_student_world();

	int xnew, ynew;
	if (m_walkDirection == left || m_walkDirection == down)
		getPositionInThisDirection(m_walkDirection, 1, xnew, ynew);
	else
		getPositionInThisDirection(m_walkDirection, 16, xnew, ynew);

	//testing for out of bounds
	if (xnew < 0 || xnew >= 256 || ynew < 0 || ynew >= 256)
		return false;

	return swPtr->is_there_a_square_at_location(xnew, ynew); // if there is a square in the next direction, then enemy can walk there
}

void Enemy::enemyOnSquareWithPlayer() {	// determines if an enemy is on the same square as a player
	StudentWorld* sw = get_ptr_to_student_world();
	for (int i = 0; i < 2; i++) {
		Player* player = sw->getPlayer(i);
		if (!player->is_walking() && player->getX() == getX() && player->getY() == getY() &&
			(player->getJustLanded() || justLanded)) {	// checks if a player just moved onto a square or if an enemy just moved onto a square
			doToPlayer(player);
		}
	}
}

void Enemy::atATurningPoint() {	// decides where an enemy should move if at a corner or turning point
	if (!canEnemyMoveForward()) {
		if (m_walkDirection == right || m_walkDirection == left) {
			m_walkDirection = up;	//preference for up
			if (!canEnemyMoveForward())
				m_walkDirection = down;
		}
		else {
			m_walkDirection = right;	// preference for right
			if (!canEnemyMoveForward())
				m_walkDirection = left;
		}
		checkAndSetSpriteDirection();	// sets sprite direction accordingly
	}
}

bool Enemy::atAFork() {
	// Checks what directions an enemy can Move in 
	int count = 0;
	bool result = false;
	if (getX() % 16 == 0 && getY() % 16 == 0) {
		if (canWalkInDirection(right)) {
			count++;
		}
		if (canWalkInDirection(left)) {
			count++;
		}
		if (canWalkInDirection(up)) {
			count++;
		}
		if (canWalkInDirection(down)) {
			count++;
		}
	}
	// At a Fork/Intersection
	if (count > 2 && m_ignoreFork == false) {		//NOTE that can walk backwards at fork
		randWalkDirection();	// make enemy walk in a random direction at a fork
		result = true;
	}
	if (count <= 2) {	// if the enemy is not at a fork then ignore these options
		m_ignoreFork = false;
	}
	return result;
}

void Enemy::walkForward() {	// makes the enemy walk forward in it walking direction
	StudentWorld* swPtr = get_ptr_to_student_world();
	int oldX = getX(), oldY = getY();
	switch (m_walkDirection) {
	case up:
		moveTo(oldX, oldY + 2);
		break;
	case down:
		moveTo(oldX, oldY - 2);
		break;
	case right:
		moveTo(oldX + 2, oldY);
		break;
	case left:
		moveTo(oldX - 2, oldY);
		break;
	}

	// checks if enemy is on a new square
	if (swPtr->get_square_at_location(oldX, oldY) != swPtr->get_square_at_location(getX(), getY())) {
		justMoved = true;
	}
	else {
		justMoved = false;
	}
}
//------------------------------------------
// DroppingSquare Implementation
DroppingSquare::DroppingSquare(StudentWorld* sw, int imageID, int startX, int startY)
	: ActivateOnPlayer(sw, imageID, startX, startY, right, 1, 1, true)
{}

void DroppingSquare::do_something() {
	StudentWorld* sw = get_ptr_to_student_world();
	for (int i = 0; i < 2; i++) {
		Player* myPlayer = sw->getPlayer(i);
		if (checkJustLanded(myPlayer)) {	// player just landed
			if (randInt(1, 2) == 1) {	// 50% chance
				int coins = myPlayer->get_coins();
				if (coins < 10) {
					myPlayer->reset_coins();	
				}
				else {
					myPlayer->adjust_coins(-10);	// player loses 10 coins
				}
			}
			else {
				if (myPlayer->get_stars() > 1) {
					myPlayer->adjust_stars(-1);	// player loses 1 star
				}
			}
			sw->playSound(SOUND_DROPPING_SQUARE_ACTIVATE);
		}
	}
}
//------------------------------------------
// Bowser Implementation
Bowser::Bowser(StudentWorld* sw, int imageID, int startX, int startY)
	:Enemy(sw, imageID, startX, startY, right, 1,0, true,0,180 )		
{}

void Bowser::doToPlayer(Player* player) {	// when interacting with a player, bowser resets coins and starts 50% of the time
	StudentWorld* sw = get_ptr_to_student_world();
	int rand = randInt(1, 2);	// 50 % chance
	if (rand == 1){
		player->reset_coins();
		player->reset_stars();
	}
	sw->playSound(SOUND_BOWSER_ACTIVATE);
}

void Bowser::dropSquare() {	// Boswer drops a dropping square
	StudentWorld* sw = get_ptr_to_student_world();
	if(randInt(1, 4) == 1)	//25% chance
	{
		Actor* square = sw->get_square_at_location(getX(), getY());
		square->set_inactive();	// makes old square at location inactive
		square = new DroppingSquare(sw, IID_DROPPING_SQUARE, getX(), getY());
		sw->add_actor(square);	// add a dropping square to the list in Studentworld
		sw->playSound(SOUND_DROPPING_SQUARE_CREATED);
	}
}
//------------------------------------------
// Boo Implementation
Boo::Boo(StudentWorld* sw, int imageID, int startX, int startY)
	: Enemy(sw, imageID, startX, startY, right, 1, 0,true ,0,180 )
{}

void Boo::doToPlayer(Player* player) {	// Boo interacting with a player
	StudentWorld* sw = get_ptr_to_student_world();
	int rand = randInt(1, 2); //50% chance
	if (rand == 1) {
		player->swap_coins();	// swap players' coins with each other
	}
	else {
		player->swap_stars();	// swap players' stars with each other
	}
	sw->playSound(SOUND_BOO_ACTIVATE);
}


