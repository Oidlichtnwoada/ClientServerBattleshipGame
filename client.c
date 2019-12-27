/**
 * @file client.c
 * @author Hannes Brantner Matrikelnummer: 01614466
 * @date 26.10.2017
 * @brief main program module to act like a client for an online battleship play against a server, USAGE: $ ./cleint [-h HOSTNAME] [-p PORT]
 * @details the port and hostname can be changed via options, this program has complete error handling and exits with EXIT_SUCCESS, if everthing went fine, otherwise it exits with EXIT_FAILURE for usage or getaddrinfo error, the exit code 2 stands for parity error and the exit code 3 for invalid coordinate error
 **/

#include "common.h"

int main(int argc, char *argv[])
{
    static const char *progName = "./client";
    static const char *usage = "[-h HOSTNAME] [-p PORT]";

    char *host = DEFAULT_HOST;
    char *port = DEFAULT_PORT;

    srand(time(NULL));

    int opt_h = 0;
    int opt_p = 0;
    
    //checking options -h HOSTNAME -p PORT
    int c;
    while ( (c = getopt(argc, argv, "h:p:")) != -1 ){
        switch ( c ) {
            case 'h': 
                opt_h++; 
                //setting host
                host = optarg;
                break;
            case 'p':
                opt_p++;
                //setting port
                port = optarg;
                break;
            case '?': 
                usageError(progName, usage);
                break;
        }
    }
    
    //only one port and one host must be specified, no additional arguments
    if ( opt_h > 1 ) usageError(progName, usage);
    if ( opt_p > 1 ) usageError(progName, usage);
    if (argv[optind] != NULL) usageError(progName, usage);

    //setting up client socket
    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int res; 
    if ((res = getaddrinfo(host, port, &hints, &ai)) !=0 ) gaiError(progName, ai, res);
    
    int sockfd; 
    if ((sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0) error(progName, ai);
 
    int val = 1;
    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val)) == -1) error(progName, ai);
    
    if ((connect(sockfd, ai->ai_addr, ai->ai_addrlen)) < 0) error(progName, ai);

    //setting up the known battlefield of the server to lower the number of rounds
    uint8_t battlefield[MAP_SIZE][MAP_SIZE];
    memset(&battlefield, 0, sizeof(battlefield));

    //these variables are the brain of the client, each one is needed
    int hits[MAX_SHIP_LEN]; //for adding the destroyed ships to the battlefield
    int indexHits = 0;
    int hitANewShip = 0; //to know if last shot was a hit
    int hitTheSameShip = 0; //to know if i have hit the ship a second time
    int thisTry = -1; //the square the client is going to shoot now
    int firstHit = -1; //the first hit square of hte ship
    int lastHit = -1; //the last hit the client has landed
    int direction[4] = {-1, MAP_SIZE, 1, -MAP_SIZE}; //an array to change the last hit in the 4 needed directions 
    int indexDirection = 0;
    
    while (1) {
        //if no ship is attacked at the moment fire randomly away, but only the squares which arent known by the client
        if (hitANewShip == 0) {
            do {thisTry = rand()%(MAP_SIZE*MAP_SIZE);}
            while (getSquareIntFromBattlefield(battlefield, thisTry) != 0);
        }
        else {
            if (hitTheSameShip == 0) {
                //if the ship was hit the first time, try hitting it one more time in the four possible directions, but only if the square is unknown and the coordinate valid
                do {thisTry = lastHit + direction[(++indexDirection)%4];}
                while (testIntCoTwo(lastHit+direction[indexDirection%4]) == -1 || getSquareIntFromBattlefield(battlefield, lastHit+direction[indexDirection%4]) != 0);
            } else {
                //if the ship was hit the second time, we know how the ship is alligned, try fire the ship one more square in the same direction as above, but only if the square is unknown and the coordinate valid
                if (testIntCoTwo(lastHit+direction[indexDirection%4]) != -1 && getSquareIntFromBattlefield(battlefield, lastHit+direction[indexDirection%4]) == 0) {
                    thisTry = lastHit+direction[indexDirection%4];} 
                //if the same direction leads to a known square or to a unvalid coordinate, try hitting the next square to the first hit in the opposite direction, this field has to be valid
                else {indexDirection+=2;
                    thisTry = firstHit+direction[indexDirection%4];
                }  
            }
        }
        
        //preparing the question with the parity bit, if one needed, then sending it
        uint8_t question = thisTry;
        if (isEvenParity(question) == 0) {question += 128;} 
        sendMsg(question,sockfd);

        //the answer from the server gives as information, we can feed our data structure with them
        uint8_t answer;
        getMsg(sockfd, &answer);
        int hit = answer%4; //the last two bits of the message
        switch (hit) {
            case 0: setDistanceTokenToBattlefield(battlefield, thisTry); //shot hitting water, setting a distance token to the battlefield, so the client knows there is nothing
                    break;
            case 1: hits[indexHits++] = thisTry; //shot hitting a ship, but hasnt shunk it, save the coordinates, updating some variables
                    if (hitANewShip == 1) { if (hitTheSameShip == 0) {hitTheSameShip = 1; lastHit = thisTry;} else {lastHit=thisTry;}} 
                    else {hitANewShip = 1; firstHit = thisTry; lastHit = thisTry;}
                    break;
            case 2: hits[indexHits++] = thisTry; //shot has sunk the ship, updating all the variables to their start value
                    addShip(getLowestIntFromArr(hits, indexHits)*100+getHighestIntFromArr(hits, indexHits), battlefield);
                    memset(&hits, 0, sizeof(hits));
                    indexHits = 0;
                    hitANewShip = 0;
                    hitTheSameShip = 0;
                    firstHit = -1;
                    lastHit = -1;
                    indexDirection = 0;
                    break;
            case 3: break; //client has won, no more action needed
            default: break;
        } 

        // getting the third and fourth bit from right, switching throug the status to know if the client has to terminate
        int status = (answer%16)>>2;
        switch (status) {
            case 0: break; //game runs
            case 1: if (hit==3) {freeaddrinfo(ai); printf("[%s] I win :)\n", progName); exit(EXIT_SUCCESS);} //client has won
                    else {freeaddrinfo(ai); printf("[%s] game lost\n", progName); exit(EXIT_SUCCESS);} //MAX_ROUNDS are over, game lost
            case 2: freeaddrinfo(ai); sError(progName, "parity", "error\n", status); //parity error
            case 3: freeaddrinfo(ai); sError(progName, "invalid", "coordinate\n", status); //unvalid coordinates
            default: break;
        }
    } 
}

