# Genius Square Board plan

## Hardware
Display Pins
* MOSI: 5
* SCLK: 4
* CS: 11
* DC: 12
* RST: 13
* 
Rotary Encoder Pins
* A (Phase): 17
* B (Phase): 18
* Button: 16

Matrix Pins
* Rows: 14, 47, 48, 38, 41, 42
* Columns (ADC): 7, 8, 10, 9, 1, 2

## Modes
1. Solver
2. Practice
3. Multiplayer

### Practice
- You select an arrangement from a list of arrangements in the menu, as a carousel of slides 
- the carousel arrangement tile would have a thumbnail of the arrangement, and the name of the arrangement and the top score for that arrangement
- The first slide of the carousel is BACK button, which takes you back to the main menu, then a random arrangement, then the rest of the arrangements
- After selecting the arrangement it is taken to the game screen, where they can play the game and try to beat the top score for that arrangement
- After completing the game, they are taken to a score screen, where they can see their score and the top score for that arrangement, and a button to go back to the main menu, if they beat the top score, the top score will be replaced in EEPROM

### Solver
- This mode is a tool for users to solve arrangements, by using the 6x6 grid scanner.
- It will scan the grid and display the grid layout, and show the placed circular blockers on the grid
- It will check the placed blockers, if there isnt the right amount, it will fill in all empty spaces on the UI grid with X's, and say below that there are too many or too few blockers. If there are the right amount of blockers, then it will check if the blockers are valid, if they aren't then it will fill in all empty spaces on the UI grid with X's, and say below that it is an invalid placement. If the blockers are valid, then it will show the user the solution for that arrangement, and a button to go back to the main menu.
- When the user clicks the button to show the solution, it will calculate and show the solution for that arrangement, and a button to go back to the main menu.

### Multiplayer
- When you click on the multiplayer icon from the main menu, it will take you to a screen where you select to host the game or join a game, another button to go back to main menu.
- if you select to host it will create a room and display the room code (a 5 digit alphabet code)
- if you select to join, it will list all the available rooms, and you can select one to join. This will be in a scrolling menu screen and display the room code with the num of people in it. The top option is to go back to multiplayer menu
- This is all done with the ESP-NOW protocol, so it is all done locally without the need for internet connection, and it is very fast and low latency.
- The host will broadcast it is open for people to join frequently and its code and the number of people in the room, and when the game starts it will broadcast the arrangement to all the players
- When 2 or more players have joined the room, The host's screen button to start will be enabled, there will be an always active button with return to main menu
- It The host's arrangement will be randomly assigned by the host, all players recieve the same arrangement
- When the host starts the game this is broadcasted to all players and they enter the game mode, but with a few multiplayer specific features
- The host broadcasts its alive every 500ms as a status. If the host disconnects or stops broadcasting, all players are returned to the main menu and a message is displayed saying the host has disconnected.

### Game Mode (shared by Practice and Multiplayer)
- When the game starts it first does the blocker placement stage it works by putting them in incrementally.
- It uses the 6x6 grid scanner to check when they are put in correctly.
- ie it displays on the screen, put in A4 then it scans the grid to check A4 is placed, and if it is there and no incorrect blockers are placed. If an incorrect one is placed then it says so and doesnt go onto the next blocker. If all is correct then it moves onto the next
- During the multiplayer version it waits for all players to have placed the same blocker ie blocker 1 correctly before moving onto the next
- When the final blocker is placed correctly, It starts the timer and the user can start placing the pieces, When all pieces are placed (every grid scan has something on it) it stops the timer
- If it is in solo mode at this point it just goes to the score screen, but if it is in multiplayer mode, it will broadcast to all players that they have finished and their time, and then it will wait for all players to finish, and then it will display the leaderboard with all players times and scores, and a button to go back to the main menu.


## Code formatting
I want to use a consistent code formatting style for this project

* At the top of each file include this description
```
/*
* Created by Ed Fillingham on DATE.
*
* FILE DESCRIPTION
  */
```

* Use standard C++ formatting
* Use function, class and enum docstrings, including breifs, parameters and return values
* Use comments to explain complex logic, but avoid obvious comments
* Use descriptive variable and function names, avoid abbreviations
* Use consistent indentation and spacing
* Use header guards in header files