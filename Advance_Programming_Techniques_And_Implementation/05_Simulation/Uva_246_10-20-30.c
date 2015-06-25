#include <stdio.h>
#include <stdlib.h>

typedef struct StateStruct{
  int handCard[52];
  int handCardLength ;
  int cards[7][52];
  int cardsLength[7];
  int whichTurn;
  int round;
} State;

State currentState;
State twoOfPowerState;
State initState;

State getNewState(){
    State state;

    state.handCardLength =0;
    state.whichTurn =0;
    state.round=0;
    memset(state.cardsLength,0, sizeof(state.cardsLength)  );
    memset(state.cards,0, sizeof(state.cards)  );
    memset(state.handCard,0, sizeof(state.handCard)  );

    return state;
}

void clearState(State *state){
    (*state).handCardLength =0;
    (*state).whichTurn =0;
    (*state).round=0;
    memset((*state).cardsLength,0, sizeof((*state).cardsLength)  );
    memset((*state).cards,0, sizeof((*state).cards)  );
    memset((*state).handCard,0, sizeof((*state).handCard)  );
}

void copyState(State *toState, State *fromState){
    int len =(*fromState).handCardLength;
    int i=0;
    for (i=0;i<len;i++){
        (*toState).handCard[i]=  (*fromState).handCard[i];
    }
    (*toState).handCardLength = len;

    i=0;
    int j=0;
    for (i=0;i<7;i++){
        len = (*fromState).cardsLength[i];
        for (j=0;j<len;j++){
            (*toState).cards[i][j] = (*fromState).cards[i][j];
        }
        (*toState).cardsLength[i] = (*fromState).cardsLength[i];
    }

    (*toState).whichTurn = (*fromState).whichTurn;
    (*toState).round = (*fromState).round;
}

void getWhichTurn(State *state){
    int i=1;
    int turn=0;

    for (;i<=7;i++){
        /** (*state).whichTurn++;     /// avoiding adding it ,before making sure the availble turn
            turn = ( (*state).whichTurn -1) % 7;
        **/

        turn = ( (*state).whichTurn + i) % 7;  /** debugging **/

        if ( (*state).cardsLength[turn] >0 ){
            (*state).whichTurn =turn;  /** debugging **/
            break;
        }
    }
}

int is2OfPower(int num){
    int i=1;
    for(i;i<=num;i=i*2){
        if (i == num) return 1;
    }
    return -1;
}

void increase3SpaceForHandCard( State *state ){
    int len = (*state).handCardLength;
    (*state).handCardLength = len +3;

    int i=0;

    for (i=(len-1);i>=0;i--){
        (*state).handCard[i+3] =  (*state).handCard[i];
    }

    (*state).handCard[0]=0;
    (*state).handCard[1]=0;
    (*state).handCard[2]=0;
}

void darwCardsFromPack(State *state){
     int whichTurn = (*state).whichTurn;
     int f2Last1Cards=0;
     int f1Last2Card=0;
     int Last3Card=0;
     int i=0;
     int len=0;
     while(  (*state).cardsLength[whichTurn] >2 ){
            len = (*state).cardsLength[whichTurn];

            f2Last1Cards = (*state).cards[whichTurn][0] + (*state).cards[whichTurn][1] +  (*state).cards[whichTurn][len-1];
            f1Last2Card  = (*state).cards[whichTurn][0] + (*state).cards[whichTurn][len-1] +  (*state).cards[whichTurn][len-2];
            Last3Card = (*state).cards[whichTurn][len-1] +  (*state).cards[whichTurn][len-2] + (*state).cards[whichTurn][len-3];

            if ( f2Last1Cards % 10 == 0){
                 increase3SpaceForHandCard(state);
                 (*state).handCard[0] =  (*state).cards[whichTurn][len-1];
                 (*state).handCard[1] =  (*state).cards[whichTurn][1];
                 (*state).handCard[2] =  (*state).cards[whichTurn][0];
                 /** debugging forgetting clear cards **/
                 for (i=2;i<(len-1);i++){
                    (*state).cards[whichTurn][i-2] =(*state).cards[whichTurn][i];
                 }
                 /** debugging forgetting clear cards **/
                 (*state).cardsLength[whichTurn] = len-3;
            }else if ( f1Last2Card % 10 == 0 ){
                 increase3SpaceForHandCard(state);
                 (*state).handCard[0] =  (*state).cards[whichTurn][len-1 /** debugging -2 **/];
                 (*state).handCard[1] =  (*state).cards[whichTurn][len-2 /** debugging -1 **/];
                 (*state).handCard[2] =  (*state).cards[whichTurn][0];
                 /** debugging forgetting clear cards **/
                 for (i=1;i<(len-2);i++){
                    (*state).cards[whichTurn][i-1] =(*state).cards[whichTurn][i];
                 }
                 /** debugging forgetting clear cards **/
                 (*state).cardsLength[whichTurn] = len-3;
            }else if (Last3Card %10 ==0 ){
                 increase3SpaceForHandCard(state);
                 (*state).handCard[0] =  (*state).cards[whichTurn][len-1 /** debugging -3 **/];
                 (*state).handCard[1] =  (*state).cards[whichTurn][len-2 /** debugging -2 **/];
                 (*state).handCard[2] =  (*state).cards[whichTurn][len-3 /** debugging -1 **/];
                 (*state).cardsLength[whichTurn] = len-3;
            }else{
                break;
            }
     }

     outputForDebug(state);
}

