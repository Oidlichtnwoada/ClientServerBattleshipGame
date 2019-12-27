/**
 * @file server.c
 * @author Hannes Brantner Matrikelnummer: 01614466
 * @date 26.10.2017
 * @brief main program module to act like a server for an online battleship play against a client, USAGE: $ ./server [-p PORT] SHIP1 ...
 * @details you can set the preferred port for the server, this program has complete error handling and exits with EXIT_SUCCESS, if everthing went fine, otherwise it exits with EXIT_FAILURE for usage or getaddrinfo error, the exit code 2 stands for parity error and the exit code 3 for invalid coordinate error
 **/

 #include "common.h"

int main(int argc, char **argv)
{
    static const char *progName = "./server";
    static const char *usage = "[-p PORT] SHIP1 ...";

    char *port = DEFAULT_PORT;
    
    int opt_p = 0;

    //checking option -p PORT
    int c;
    while ( (c = getopt(argc, argv, "p:")) != -1 ){
        switch ( c ){
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

    //only one port must be specified, rigt amount of ships must be given
    if ( opt_p > 1 ) usageError(progName, usage);
    if (argc-optind != SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4) sError(progName, "not right total amount of", "ships", EXIT_FAILURE);

    //initialising blank battlefield
    uint8_t battlefield[MAP_SIZE][MAP_SIZE];
    memset(&battlefield, 0, sizeof(battlefield));
    
    //initialising blank armada
    struct battleship armada[SHIP_CNT_LEN2+SHIP_CNT_LEN3+SHIP_CNT_LEN4];
    memset(&armada, 0, sizeof(armada));
    int armadaIndex = 0;

    //checking every argument for validity and then the ship is added to the battlefield and armada
    for (int i = optind; i < argc; i++) {
        c = testStrCoFour(argv[i]);
        switch (c) {
            //if everthing went fine 0 is returned by testStrCoFour
            case 0: 
                break;
            case 1:
                sError(progName, "wrong syntax for ship coordinates:", argv[i], EXIT_FAILURE); break;
            case 2: 
                sError(progName, "coordinates outside of map:", argv[i], EXIT_FAILURE); break;
            case 3:
                sError(progName, "ships must be alligned either horizontally or vertically:", argv[i], EXIT_FAILURE); break;
            case 4: 
                sError(progName, "ships must have a specified length:", argv[i], EXIT_FAILURE); break;
            default: 
                break;
        }
        //concerting the argument to a four digit standard int number (y1)(x1)(y2)(x2) which is used in the entire project, the lower coordinate is the first one
        int intCoFour = strCoFourToIntCoFour(argv[i]);
        //filling the armada with ships
        initialiseShipToArmada(armada, armadaIndex, intCoFour);
        armadaIndex++;
        //adding ships to the battlefield, distance tokens are set to the map to check if there is enough space between the ships
        if (addShip(intCoFour, battlefield) == -1) sError(progName, "ships must not touch each other:", argv[i], EXIT_FAILURE);
        
    }
    //controlling if the right amount of ships with the specified lengths are in the armada
    if (controlArmada(armada) == -1) sError(progName, "not right amount of ships with the specified", "lenghts", EXIT_FAILURE);
    
    //setting up the server socket 
    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    int res;
    if ((res = getaddrinfo(NULL, port, &hints, &ai)) != 0) gaiError(progName, ai, res);
    
    int sockfd;
    if ((sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) error(progName, ai);
 
    int val = 1;
    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val)) == -1) error(progName, ai);

    if ((bind(sockfd, ai->ai_addr, ai->ai_addrlen)) == -1) error(progName, ai);
    
    if ((listen(sockfd, 1)) == -1) error(progName, ai);

    int connfd;
    if ((connfd = accept(sockfd, NULL, NULL)) == -1) error(progName, ai);

    //counting the rounds and get message from client
    for (int rounds = 0; rounds < MAX_ROUNDS; rounds++) {
        uint8_t intCoTwo;
        getMsg(connfd, &intCoTwo);
        int status = 0;
        int hit = 0;
        //checking for parity error
        if (isEvenParity(intCoTwo) == 0) {status = 2;} 
        else {
            if (intCoTwo>>7==1) intCoTwo-=128; 
            //checking for valid coordinates
            if (testIntCoTwo(intCoTwo) == -1) {status = 3;} 
            else {
                //ckecking what was in the square the client shot
                int square = getSquareIntFromBattlefield(battlefield, intCoTwo);
                //if it was a ship
                if (square == SQUARE_SHIP) {
                    //updating the battlefield with the destroyed ship part and reducing the health of the ship in the armada
                    setHitShipToBattlefield(battlefield, intCoTwo);
                    int shipIndex = indexOfShipInArmadaFromIntCoTwo(armada, intCoTwo);
                    reduceHealthOfShip(&armada[shipIndex]);
                    //if the current ship was sunk, check if there are any more ships
                    if (getHealthOfShip(armada[shipIndex]) == 0) {
                        //if not, client has won, otherwise tell him the ship he shot has sunk
                        if (gameWon(armada) == 1) {hit=3; status=1;} 
                        else {hit=2;}
                    // or tell him the ship he shot is not sunk yet
                    } else {hit=1;}           
                }                        
            }
        }

        //telling the client its the last round, if no other error or game win occur before
        if (status == 0 && rounds == MAX_ROUNDS-1) {status=1;}

        //creating the message via bitshifting and sending it
        uint8_t msg = (status<<2) + hit;
        sendMsg(msg,connfd);

        //switch-operation to check if the server has to terminate in its current status
        switch (status) {
            case 0: break;
            case 1: if (hit==3) {freeaddrinfo(ai); printf("[%s] client wins in %i rounds\n", progName, rounds); fflush(stdin); exit(EXIT_SUCCESS);}
                    else {freeaddrinfo(ai); printf("[%s] game lost\n", progName); fflush(stdin); exit(EXIT_SUCCESS);}
            case 2: freeaddrinfo(ai); sError(progName, "parity", "error\n", status);
            case 3: freeaddrinfo(ai); sError(progName, "invalid", "coordinate\n", status);
            default: break;
        }  
    }
}