/**
 * @file common.c
 * @author Hannes Brantner Matrikelnummer: 01614466
 * @date 26.10.2017
 * @brief main source file which defines all the methods the client and server are using
 * @details the declarations of the methods are done in common.h, methodes are documented in the header file
 **/

#include "common.h"

int abs(int i) {
    return i < 0 ? -i : i;
}

void usageError (const char *progName, const char *usage) {
    fprintf(stderr, "useage: %s %s\n", progName, usage);
    fflush(stderr);
    exit(EXIT_FAILURE);
}

void error (const char *progName, struct addrinfo *ai) {
    fprintf(stderr, "[%s] ERROR: %s\n", progName, strerror(errno));
    fflush(stderr);
    freeaddrinfo(ai);
    exit(EXIT_FAILURE);
}

void gaiError (const char *progName, struct addrinfo *ai, int i) {
    fprintf(stderr, "[%s] ERROR: %s\n", progName, gai_strerror(i));
    fflush(stderr);
    freeaddrinfo(ai);
    exit(EXIT_FAILURE);
}

void sError (const char *progName, const char *msg, char *info, int returnValue) {
    fprintf(stderr, "[%s] ERROR: %s %s\n", progName, msg, info);
    fflush(stderr);
    exit(returnValue);
}

int getMsg (int sockfd, uint8_t *storage) {
    if (recv(sockfd, storage, sizeof(*storage), 0) == -1) return -1;
    return 0;
}

int sendMsg (uint8_t msg, int sockfd) {
    if (send(sockfd, &msg, sizeof(msg), 0) == -1) return -1;
    return 0;
}

int isEvenParity (uint8_t x) {
    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
    return (~x) & 1;
}

int shipHorOrVer(int intCoFour) {
    if (intCoFour/(MAP_SIZE*MAP_SIZE*MAP_SIZE) == intCoFour%(MAP_SIZE*MAP_SIZE)/MAP_SIZE) {return 0;}
    else {return 1;}
}

int shipLengthStr(const char strCoFour[4]) {
    return abs(strCoFour[0]-strCoFour[2]+strCoFour[1]-strCoFour[3])+1;
}

int shipLengthFromIntCoFour(int intCoFour) {
    int i = abs(intCoFour%(MAP_SIZE*MAP_SIZE) - intCoFour/(MAP_SIZE*MAP_SIZE));
    if ( i < MAP_SIZE) {return i+1;} else {
        return i/MAP_SIZE+1;
    }
}

void initialiseShipToArmada(struct battleship armada[SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4], int index, int intCoFour) {
    int startIntCoTwo = intCoFour/(MAP_SIZE*MAP_SIZE);
    int endIntCoTwo = intCoFour%(MAP_SIZE*MAP_SIZE);
    int length = shipLengthFromIntCoFour(intCoFour);
    armada[index].length = length;
    armada[index].health = length;
    for (int i = startIntCoTwo, k = 0; i <= endIntCoTwo; k++) {
        armada[index].intCoTwo[k] = i;
        if (shipHorOrVer(intCoFour)) {i+=MAP_SIZE;} else {i++;}
    }
}

int testStrCoFour(char strCoFour[4]) {
    if (strlen(strCoFour) != 4) return 1;
    if (strCoFour[0] - 'A' < 0 || strCoFour[0] - 'A' > MAP_SIZE-1) return 2;
    if (strCoFour[1] - '0' < 0 || strCoFour[1] - '0' > MAP_SIZE-1) return 2;
    if (strCoFour[2] - 'A' < 0 || strCoFour[2] - 'A' > MAP_SIZE-1) return 2;
    if (strCoFour[3] - '0' < 0 || strCoFour[3] - '0' > MAP_SIZE-1) return 2;
    if ((strCoFour[0] - strCoFour[2] != 0) && (strCoFour[1] - strCoFour[3] != 0)) return 3;
    if (shipLengthStr(strCoFour) < MIN_SHIP_LEN || shipLengthStr(strCoFour) > MAX_SHIP_LEN) return 4;
    return 0;
}

int strCoFourToIntCoFour(char strCoFour[4]) {
    int intCoTwoA = strCoFour[0] - 'A' + MAP_SIZE*(strCoFour[1]-'0');
    int intCoTwoB = strCoFour[2] - 'A' + MAP_SIZE*(strCoFour[3]-'0');
    if (intCoTwoA <= intCoTwoB) {return intCoTwoA*MAP_SIZE*MAP_SIZE + intCoTwoB;} 
    else {return intCoTwoB * MAP_SIZE * MAP_SIZE + intCoTwoA;}
}

