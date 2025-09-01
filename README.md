# FRUIT STORM

Author: Zhijian Wang

Design: Don't let them catch your fruit! Throw fruits for a minute to get more points from uncaught fruits.

Screen Shot:

![Screen Shot](screenshot.png)

How Your Asset Pipeline Works:

- Tiles and palette are first read from PNG files, which each PNG consisting at most 4 colors and the width/height pixel multipliable by 8. All original PNGs are located in /dist folder. The PNG parsing tool function will first scan the PNG and create a palette, and use the palette to store tiles into the tile table.
- The tool functions will then save the loaded tile table and palette table into the game.assets in /dist folder.
- After executing the game, it will load the game.assets and read the preloaded tile table and palette table without the need of PNG files.
- The scene consists of a controllable player, a moving catcher, dropping fruits, and some number texts(made from sprites). Considering the number of static game objects in the game and that all game objects in the game are moving/adding/removing/switching sprites all the time, the scene is not loaded from asset file but built in runtime using the loaded tiles and palettes. The rendered sprites are gathered from the game object list in the scene before rendering and uploaded to PPU466.
- The background is simply a random spreading of four different grass tiles. Considering the simplicity of the background in the game, its placement is dynamically coded with loops (just like the demo) in runtime. But their tiles are read from the loaded tile table. 

PNG files and assets:

[Background](dist/background.png),
[Cherry and Watermelon](dist/fruits1.png),
[Orange and Banana](dist/fruits2.png),
[Player and Catcher](dist/characters.png),
[Number and Text](dist/nums.png),
[Game files:](dist/game.assets)


How To Play:

The player needs to control the fruit throwing position and determine which type of fruit to throw.

Fruits have different attributes:
- Cherry: normal speed, normal acc, short cooldown
- Watermelon: low speed, high acc, long cooldown
- Orange: high speed, no acc, normal cooldown
- Banana: normal speed, low acc, normal cooldown

Control: LEFT/RIGHT to control the throwing position, UP/DOWN/SPACE to switch to another fruit. Fruits drop automatically.

The catcher (a bowl) will try to catch all the fruits coming down, but it is slow (at least compare to the player's speed). If the fruit is not caught by the bowl, the player will get points. The player has 1 minutes to try to get the highest point. Both the time and the point are shown on the top right of the screen.


This game was built with [NEST](NEST.md).

