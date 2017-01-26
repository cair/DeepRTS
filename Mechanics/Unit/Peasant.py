from Mechanics.Constants import Unit as UnitC
from Mechanics.Unit.Unit import Unit


class Peasant(Unit):

    # Identifier
    id = UnitC.PEASANT
    structure = UnitC.UNIT

    # Tile Data
    width = 1
    height = 1

    # Economics
    cost_lumber = 0
    cost_gold = 400

    # Stats
    name = "Peasant"
    damage_min = 2
    damage_max = 9
    damage_piercing = 2
    damage_range = 1
    armor = 0
    health = 30
    health_max = 30
    speed = 10
    sight = 4

    def __init__(self, player):
        super().__init__(player)
        self.player = player

        self.can_build = [
            player.unit_manager.UNITS[player.unit_manager.BASE_1][player.race],
            player.unit_manager.UNITS[player.unit_manager.FARM][player.race],
            player.unit_manager.UNITS[player.unit_manager.BARRACKS][player.race]
        ]



