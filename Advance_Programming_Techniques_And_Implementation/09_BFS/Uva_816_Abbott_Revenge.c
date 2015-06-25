#include <stdio.h>
#include <stdlib.h>
#include <string.h>
# define LEN  100000

int dx[]={0,1,0,-1}; /** N, E,S,W **/
int dy[]={-1,0,1,0}; /** N, E,S,W **/  /** debugging **/

int getIndex(char c){
   int index=-1;
   switch(c){
        case 'N':
            index =0;
            break;
        case 'E':
            index =1;
            break;
        case 'S':
            index =2;
            break;
        case 'W':
            index =3;
            break;
   }
   return index;
}


int moveByLFR(int d,char c){
    int location=-1;
    switch(c){
        case 'L':
            location = (d+3)%4;
        break;
        case 'F':
            location =d;
        break;
        case 'R':
            location = (d+1)%4;
        break;
    }
    return location;
}

typedef struct MapStrcture{
    int d[4][4];
}Map;
Map map[10][10];

typedef struct NodeStructure{
   int x;
   int y;
   int d;
}Node;

Node nodes[LEN];
int parent[LEN];
int isVisit[10][10][4][4];

int nodeLen=0;
int parentLen=0;

void mapInfo() {
    int i=0;
    int j=0;

    char direction[4]="NESW";
    int d=0;
    int turn=0;
    for ( i=1;i<10;i++) {
        for ( j=1;j<10;j++) {
              printf("(%d %d) ", i, j);
            for ( d=0;d<4;d++) {
                printf("%turn: ", direction[d]);
                for ( turn=0;turn<4;turn++){
                    if (map[i][j].d[d][turn]==1)   printf("%turn", direction[turn]);
                }
                printf(" ");
            }
            printf("\n");
        }
    }
}

int main()
{
    #if !defined(ONLINE_JUDGE)
      freopen("816.in","r",stdin);
      ///freopen("input.txt","r",stdin);
      freopen("output.txt","w",stdout);
    #endif

    char title[20];

    while( scanf("%s",&title) !=EOF && strcmp(title,"END") !=0  ){

        memset(map,0,sizeof(map));
        memset(nodes,0,sizeof(nodes));
        memset(parent,-1,sizeof(parent));
        memset(isVisit,0,sizeof(isVisit));
        parentLen =0;
        nodeLen =0;

        int sX=0;
        int sY=0;
        char sD;
        int sDIdx;
        int eX=0;
        int eY=0;
        scanf("%d %d %c %d %d",&sY,&sX,&sD,&eY,&eX);
        sDIdx=getIndex(sD);

        int x=0;
        int y=0;
        char dInfo[10];  /** debugging dInfo[4], making x & y to be updated once the length is not enough. **/
        char way;
        while( scanf("%d %d",&y,&x) ==2 ){

             while( scanf("%s",dInfo) !=EOF ){
                  if (dInfo[0] == '*'){
                    break;
                  }
                  int len = strlen(dInfo);
                  int dIndex= getIndex(dInfo[0]);
                  int newLocation = -1;


                  if (len>1){
                      way = dInfo[1];
                      newLocation = moveByLFR(dIndex, way); /** debugging, key !! need to find available location **/

                      map[x][y].d[dIndex][newLocation]=1;
                  }
                  if (len>2){
                      way = dInfo[2];
                      newLocation = moveByLFR(dIndex, way); /** debugging, key !! need to find available location **/

                      map[x][y].d[dIndex][newLocation]=1;
                  }
                  if (len>3){
                      way = dInfo[3];
                      newLocation = moveByLFR(dIndex, way); /** debugging, key !! need to find available location **/

                      map[x][y].d[dIndex][newLocation]=1;
                  }


             }

        }

        mapInfo();

        int head=0;
        int tail=1;
        nodes[head].x=sX;
        nodes[head].y=sY;
        nodes[head].d=sDIdx;
        parent[0]=-1;
        parentLen=1;
        int d=0;

        int nextX=0;
        int nextY=0;
        int i=0;
        int isFind =0;
        while (head < tail){
             x =nodes[head].x;
             y =nodes[head].y;
             d =nodes[head].d;

             nextX =  x +dx[d]; /** debugging key!!for walking **/
             nextY =  y +dy[d];

             if (nextX == eX && nextY == eY){
                  isFind=1;
                  break;
             }

             for (i=0;i<4;i++){
                  if ( nextX>=1 && nextX<10 && nextY>=1 && nextY<10 && isVisit[nextX][nextY][d][i] ==0 && map[nextX][nextY].d[d][i] ==1 ){
                       isVisit[nextX][nextY][d][i]=1;
                       nodes[tail].x=nextX;
                       nodes[tail].y=nextY;
                       nodes[tail].d=i ; /** debugging for getting new direction**/
                       parent[parentLen++]=head;
                       tail++;
                  }
             }
             head++;
        }

        printf("%s\n",&title);
        if (isFind==0){
            printf("  No Solution Possible\n");
        }else{
            int sToEndArray[LEN];
            int len=0;
            int index = head;
            while (index >=0){
                /**  debugging !!!key !!!
                index = parent[index];
                sToEndArray[len++]=index;
                **/
                sToEndArray[len++]=index;
                index = parent[index];
            }

            int ans[len];
            for (i=len-1;i>=0;i--){
                ans[len-1-i]= sToEndArray[i];
            }

            printf("  ");
            for (i=0;i<=len-1;i++){
                index = ans[i];
                printf("(%d,%d)",nodes[index].y ,nodes[index].x);
                if ( i !=0 && (i+1)%10 == 0){  /** debugging   if ( i !=0 && i%10 == 0){ **/
                    printf("\n  ");
                }else{
                   printf(" ");
                }
            }

            printf("(%d,%d)",eY ,eX);
            printf("\n");
        }

    }

    return 0;
}
