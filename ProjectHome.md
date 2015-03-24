# An open source project created to simulate plan and predict orbital trajectories and other aspects of satellite operation. #
Updates:
  * Orbital elements are now being calculated at all times for the satellite and can be previewed as an orbital plane that can be switched on and off.
  * The earth moon is now positioned according to the Meeus Ch45 series and is accurately showing the phase for the current date.
  * A model of the earths magnetosphere has been implemented, tracing the field lines. It's based on the Geopack 2008 fortran source code written by Dr. N. Tsyganenko from the University of St. Petersburg. Currently it is calculated once because it is far to slow to be calculated in realtime, however I'm planning on a movie export function to simulate solar flare responses etc.
  * The interplanetary trajectory planning mode has been significantly improved, there will be a listbox to choose the departure and arrival planet. A launch window will then be calculated and the lambert problem will be solved a few thousand times to determine an optimal trajectory by testing different dates in a margin close to the launch date, minimizing delta V requirements. So far I've been concentrating on a earth mars trajectory. For the future I'm planning to read myself into swing by maneuvers to find multi gravity assisted trajectories (MGA's) to the outer planets.

enjoy.