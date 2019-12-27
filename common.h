/**
 * @file common.h
 * @author Hannes Brantner Matrikelnummer: 01614466
 * @date 26.10.2017
 * @brief main header file which declares all the methods the client and server are using
 * @details the definitions of the methods are done in common.c, guard clock included to prevent multiple inclusion
 **/

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdint.h>

#define DEFAULT_HOST    "localhost"
#define DEFAULT_PORT    "1280"
#define MAP_SIZE        10
#define MIN_SHIP_LEN    2
#define MAX_SHIP_LEN    4
#define SHIP_CNT_LEN2   2  
#define SHIP_CNT_LEN3   3   
#define SHIP_CNT_LEN4   1 
#define MAX_ROUNDS      80
#define SQUARE_SEA      0 
#define SQUARE_SHIP     1
#define SQUARE_DISTANCE 2   
#define SQUARE_HIT_SHIP 3 

/**
 * @brief return the absolut type of the integer i
 * @details acts like math.h abs(int i)
 * @param one integer is neede
 * @return return type is an integer
 **/
int abs(int i);

/**
 * @brief prints usage message to stderr and exits the program with EXIT_FAILURE
 * @details uses no global variables
 * @param you have to pass the strings progName and usage
 * @return return type is void
 **/
void usageError (const char *progName, const char *usage);

/**
 * @brief prints error message to stderr using strerror and exits the program with EXIT_FAILURE
 * @details the function also frees the given addrinfo struct
 * @param progName and the pointer to the affrinfo struct is needed
 * @return return type is void
 **/
void error (const char *progName, struct addrinfo *ai);

/**
 * @brief prints error message to stderr using gaierror + the given errornr i and exits the program with EXIT_FAILURE
 * @details the function also frees the given addrinfo struct
 * @param progName and the pointer to the affrinfo struct is needed
 * @return return type is void
 **/
void gaiError (const char *progName, struct addrinfo *ai, int i); 

/**
 * @brief prints error message to stderr using the meassage + info and exits the program with exit code returntype
 * @details the function does not free an addrinfo struct
 * @param progName, the two string message and info and the return type are needed
 * @return return type is void
 **/
void sError (const char *progName, const char *msg, char *info, int returnValue);

/**
 * @brief receives the msg and saves it to the address given with the storage pointer
 * @details returns -1 on error, otherwise 0
 * @param the socket file descriptor and the pointer to the uint8_t addres must be given
 * @return return type is int
 **/
int getMsg (int sockfd, uint8_t *storage);

/**
 * @brief sends the msg to the given socket file descriptor
 * @details returns -1 on error, otherwise 0
 * @param the socket file descriptor and the message must be given
 * @return return type is int
 **/
int sendMsg (uint8_t msg, int sockfd);

/**
 * @brief return 1 if there is even parity, otherwise 0
 * @details nothing
 * @param only one int is needed
 * @return return type is int
 **/
int isEvenParity (uint8_t x); 

/**
 * @brief struct for the ships
 * @details it contains the info for length, health and location
 * @param 
 * @return 
 **/
struct battleship {
    int length;
    int intCoTwo[MAX_SHIP_LEN];
    int health;
};

/**
 * @brief returns 1 if ship is vertically, otherwise 0 (horizontally)
 * @details 
 * @param the coordinates of the ship has to be given in intCoFour standard form
 * @return return type is int
 **/
int shipHorOrVer(int intCoFour); 

/**
 * @brief returns the ship length
 * @details for using with the command line arguments
 * @param one string is needed for this function
 * @return return type is int
 **/
int shipLengthStr(const char strCoFour[4]); 

/**
 * @brief returns the ship length
 * @details for using with standard intCoFour integers
 * @param one integer is needed
 * @return return type is int
 **/
int shipLengthFromIntCoFour(int intCoFour); 

/**
 * @brief enlarging the armada
 * @details the ship is added to the armada and all its variables are set
 * @param the armada, the lowest free index and coordinates in intCoFour form must be given
 * @return return type void
 **/
void initialiseShipToArmada(struct battleship armada[SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4], int index, int intCoFour);

/**
 * @brief return 0 or an error code if the coordinates in string form are unvalid
 * @details return type, 0: everything fine, 1: too long/short, 2: out of the map, 3: ship is alligned diagonally, 4: ship is too long/short
 * @param one string is needed for this function
 * @return return type is int
 **/
int testStrCoFour(char strCoFour[4]); 

