## 1. Establish the runtime file structure

- [x] 1.1 Add `include/main/Game.hpp` and `src/main/Game.cpp` and move the runtime entry class into that layout
- [x] 1.2 Add the project file banner to the new `Game` header and source, and use the `RPG_MAIN_GAME_HPP` include guard in the header
- [x] 1.3 Declare the runtime types in the project namespace instead of the global namespace
- [x] 1.4 Reduce `main.cpp` to application startup and handoff into the `Game` runtime object
- [x] 1.5 Add Doxygen documentation to the public `Game` header so the runtime API is self-describing

## 2. Split the game loop into lifecycle phases

- [x] 2.1 Move event polling into a dedicated runtime method responsible for close and exit handling
- [x] 2.2 Add distinct update and render methods and wire them into the main loop in event/update/render order
- [x] 2.3 Introduce frame delta time handling for the update phase so later gameplay systems can use time-based simulation

## 3. Enforce public header boundaries

- [x] 3.1 Structure `Game.hpp` to avoid direct SFML includes by using forward declarations and any required implementation indirection
- [x] 3.2 Keep SFML includes and concrete window-management details in `Game.cpp` rather than the public header

## 4. Create extension points for gameplay systems

- [x] 4.1 Add initial runtime-owned placeholders or concrete objects for future world, player, and camera coordination
- [x] 4.2 Ensure the runtime coordinates those collaborators without embedding their internal gameplay rules in the top-level loop class
- [x] 4.3 Keep the runtime buildable and behavior-equivalent to the current window-opening executable after the refactor