int setShipPartToBattlefield (uint8_t battlefield[MAP_SIZE][MAP_SIZE], int intCoTwo) {
    int x = intCoTwo%MAP_SIZE;
    int y = intCoTwo/MAP_SIZE;
    if (battlefield[x][y] == SQUARE_SEA) {battlefield[x][y] = SQUARE_SHIP; return 0;} 
    else {return -1;} 
}

int setHitShipToBattlefield (uint8_t battlefield[MAP_SIZE][MAP_SIZE], int intCoTwo) {
    int x = intCoTwo%MAP_SIZE;
    int y = intCoTwo/MAP_SIZE;
    if (battlefield[x][y] == SQUARE_SHIP) {battlefield[x][y] = SQUARE_HIT_SHIP; return 0;} 
    else {return -1;} 
}

int setDistanceTokenToBattlefield (uint8_t battlefield[MAP_SIZE][MAP_SIZE], int intCoTwo) {
    int x = intCoTwo%MAP_SIZE;
    int y = intCoTwo/MAP_SIZE;
    if (battlefield[x][y] == SQUARE_SEA || battlefield[x][y] == SQUARE_DISTANCE) {battlefield[x][y] = SQUARE_DISTANCE; return 0;} 
    else {return -1;} 
}

void shootAtBattlefield(uint8_t battlefield[MAP_SIZE][MAP_SIZE], int intCoTwo){
    int x = intCoTwo%MAP_SIZE;
    if (x < 0 || x > MAP_SIZE-1) return;
    int y = intCoTwo/MAP_SIZE;
    if (y < 0 || y > MAP_SIZE-1) return;
    if (battlefield[x][y] == SQUARE_SHIP) {battlefield[x][y] = SQUARE_HIT_SHIP;} 
}

int addShipVertically(uint8_t battlefield[MAP_SIZE][MAP_SIZE], int startIntCoTwo, int endIntCoTwo) {
    if (startIntCoTwo%MAP_SIZE != 0 && startIntCoTwo/MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, startIntCoTwo - MAP_SIZE - 1) == -1) return -1;
    if (startIntCoTwo/MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, startIntCoTwo - MAP_SIZE) == -1) return -1;
    if (startIntCoTwo/MAP_SIZE != 0 && startIntCoTwo%MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, startIntCoTwo - MAP_SIZE + 1) == -1) return -1;
    if (startIntCoTwo%MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, startIntCoTwo - 1) == -1) return -1;
    if (startIntCoTwo%MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, startIntCoTwo + 1) == -1) return -1;
    if (setShipPartToBattlefield(battlefield, startIntCoTwo) == -1) return -1;
    for (int i = startIntCoTwo + MAP_SIZE; i < endIntCoTwo; i += MAP_SIZE) {
         if (i%MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, i - 1) == -1) return -1;
         if (i%MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, i + 1) == -1) return -1;
         if (setShipPartToBattlefield(battlefield, i) == -1) return -1;
    }
    if (endIntCoTwo%MAP_SIZE != MAP_SIZE-1 && endIntCoTwo/MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, endIntCoTwo + MAP_SIZE + 1) == -1) return -1;
    if (endIntCoTwo/MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, endIntCoTwo + MAP_SIZE) == -1) return -1;
    if (endIntCoTwo/MAP_SIZE != MAP_SIZE-1 && endIntCoTwo%MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, endIntCoTwo + MAP_SIZE - 1) == -1) return -1;
    if (endIntCoTwo%MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, endIntCoTwo + 1) == -1) return -1;
    if (endIntCoTwo%MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, endIntCoTwo - 1) == -1) return -1;
    if (setShipPartToBattlefield(battlefield, endIntCoTwo) == -1) return -1;
    return 0;
}

