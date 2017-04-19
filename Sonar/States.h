//Define Macros to aid in Coding States

#ifndef STATES_H_
#define STATES_H_

#define MotorAccelRate 20
#define MotorDecelRate 10

#define MotorMinSpeed 200

//Define Sonar Reading Indices
#define SonarForward 1
#define SonarLeft 0
#define SonarRight 2

#define SonarNoise 150
#define SonarError 65355

#define FarDistance 1350
#define AttackDistance 100

#define SpotLeft 0
#define SpotRight 1

//Sonar Prop. Drive Parameters
#define HardTurn 0.95
#define TurnRate 1

#define AttackState	0
#define ScanState	1
#define HomeState	2

//Declare LastSeen as extern so that files can modify it
extern volatile unsigned short LastSeen;

//Declare Functions for handling states
void Scan(void);

void Attack(void);

void Home(const unsigned long SonarReading[]);

#endif /* STATES_H_ */