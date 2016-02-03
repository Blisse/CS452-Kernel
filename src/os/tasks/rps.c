#include "rps.h"

#include <bwio/bwio.h>
#include <rtosc/assert.h>
#include <rtos.h>

#include "name_server.h"

#define RPS_SERVER_NAME "RpsServer"

typedef enum _RPS_SERVER_REQUEST_TYPE
{
    SignupRequest = 0,
    PlayRequest,
    QuitRequest
} RPS_SERVER_REQUEST_TYPE;

typedef enum _RPS_MOVE
{
    RockMove = 0,
    PaperMove,
    ScissorsMove,
    MaxMove
} RPS_MOVE;

typedef struct _RPS_SERVER_REQUEST
{
    RPS_SERVER_REQUEST_TYPE type;
    RPS_MOVE move;
} RPS_SERVER_REQUEST;

typedef enum _RPS_SERVER_RESPONSE
{
    PartnerQuitResponse = 0,
    WinResponse,
    TieResponse,
    LoseResponse
} RPS_SERVER_RESPONSE;

static
inline
RPS_MOVE
RpspClientSelectMove
    (
        VOID
    )
{
    static INT nextMove = 20;

    // "Random" number generator
    nextMove *= 4;
    nextMove -= 2;
    nextMove /= 3;
    nextMove += 5;

    return nextMove % MaxMove;
}

static
inline
VOID
RpspClientPrintMove
    (
        IN INT myTaskId,
        IN RPS_MOVE move
    )
{
    switch(move)
    {
        case RockMove:
            bwprintf(BWCOM2, "Client %d: I choose rock \r\n", myTaskId);
            break;

        case PaperMove:
            bwprintf(BWCOM2, "Client %d: I choose paper \r\n", myTaskId);
            break;

        case ScissorsMove:
            bwprintf(BWCOM2, "Client %d: I choose scissors \r\n", myTaskId);
            break;

        default:
            ASSERT(FALSE, "Client chose a bad move \r\n");
            break;
    }
}

static
inline
VOID
RpspClientPrintResult
    (
        IN INT myTaskId,
        IN RPS_SERVER_RESPONSE response
    )
{
    if(PartnerQuitResponse == response)
    {
        bwprintf(BWCOM2, "Client %d: My partner quit! \r\n", myTaskId);
    }
    else if(WinResponse == response)
    {
        bwprintf(BWCOM2, "Client %d: I won the match! \r\n", myTaskId);
    }
    else if(TieResponse == response)
    {
        bwprintf(BWCOM2, "Client %d: We tied that match \r\n", myTaskId);
    }
    else
    {
        bwprintf(BWCOM2, "Client %d: I lost the match :( \r\n", myTaskId);
    }
}

static
VOID
RpspClientTask
    (
        VOID
    )
{
    INT i;
    INT myTaskId = MyTid();
    INT rpsServerTaskId = WhoIs(RPS_SERVER_NAME);
    RPS_SERVER_REQUEST request = { SignupRequest };

    bwprintf(BWCOM2, "Client %d: I want to play %d rounds \r\n", myTaskId, myTaskId);

    // Ask to join the game
    Send(rpsServerTaskId,
         &request,
         sizeof(request),
         NULL,
         0);

    // Play the game
    request.type = PlayRequest;

    for(i = 0; i < myTaskId; i++)
    {
        RPS_SERVER_RESPONSE response;

        // Select a move
        request.move = RpspClientSelectMove();

        // Print the move
        RpspClientPrintMove(myTaskId, request.move);

        // Let the server know the move
        Send(rpsServerTaskId,
             &request,
             sizeof(request),
             &response,
             sizeof(response));

        // Check and see the result
        RpspClientPrintResult(myTaskId, response);

        // If my partner quit, I should too
        if(PartnerQuitResponse == response)
        {
            break;
        }
        else
        {
            // Wait for my partner to print result
            Send(rpsServerTaskId, NULL, 0, NULL, 0);
        }
    }

    // This client is done playing
    request.type = QuitRequest;

    Send(rpsServerTaskId,
         &request,
         sizeof(request),
         NULL,
         0);
}

static
VOID
RpsServerHandleBothPlayersQuitting
    (
        IN INT player1,
        IN INT player2
    )
{
    bwprintf(BWCOM2, "Server: Client %d is quitting \r\n", player1);
    bwprintf(BWCOM2, "Server: Client %d is quitting \r\n", player2);

    Reply(player1, NULL, 0);
    Reply(player2, NULL, 0);
}

