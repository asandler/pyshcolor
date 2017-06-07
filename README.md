# pyshcolor
Simple ncurses-based console game. Stay alive as long as possible avoiding ever-growing number of enemies.

Control the '@' object with arrows, avoid asterisks. You have three lives at start.

Score is a sum of segment lengths of your trajectory. If you make 90 degree turn, the last segment is multiplied by two, if you go 180 degree turn, it is just added to the sum.

![pyshcolor](/pyshcolor.png)
