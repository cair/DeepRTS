//
// Created by Per-Arne on 24.02.2017.
//


#include "Player.h"
#include "../Game.h"
#include "../util/ColorConverter.hpp"
#include "../algorithms/RANDOM/AlgoRandom.h"
#include "../unit/Unit.h"
#include "../unit/UnitManager.h"
#include <algorithm>



Player::Player(Game &game, int id) : game_(game)
{
	id_ = id;
	name_ = "Player: " + std::to_string(id_);

	std::tuple<uint8_t , uint8_t, uint8_t> colors[4] = { // TODO use Color generator
			std::tuple<uint8_t , uint8_t, uint8_t>(255, 0, 0),
			std::tuple<uint8_t , uint8_t, uint8_t>(0, 0, 255),
			std::tuple<uint8_t , uint8_t, uint8_t>(0, 255, 0),
			std::tuple<uint8_t , uint8_t, uint8_t>(0, 255, 255),
	};
	playerColor = colors[id_];
	



	unitIndexes.reserve(1000);
	faction = 0;
	gold = 1500;
	lumber = 750;
	oil = 0;
	foodConsumption = 0;
	food = 1;
	defeated = false;

	statGoldGather = 0;
	statLumberGather = 0;
	statOilGather = 0;
	statUnitDamageDone = 0;
	statUnitDamageTaken = 0;
	statUnitBuilt = 0;
	statUnitMilitary = 0;

}


Unit& Player::spawn(Tile &spawnPoint) {
	// Spawn a builder


	Unit *unit = NULL;
	if (faction == 0) // Human
	{

		unit = &addUnit(Constants::Unit::Peasant);
	}
	else if (faction == 1) // Orc
	{
		unit = &addUnit(Constants::Unit::Peasant);
	}
	else {
		assert(false);
	}

	assert(unit && "Unit was null for some reason");
	unit->spawn(spawnPoint, unit->spawnDuration);

	// Set targeted unit to newly spawned unit
	targetedUnitID = unit->id;


	return *unit;
}

void Player::update() {


	if (!actionQueue.empty()) {
		Constants::Action actionID = actionQueue.front();
		actionQueue.pop_front();

		if (!getTargetedUnit() and (actionID != Constants::Action::NextUnit and actionID != Constants::Action::PreviousUnit) or unitIndexes.empty()) {
			// No selected unit by the player and he attempts to right click on a targetedUnit
			return;
		}

		Unit *targetedUnit = getTargetedUnit();

		switch (actionID) {
			case Constants::Action::NextUnit:
				nextUnit();
				break;
			case Constants::Action::PreviousUnit:
				previousUnit();
				break;
			case Constants::Action::MoveUpRight:
				targetedUnit->tryMove(-1, 1);
				break;
			case Constants::Action::MoveUpLeft:
				targetedUnit->tryMove(-1, -1);
				break;
			case Constants::Action::MoveDownRight:
				targetedUnit->tryMove(1, 1);
				break;
			case Constants::Action::MoveDownLeft:
				targetedUnit->tryMove(1, -1);
				break;
			case Constants::Action::MoveUp:
				targetedUnit->tryMove(0, -1);
				break;
			case Constants::Action::MoveDown:
				targetedUnit->tryMove(0, 1);
				break;
			case Constants::Action::MoveLeft:
				targetedUnit->tryMove(-1, 0);
				break;
			case Constants::Action::MoveRight:
				targetedUnit->tryMove(1, 0);
				break;
			case Constants::Action::Attack:
				targetedUnit->tryAttack(); // TODO
				break;
			case Constants::Action::Harvest:
				targetedUnit->tryHarvest(); // TODO
				break;
			case Constants::Action::Build0:
				targetedUnit->build(0);
				break;
			case Constants::Action::Build1:
				targetedUnit->build(1);
				break;
			case Constants::Action::Build2:
				targetedUnit->build(2);
				break;

		}


	}





}

void Player::reset()
{
	faction = 0;
	gold = 1500;
	lumber = 750;
	oil = 0;
	foodConsumption = 0;
	food = 1;
	defeated = false;

	statGoldGather = 0;
	statLumberGather = 0;
	statOilGather = 0;
	statUnitDamageDone = 0;
	statUnitDamageTaken = 0;
	statUnitBuilt = 0;
	statUnitMilitary = 0;
	unitIndexes.clear();
	actionQueue.clear();
	targetedUnitID = -1;
	actionStatistics[20] = { 0 };

	if (algorithm_) {
		algorithm_->reset();
	}
}



int Player::getFoodConsumption() {
	return foodConsumption;
}

int Player::getFood() {
	return food;
}

int Player::getGold() {
	return gold;
}

int Player::getOil() {
	return oil;
}

int Player::getLumber() {
	return lumber;
}

size_t Player::getUnitCount() {
	return unitIndexes.size();
}

int Player::getId() {
	return id_;
}

void Player::addGold(int n) {
	gold += n;
}