static
VOID
RpspServerHandleOnePlayerQuitting
    (
        IN INT quittingPlayer,
        IN INT otherPlayer
    )
{
    RPS_SERVER_REQUEST otherPlayerRequest;
    RPS_SERVER_RESPONSE otherPlayerResponse = PartnerQuitResponse;

    bwprintf(BWCOM2, "Server: Client %d is quitting \r\n", quittingPlayer);

    // Acknowledge quitting player, and inform other player
    Reply(quittingPlayer, NULL, 0);
    Reply(otherPlayer, &otherPlayerResponse, sizeof(otherPlayerResponse));

    // Wait for other player to quit
    Receive(&otherPlayer, &otherPlayerRequest, sizeof(otherPlayerRequest));
    ASSERT(QuitRequest == otherPlayerRequest.type, "Other player is supposed to quit \r\n");

    bwprintf(BWCOM2, "Server: Client %d is quitting \r\n", otherPlayer);

    // Acknowledge player 2 quitting
    Reply(otherPlayer, NULL, 0);
}

static
VOID
RpspServerTask
    (
        VOID
    )
{
    INT player1;
    RPS_SERVER_REQUEST player1Request;
    INT player2;
    RPS_SERVER_REQUEST player2Request;
    RPS_SERVER_RESPONSE results[MaxMove][MaxMove];

    RegisterAs(RPS_SERVER_NAME);

    // Set up possible results
    results[RockMove][RockMove] = TieResponse;
    results[RockMove][PaperMove] = LoseResponse;
    results[RockMove][ScissorsMove] = WinResponse;
    results[PaperMove][RockMove] = WinResponse;
    results[PaperMove][PaperMove] = TieResponse;
    results[PaperMove][ScissorsMove] = LoseResponse;
    results[ScissorsMove][RockMove] = LoseResponse;
    results[ScissorsMove][PaperMove] = WinResponse;
    results[ScissorsMove][ScissorsMove] = TieResponse;

    // Wait for players
    Receive(&player1, &player1Request, sizeof(player1Request));
    ASSERT(SignupRequest == player1Request.type, "Bad player1 signup request \r\n");

    Receive(&player2, &player2Request, sizeof(player2Request));
    ASSERT(SignupRequest == player2Request.type, "Bad player2 signup request \r\n");

    // Let the players know the game is about to start
    Reply(player1, NULL, 0);
    Reply(player2, NULL, 0);

    // Start the game
    while(1)
    {
        bwprintf(BWCOM2, "Server: Starting match \r\n");

        // Wait for hands from both players
        Receive(&player1, &player1Request, sizeof(player1Request));
        Receive(&player2, &player2Request, sizeof(player2Request));

        // Figure out who won
        if(PlayRequest == player1Request.type &&
           PlayRequest == player2Request.type)
        {
            RPS_SERVER_RESPONSE player1Response = results[player1Request.move][player2Request.move];
            RPS_SERVER_RESPONSE player2Response = results[player2Request.move][player1Request.move];

            Reply(player1, &player1Response, sizeof(player1Response));
            Reply(player2, &player2Response, sizeof(player2Response));
        }
        else
        {
            break;

        }

        // Wait for clients to print out results
        Receive(&player1, NULL, 0);
        Receive(&player2, NULL, 0);
        Reply(player1, NULL, 0);
        Reply(player2, NULL, 0);

        // Wait for the TA before starting another round
        bwprintf(BWCOM2, "Server: Match over. Press any key to start next match \r\n\r\n");
        bwgetc(BWCOM2);
    }

    // Looks like someone quit
    if(QuitRequest == player1Request.type &&
       QuitRequest == player2Request.type)
    {
        RpsServerHandleBothPlayersQuitting(player1, player2);
    }
    else if(QuitRequest == player1Request.type)
    {
        RpspServerHandleOnePlayerQuitting(player1, player2);
    }
    else if(QuitRequest == player2Request.type)
    {
        RpspServerHandleOnePlayerQuitting(player2, player1);
    }
    else
    {
        ASSERT(FALSE, "How did we get here? \r\n");
    }
}

VOID
RpsInit
    (
        VOID
    )
{
    Create(PRIORITY_17, RpsServerTask);
    Create(PRIORITY_16, RpsClientTask);
    Create(PRIORITY_16, RpsClientTask);
}