void dispatchOneCard(State *state){

    /**  getWhichTurn(state);  debugging, it should be put after dispatchOneCard action
                               has been done.   **/

    int turn = (*state).whichTurn;

    int len = (*state).handCardLength;

    int card =(*state).handCard[len-1];
    (*state).handCardLength = len-1;

    (*state).cardsLength[turn]++;

    len = (*state).cardsLength[turn];

    (*state).cards[turn][len-1] = card;

    /**  debugging, not get next turn when dispatch card,
         separate it with dispatchOneCard action.
         call getWhichTurn() , when each round has been done,
         which is more easy to control whichturn number.

    getWhichTurn(state);

    **/
}



int isDraw(State *state1,State *state2){
    if ( (*state1).handCardLength !=  (*state2).handCardLength ){
        return -1;
    }

    int i=0,j=0;
    for (;i<7;i++){
       if ((*state1).cardsLength[i] !=  (*state2).cardsLength[i] ){
            return -1;
       }
    }

    int len =(*state2).handCardLength;
    i=0;
    j=0;
    for (;i<len;i++){
        if ((*state1).handCard[i]  !=  (*state2).handCard[i] ){
            return -1;
        }
    }

    i=0;
    j=0;
    for (;i<7;i++){
        len =(*state2).cardsLength[i];
        for(j=0;j<len;j++){
            if ((*state1).cards[i][j]  !=  (*state2).cards[i][j]  ){
                return -1;
            }
        }
    }

    return 1;
}

int findDrawLocation(){
    int cycle =currentState.round - twoOfPowerState.round;
    State state1;
    State state2;
    copyState(&state1, &initState);
    copyState(&state2, &initState);

    int cycleCount = cycle;
    while(cycleCount-- /** debugging , cycle--**/ ){
        dispatchOneCard( &state2);
        darwCardsFromPack( &state2);
        state2.round++;
        getWhichTurn( &state2);
    }

    int i=15;  /** int i=14;  debugging, because round is start by 1**/

    for ( ;i<=currentState.round;i++  ){

        dispatchOneCard( &state1);
        darwCardsFromPack( &state1);

        dispatchOneCard( &state2);
        darwCardsFromPack( &state2);

        if (isDraw(&state1 , &state2) ==1 ){
            return state1.round + cycle; /**debugging  return state1.round; **/
        }

        state1.round++;
        state2.round++;
        getWhichTurn( &state1);
        getWhichTurn( &state2);
    }


    return -1;
}


void outputForDebug(State *state) {
    if (1) return;

    printf("----------------------------\n");
    int i=0,j=0;


    printf("round: %d\n",(*state).round);
    printf("handLength: %d \n",(*state).handCardLength);
    printf("hand card: ");


    for ( i=0;i<( (*state).handCardLength) ;i++) {
        printf("%d ",(*state).handCard[i]);
        if ( i%20==0 ) printf("\n");
    }
    printf("\n");

    printf("which turn: %d\n", (*state).whichTurn );

    for ( i=0;i<7;i++) {
        if( (*state).cardsLength[i] >0 ) {
            printf("card[%d]: ", i);
            for ( j=0;j< (*state).cardsLength[i] ;j++){
                printf("%d ", (*state).cards[i][j]);
            }
            printf("\n");
        }
    }
}

int main(){

    #if !defined(ONLINE_JUDGE)
        freopen("246.in","r",stdin);
        ///freopen("error_input.txt","r",stdin);
        freopen("output.txt","w",stdout);
    #endif

    currentState = getNewState();
    initState = getNewState();
    twoOfPowerState = getNewState();

    int i=0;


    int topCard=0;
    while ( scanf("%d",&topCard) != EOF && topCard ){

        clearState(&currentState);
        clearState(&initState);
        clearState(&twoOfPowerState);

        currentState.handCard[51] = topCard;
        for(i=0;i<51;i++){
            scanf("%d",&currentState.handCard[50-i]);
        }
        currentState.handCardLength =52;

        currentState.round = 1; /** debugging for round start with 1 **/

        for (i=1;i<=14;i++){

           /** turn = getWhichTurn( &currentState);  debug for all cardsLength is zero **/



           int handCard = currentState.handCard[ (currentState.handCardLength -1) ];
           int cardLength = currentState.cardsLength[currentState.whichTurn];

           currentState.cards[currentState.whichTurn][ cardLength] =handCard;

           currentState.handCardLength--;
           currentState.cardsLength[currentState.whichTurn]++;


           if ( is2OfPower(i) >0 ){
                copyState( &twoOfPowerState , &currentState);
           }

           outputForDebug(&currentState);

           /** debugging , put on the last part after dispatching card.**/
           currentState.whichTurn= (currentState.whichTurn +1 ) % 7;
           currentState.round ++;  /** debugging put it than copyState **/


        }

        copyState(&initState, &currentState);  /** use initSate when try to find loop's location. **/


        while (1){

              /** for debugging
              if ( currentState.round == 82  ){
                 int bbb=1;
              }
              **/


               dispatchOneCard( &currentState);
               darwCardsFromPack( &currentState);


               if ( currentState.handCardLength == 52){
                   printf("Win : %d\n",  currentState.round);
                   break;
               }else if ( currentState.handCardLength ==0 ){
                   printf("Loss: %d\n",  currentState.round);
                   break;
               }else if (isDraw(&currentState, &twoOfPowerState)>0 ){
                   int roundLocation = findDrawLocation();
                   printf("Draw: %d\n", roundLocation);
                   break;
               }

               /** debugging, should be put behind isDraw()!!
                    before copying  currentState to towOfPowerState,
                    it should be done the isDraw action.
                **/
               if ( is2OfPower(currentState.round  ) >0  ){  /**debugging is2OfPower(i) **/
                    copyState( &twoOfPowerState , &currentState);
               }

               currentState.round++;
               getWhichTurn( &currentState );

        }


    }


    return 0;
}
