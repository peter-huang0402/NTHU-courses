#include <stdio.h>
#include <stdlib.h>
#include <string.h>
# define N 5
# define HASH_SIZE 7713
/** # define MAX 10000 **/

int dx[]={2,2,1,-1,-2,-2,-1,1};
int dy[]={1,-1,-2,-2,-1,1,2,2};
int final_Status[N][N]={{1,1,1,1,1},{0,1,1,1,1},{0,0,2,1,1},{0,0,0,0,1},{0,0,0,0,0}};

typedef struct StateStructre{
    int x;
    int y;
    int status[N][N];
    int step;
} State;


State f_state[HASH_SIZE];
int f_head[HASH_SIZE];
int f_next[HASH_SIZE];

int hash(State state){
    int i=0;
    int j=0;
    int k=1;
    int value =0;

    for (i=0;i<N;i++){
        for (j=0;j<N;j++){
            if ( state.status[i][j] >0 ){
                value = value + (( 1<<k )*state.status[i][j] );
            }
            k++;
        }
    }

    int hashIdx = value %HASH_SIZE;
    return hashIdx;
}

int try_insert_f_state(int f_idx){

    int h = hash(f_state[f_idx]);
    int u  = f_head[h];


    while(u != -1){

        if ( memcmp(f_state[u].status, f_state[f_idx].status, sizeof(f_state[f_idx].status)) ==0  /** debugging, ==0 take long long time **/  ){
            return -1;
        }
        u = f_next[u];
    }

    f_next[f_idx] = f_head[h];
    f_head[h] =f_idx;

    return 1;
}

int debugging_i=0;
void debugging(State state, int count){
     debugging_i++;
     if ( debugging_i % 4 ==0){
          printf("\n");
     }
     printf("(x=%d,y=%d,step=%d,count=%d),",state.x, state.y ,state.step, count);
}


void bfsFromFinal(){
    memset(f_head,-1,sizeof(f_head) );
    memset(f_next,-1,sizeof(f_next) );

    /** debugging , forgetting initialize f_state **/
    f_state[0].x=2;
    f_state[0].y=2;
    f_state[0].step=0;
    memcpy( f_state[0].status , final_Status, sizeof(final_Status)  );
    /** debugging , forgetting initialize f_state **/

    int front =0;
    int rear =1;
    State tmp;
    State newState;
    int i=0;

    int count =0;
    while(front < rear){
        tmp = f_state[front];
        if (tmp.step >4 ){
                return -1;
        }

        for (i=0;i<8;i++){
            int newX = tmp.x + dx[i];
            int newY = tmp.y + dy[i];

            if (newX <0 || newX>=5 || newY <0 || newY>=5) continue;
            /** debugging, take long long time, because tmp should not be updated!!
                tmp will be used on 0~8 **/
            /**
            int value = tmp.status[newX][newY];
            tmp.status[newX][newY]=2;
            tmp.status[tmp.x][tmp.y]=value;
            tmp.x= newX;
            tmp.y= newY;
            **/
            newState = tmp;
            newState.status[newX][newY]=tmp.status[tmp.x][tmp.y];
            newState.status[tmp.x][tmp.y]=tmp.status[newX][newY];
            newState.x= newX;
            newState.y= newY;


            f_state[rear]=newState;
            count++;


            if ( try_insert_f_state(rear) >=0 ){
                /** debugging(f_state[rear] ,count ); **/
                f_state[rear].step++;
                rear++;
            }
        }
        front++;
    }
   return -1;
}


State s_state[HASH_SIZE];
int s_head[HASH_SIZE];
int s_next[HASH_SIZE];


int find_f_state(int s_state_idx ){
    int h = hash(s_state[s_state_idx]);
    int u = f_head[h];

    while( u!= -1){
        if (memcmp(f_state[u].status, s_state[s_state_idx].status, sizeof(s_state[s_state_idx].status)  )==0  /** debugging, ==0 take long long time **/  ){
            return u;  /** debugging , not return 1, return idx **/
        }
        u = f_next[u];
    }

    return -1;
}



int try_insert_s_state(int s_idx){

    int h = hash(s_state[s_idx]);
    int u  = s_head[h];


    while(u != -1){
        if (memcmp(s_state[u].status, s_state[s_idx].status, sizeof(s_state[s_idx].status)  )==0  /** debugging, ==0 take long long time **/  ){
            return -1;
        }
        u = s_next[u];
    }

    s_next[s_idx] = s_head[h];
    s_head[h] =s_idx;

    return 1;
}

int bfsFromStart(){
    memset(s_head,-1,sizeof(s_head) );
    memset(s_next,-1,sizeof(s_next) );
    int front =0;
    int rear =1;
    State tmp;
    State newState;
    int i=0;
    int count =0;

    while(front < rear){
        tmp = s_state[front];
        if (tmp.step >5) return -1;
        int f_idx = find_f_state(front);

        if ( f_idx >=0 ){
            int step = tmp.step+f_state[f_idx].step;
            return step;
        }

        for (i=0;i<8;i++){
            int newX = tmp.x + dx[i];
            int newY = tmp.y + dy[i];

            if (newX <0 || newX>=5 || newY <0 || newY>=5) continue;

            /** debugging, take long long time, because tmp should not be updated!!
                tmp will be used on 0~8 **/
            /**
            int value = tmp.status[newX][newY];
            tmp.status[newX][newY]=2;
            tmp.status[tmp.x][tmp.y]=value;
            tmp.x= newX;
            tmp.y= newY;
            **/
            newState = tmp;
            newState.status[newX][newY]=tmp.status[tmp.x][tmp.y];
            newState.status[tmp.x][tmp.y]=tmp.status[newX][newY];
            newState.x= newX;
            newState.y= newY;



            s_state[rear]=newState;
            count++;

            if ( try_insert_s_state(rear) >=0 ){
                /** debugging(s_state[rear] ,count ); **/
                s_state[rear].step++;
                rear++;
            }
        }
        front++;
    }
   return -1;
}





int main()
{
    #if !defined(ONLINE_JUDGE)
         ///freopen("input.txt","r",stdin);
        freopen("10422_2.in","r",stdin);
        freopen("output.txt","w",stdout);
    #endif
    int testcase=0;

    scanf("%d",&testcase);
    getchar(); /** debugging **/
    bfsFromFinal();
    int i=0;
    int j=0;
    while(testcase--){
        memset(s_state,-1,sizeof(s_state) );
        char a;
        for(i=0;i<N;i++){
            for(j=0;j<N;j++){
                scanf("%c",&a);
                if (a == ' ' ){
                    s_state[0].x=i;
                    s_state[0].y=j;
                    s_state[0].step=0;
                    s_state[0].status[i][j]=2;
                }else{
                   s_state[0].status[i][j]= a -'0';
                }
            }
            getchar(); /** debugging **/
        }

        if ( memcmp(s_state[0].status , final_Status, sizeof(final_Status)) ==0 ){
            printf("Solvable in 0 move(s).\n");
            continue;
        }

        int step = bfsFromStart();

        if (step <0 || step > 10){
            printf("Unsolvable in less than 11 move(s).\n");
        }else{
            printf("Solvable in %d move(s).\n",step);
        }

    }

    return 0;
}