void Player::addLumber(int n) {
	lumber += n;
}

void Player::addOil(int n) {
	oil += n;
}

int Player::getScore() {
	uint8_t GOLD_VALUE = 2;
	uint8_t LUMBER_VALUE = 1;
	uint8_t OIL_VALUE = 3;


	double_t gatherScore = (statGoldGather * GOLD_VALUE + statLumberGather * LUMBER_VALUE) * .5;
	double_t builtScore = statUnitBuilt;
	double_t damageScore = std::max(0.0, statUnitDamageDone - (statUnitDamageTaken * .5));
	double_t unitScore = unitIndexes.size();


	double_t militaryScore = 0;
	double_t defenceScore = 0;


	return static_cast<uint32_t>(gatherScore + builtScore + unitScore + militaryScore + defenceScore + damageScore);

}

void Player::removeUnit(Unit & unit) {
	for (auto &p : game_.players) {
		if (p.targetedUnitID == unit.id) {
			p.targetedUnitID = -1;
		}
	}

	unit.removedFromGame = true;

	ptrdiff_t pos = std::find(game_.units.begin(), game_.units.end(), unit.id) - game_.units.begin();
	unitIndexes.erase(std::remove(unitIndexes.begin(), unitIndexes.end(), pos), unitIndexes.end());

	// If no more units in the index list, clear the action queue
	if (unitIndexes.empty()) {
		actionQueue.clear();
	}
	//units.erase(std::remove(units.begin(), units.end(), unit), units.end());
	//std::cout << "Implement removeUnit" << std::endl;

}

bool Player::checkDefeat() {
	if (defeated) return defeated;
	if (unitIndexes.size() > 0) return false;

	defeated = true;
	return defeated;
}

bool Player::canPlace(Unit & builder, Unit & unit, Tile *_tile) {

	for (auto &tile : game_.map.getTiles(_tile, unit.width, unit.height)) {
		if (tile == builder.tile) // Ignore tile of the builder, this is handled in Unit::Build when builder despawns
			continue;

		if (!tile->isBuildable()) {
			return false;
		}

	}

	return true;
}

bool Player::canAfford(Unit & unit) {
	return gold >= unit.goldCost and lumber >= unit.lumberCost and oil >= unit.oilCost;

}

Unit& Player::addUnit(Constants::Unit unitType) {


	game_.units.push_back(UnitManager::constructUnit(unitType, this));
	uint16_t id = game_.units.back().id;
	unitIndexes.push_back(id);
	return game_.units.back();
}

void Player::subOil(int n) {
	oil -= n;
}

void Player::subLumber(int n) {
	lumber -= n;
}

void Player::subGold(int n) {
	gold -= n;
}

void Player::setName(std::string name) {
	name_ = name;
}

void Player::setAlgorithm(Algorithm *theAlg) {
	theAlg->setPlayer(this);
	algorithm_ = theAlg;
}

int Player::_getNextPrevUnitIdx() {
	
	if (unitIndexes.size() == 0) {
		return -1;
	}

	if (targetedUnitID == -1) {
		return 0;
	}

	uint16_t idx = 0;
	for (auto &uIDX : unitIndexes) {
		if (targetedUnitID == uIDX) {
			return idx;
		}
		idx++;
	}

	return -1;
}

void Player::nextUnit() {
	//assert(unitIndexes.size() == 0 && "AI is attempting to play on a terminal state!");

	// Get which index in the unitIndexes to lookup
	int idx = _getNextPrevUnitIdx();

	// Just return if no next unit is returned
	if (idx == -1)
		return;

	// Increment since we want Next unit
	idx++;

	// Retrieve which index it has
	uint16_t unitIndex = unitIndexes[idx % unitIndexes.size()];
	//std::cout << static_cast<int>(unitIndex) << " -N- " << unitIndexes.size() << std::endl;
	// Get unit from the game vector
	targetedUnitID = unitIndex;

	return;
}

void Player::previousUnit() {
	//assert(unitIndexes.size() == 0 && "AI is attempting to play on a terminal state!");

	// Get which index in the unitIndexes to lookup
	int idx = _getNextPrevUnitIdx();

	// Just return if no next unit is returned
	if (idx == -1)
		return;

	// Increment since we want Previous unit
	idx--;

	// Retrieve which index it has
	uint16_t unitIndex = unitIndexes[idx % unitIndexes.size()];
	//std::cout << static_cast<int>(unitIndex) << " -P- " << unitIndexes.size() << std::endl;

	// Get unit from the game vector
	targetedUnitID = unitIndex;

	return;
}

Unit *Player::getTargetedUnit() {
	if(targetedUnitID == -1) {
		return NULL;
	}
	return &game_.units[targetedUnitID];
}

void Player::queueAction(Constants::Action actionID) {
	apm_counter++;
	actionStatistics[actionID] += 1; // Increment statistics
	actionQueue.push_back(actionID);
}

size_t Player::getQueueSize()
{
	return actionQueue.size();
}