/**
 * @brief converts the arguments to standard intCoFour form and returns it
 * @details necassary in use with other functions
 * @param one string is needed for this function
 * @return return type is int
 **/
int strCoFourToIntCoFour(char strCoFour[4]); 

/**
 * @brief adds a ship token to the battlefield
 * @details returns -1 on error, otherwise 0
 * @param the battlefield and the intCoTwo coordinate must be given
 * @return return value is int
 **/
int setShipPartToBattlefield (uint8_t battlefield[MAP_SIZE][MAP_SIZE], int intCoTwo); 

/**
 * @brief adds a hit ship token to the battlefield
 * @details returns -1 on error, otherwise 0
 * @param the battlefield and the intCoTwo coordinate must be given
 * @return return value is int
 **/
int setHitShipToBattlefield (uint8_t battlefield[MAP_SIZE][MAP_SIZE], int intCoTwo); 

/**
 * @brief adds a distance Token to the battlefield
 * @details returns -1 on error, otherwise 0
 * @param the battlefield and the intCoTwo coordinate must be given
 * @return return value is int
 **/
int setDistanceTokenToBattlefield (uint8_t battlefield[MAP_SIZE][MAP_SIZE], int intCoTwo); 

/**
 * @brief adds a vertical ship to the battlefield
 * @details returns -1 on error, otherwise 0
 * @param the battlefield and the start and endintCoTwo must be given
 * @return return type is int
 **/
int addShipVertically(uint8_t battlefield[MAP_SIZE][MAP_SIZE], int startIntCoTwo, int endIntCoTwo); 

/**
 * @brief adds a horizontal ship to the battlefield
 * @details returns -1 on error, otherwise 0
 * @param the battlefield and the start and endintCoTwo must be given
 * @return return type is int
 **/
int addShipHorizontally(uint8_t battlefield[MAP_SIZE][MAP_SIZE], int startIntCoTwo, int endIntCoTwo); 

/**
 * @brief adds a ship to the battlefield
 * @details returns -1 on error, otherwise 0
 * @param the battlefield and an intCoFour must be given
 * @return return type is int
 **/
int addShip (int intCoFour, uint8_t battlefield[MAP_SIZE][MAP_SIZE]); 

/**
 * @brief returns -1 if the intCoTwo is unvalid, otherwise 0
 * @details 
 * @param the ccoordinates must be given in intCoTwo form
 * @return return type is int
 **/
int testIntCoTwo (int intCoTwo); 

/**
 * @brief returns the number from the square in the battlefield
 * @details 
 * @param the battlefield and the coordinate in intCoTwo form must be given
 * @return return type is uint8_t
 **/
uint8_t getSquareIntFromBattlefield(uint8_t battlefield[MAP_SIZE][MAP_SIZE], int intCoTwo); 

/**
 * @brief returns 1 if the game is won, otherwise 0
 * @details 
 * @param the armada must be given (battleship array)
 * @return return type is int
 **/
int indexOfShipInArmadaFromIntCoTwo(struct battleship armada[SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4], int intCoTwo); 

/**
 * @brief reduces the health of the given battleship by pointer by one
 * @details 
 * @param the battleship must be given
 * @return return type is int
 **/
void reduceHealthOfShip (struct battleship *ship); 

/**
 * @brief returns the health of the given battleship
 * @details 
 * @param one battleship must be given
 * @return return type is int
 **/
int getHealthOfShip (struct battleship ship); 

/**
 * @brief returns 1 if the game is won, otherwise 0
 * @details 
 * @param the armada must be given (battleship array)
 * @return return type is int
 **/
int gameWon (struct battleship armada[SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4]); 

/**
 * @brief returns -1 if armada is invalid, otherwise 0
 * @details controls the total number and the length of each ship
 * @param the armada must be given (battleship array)
 * @return return type is int
 **/
int controlArmada(struct battleship armada[SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4]);

/**
 * @brief prints the battlefield to stdout
 * @details ships are SQUARE_SHIP, ... look in the header file for the #define lines
 * @param the battlefield must be given
 * @return return type is void
 **/
void prntBattlefield (uint8_t array[MAP_SIZE][MAP_SIZE]); 

/**
 * @brief returns the min of an int array
 * @details 
 * @param an array and its length must be given
 * @return return type is int
 **/
int getLowestIntFromArr(int arr[], int length);

/**
 * @brief returns the max of an int array
 * @details 
 * @param an array and its length must be given
 * @return return type is int
 **/
int getHighestIntFromArr(int arr[], int length);

#endif //COMMON_H
