Somehow, my files have gotten the directions, and numbering mixed up.

Here is what I want:
// inits for the controller unit
// NE has all relays off and is perfect for default
int last_button = 0;  //initialize last button pressed, default to NE

//directions
#define NE 0
#define E  1
#define SE 2
#define S  3
#define SW 4
#define W  5
#define NW 6
#define N  7

int direction = 0;  //current position

And in the phaser:

//array of relay status for each position
boolean relay[8][6]{ //remoteqth 
  {LOW,  LOW,  LOW,  LOW,  LOW,  LOW},  //NE 0
  {LOW,  LOW,  HIGH, HIGH, LOW,  HIGH}, //E 1
  {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH}, //SE 2
  {LOW,  HIGH, HIGH, LOW,  LOW,  HIGH}, //S 3
  {LOW,  LOW,  LOW,  LOW,  HIGH, HIGH}, //SW 4
  {HIGH, HIGH, LOW,  LOW,  LOW,  HIGH}, //W 5
  {HIGH, HIGH, HIGH,  HIGH, LOW,  LOW},  //NW 6
  {HIGH, LOW,  LOW,  HIGH, LOW,  LOW}   //N 7
};