int addShipHorizontally(uint8_t battlefield[MAP_SIZE][MAP_SIZE], int startIntCoTwo, int endIntCoTwo) {
    if (startIntCoTwo/MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, startIntCoTwo - MAP_SIZE) == -1) return -1;
    if (startIntCoTwo/MAP_SIZE != 0 && startIntCoTwo%MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, startIntCoTwo - MAP_SIZE - 1) == -1) return -1;
    if (startIntCoTwo%MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, startIntCoTwo - 1) == -1) return -1;
    if (startIntCoTwo/MAP_SIZE != MAP_SIZE-1 && startIntCoTwo%MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, startIntCoTwo + MAP_SIZE - 1) == -1) return -1;
    if (startIntCoTwo/MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, startIntCoTwo + MAP_SIZE) == -1) return -1;
    if (setShipPartToBattlefield(battlefield, startIntCoTwo) == -1) return -1;
    for (int i = startIntCoTwo + 1; i < endIntCoTwo; i++) {
        if (i/MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, i - MAP_SIZE) == -1) return -1;
        if (i/MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, i + MAP_SIZE) == -1) return -1;
        if (setShipPartToBattlefield(battlefield, i) == -1) return -1;
    }
    if (endIntCoTwo/MAP_SIZE != 0 && setDistanceTokenToBattlefield(battlefield, endIntCoTwo - MAP_SIZE) == -1) return -1;
    if (endIntCoTwo/MAP_SIZE != 0 && endIntCoTwo%MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, endIntCoTwo - MAP_SIZE + 1) == -1) return -1;
    if (endIntCoTwo%MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, endIntCoTwo + 1) == -1) return -1;
    if (endIntCoTwo/MAP_SIZE != MAP_SIZE-1 && endIntCoTwo%MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, endIntCoTwo + MAP_SIZE + 1) == -1) return -1;
    if (endIntCoTwo/MAP_SIZE != MAP_SIZE-1 && setDistanceTokenToBattlefield(battlefield, endIntCoTwo + MAP_SIZE) == -1) return -1;
    if (setShipPartToBattlefield(battlefield, endIntCoTwo) == -1) return -1;
    return 0;
}

int addShip (int intCoFour, uint8_t battlefield[MAP_SIZE][MAP_SIZE]) {
    int startIntCoTwo = intCoFour/(MAP_SIZE*MAP_SIZE);
    int endIntCoTwo = intCoFour%(MAP_SIZE*MAP_SIZE);
    if (shipHorOrVer(intCoFour)) {return addShipVertically(battlefield, startIntCoTwo, endIntCoTwo);} 
    else {return addShipHorizontally(battlefield, startIntCoTwo, endIntCoTwo);}
}

int testIntCoTwo (int intCoTwo) {
    int x = intCoTwo%MAP_SIZE;
    int y = intCoTwo/MAP_SIZE;
    if (x < 0 || x > MAP_SIZE-1) return -1;
    if (y < 0 || y > MAP_SIZE-1) return -1;
    return 0;
}

uint8_t getSquareIntFromBattlefield(uint8_t battlefield[MAP_SIZE][MAP_SIZE], int intCoTwo) {
    return battlefield[intCoTwo%MAP_SIZE][intCoTwo/MAP_SIZE];
}

int indexOfShipInArmadaFromIntCoTwo(struct battleship armada[SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4], int intCoTwo) {
    for (int i = 0; i < SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4; i++) {
        for (int k = 0; armada[i].intCoTwo[k]  != 0; k++) {
            if (armada[i].intCoTwo[k] == intCoTwo) return i;
        }
    }
    return -1;
}

void reduceHealthOfShip (struct battleship *ship) {
    ship->health--;
}

int getHealthOfShip (struct battleship ship) {
    return ship.health;
}

int gameWon (struct battleship armada[SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4]) {
    for (int i = 0; i < SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4; i++) {
        if (getHealthOfShip(armada[i]) != 0) return 0;
    }
    return 1;
}

int controlArmada(struct battleship armada[SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4]) {
    int controlArray[3];
    memset(&controlArray, 0 , sizeof(controlArray));
    for (int i = 0; i < SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4; i++) {
        controlArray[armada[i].length-2]++;
    }
    if (controlArray[0] != SHIP_CNT_LEN2) return -1;
    if (controlArray[1] != SHIP_CNT_LEN3) return -1;
    if (controlArray[2] != SHIP_CNT_LEN4) return -1;
    return 0;
}

void prntBattlefield (uint8_t array[MAP_SIZE][MAP_SIZE]) {
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int k = 0; k < MAP_SIZE; k++) {
            printf("%i", array[k][i]);
        }
        printf("\n");
    }
    printf("\n");
}

int getLowestIntFromArr(int arr[], int length) {
    int currentLowest = INT_MAX;
    for (int i = 0; i < length; i++) {
        if (arr[i] < currentLowest) currentLowest = arr[i];
    }
    return currentLowest;
}

int getHighestIntFromArr(int arr[], int length) {
    int currentHighest = INT_MIN;
    for (int i = 0; i < length; i++) {
        if (arr[i] > currentHighest) currentHighest = arr[i];
    }
    return currentHighest;
}