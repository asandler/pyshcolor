# pyshcolor
Simple ncurses-based console game. Stay alive as long as possible avoiding ever-growing number of enemies.

Control the '@' object with arrows, avoid asterisks. You have three lives at the start.

Score is calculated as a sum of lengths of your trajectory segments. If you make 90 degrees turn, the last segment
is multiplied by two, if you go 180 degrees turn, it is just added to the sum.

![pyshcolor](/pyshcolor.png)
