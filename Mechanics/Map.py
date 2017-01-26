import os
import pygame
import sys
import os
import math
import random
import numpy as np

from Mechanics.Constants import Map as MapC
from Mechanics.Constants import Unit as UnitC
from Mechanics.Util import ArrayUtil


class Map:

    TILES_THEME = "summer"

    def __init__(self, game, map_name, n_players):
        self.tiles = None       # Raw tile data (Numbers)
        self.game = game

        # Parse Map
        with open(os.path.join('./data/maps/', map_name + ".map")) as f:
            self.tiles = [[int(digit) for digit in line.split()] for line in f]
            self.tiles = np.array(self.tiles, dtype=np.int)   # Convert to np.array
            self.spawn_points = self._spawn_points()

        self.MAP_WIDTH = len(self.tiles[0])
        self.MAP_HEIGHT = len(self.tiles)

    def _spawn_points(self):
        spawn_tiles = np.where(self.tiles == MapC.SPAWN_POINT)
        spawn_tiles = list(zip(*spawn_tiles))

        # Remove spawn points
        for (x, y) in spawn_tiles:
            self.tiles[x][y] = MapC.GRASS

        return spawn_tiles

    def free_tiles(self):
        """

        :return: all tiles that units can be placed on
        """

        # Environment tiles
        env_tiles = np.where(self.tiles == MapC.GRASS)
        env_tiles = set(zip(*env_tiles))

        # Unit tiles
        unit_tiles = np.where(self.game.unit_map == UnitC.NONE)
        unit_tiles = set(zip(*unit_tiles))

        common = list(env_tiles.intersection(unit_tiles))

        return common

    def walkable_neighbor_tiles(self, x, y, d):
        # All possible tiles
        neighbors = ArrayUtil.neighbors(self.tiles, x, y, d + 1)
        neighbors.append((x, y))

        # Filter occupied tiles
        neighbors = [(x, y) for x, y in neighbors if self.game.unit_map[x][y] == UnitC.NONE]

        # Filter non walkable tiles
        neighbors = [(x, y) for x, y in neighbors if self.tiles[x][y] == MapC.GRASS]

        return neighbors

    def can_build_here(self, builder, x, y, d):

        neighbors = ArrayUtil.neighbors(self.tiles, x, y, d)

        common = list(set(neighbors).intersection(set(self.free_tiles())))

        return len(common) == len(neighbors)

    def get_tile(self, x, y):
        tile_id = self.tiles[x][y]
        tile = MapC.TILE_DATA[tile_id]
        return tile


