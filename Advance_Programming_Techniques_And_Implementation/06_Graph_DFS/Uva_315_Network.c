#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LENGTH 201

int adjMatrix[LENGTH][LENGTH];
int color[LENGTH];  /** 0-> white(unvisited) , 1-> gray(visiting), 2 -> black(having been visited) ; **/
int parent[LENGTH];
int sTime[LENGTH];
int endTime[LENGTH];
int n;
int time;

int DFS(int skipNode){
    int i=1;
    for (;i<=n;i++){
        color[i]=0;
        parent[i]=0;
        sTime[i]=0;
        endTime[i]=0;
    }

    time =0;
    color[skipNode] = 2;
    int treeSize =0;


    for(i=1 /**debugging not 0 is 1 **/;i<=n;i++){
        if ( color[i] != 0) continue;
        treeSize++;
        dfsVisit(i);
    }
    return treeSize;
}

void dfsVisit(int v){
    color[v]=1;
    time++;
    sTime[v]=time;

    int i=0;
    for( i=1;i<=n;i++){
        if (adjMatrix[v][i] !=1 ) continue;
        if ( color[i] != 0 ) continue;
        parent[i] =v;
        dfsVisit(i);
    }

    color[v] =2;
    time++;
    endTime[v]= time;
}



void DFS_version2(int startNode, int skipNode){
   int i=1;
   if (color[startNode] !=0 ){
       return ;
   }else{
        color[skipNode] = 2;
        color[startNode] =2;
        for(i=1;i<=n;i++){
            if (adjMatrix[startNode][i] ==1 && color[i]==0  ){
                DFS_version2(i, skipNode);
            }
        }
   }
}

int isArticulatePoint(){
    int i=1;
    for (i=1;i<=n;i++ ){
        if ( color[i] == 0 ) return 1;
    }
    return -1;
}

int main()
{
    #if !defined(ONLINE_JUDGE)
       ///freopen("input.txt","r",stdin);
       freopen("315.in","r",stdin);
       freopen("output.txt","w",stdout);
    #endif


    char c="\0";
    int startV=0;
    int endV=0;

    time =0;
    int i=0;

    int articulatedPointCount=0;



    while( scanf("%d",&n) ==1 && n ){

            memset(adjMatrix,-1, sizeof(adjMatrix));
            memset(color,0,sizeof(color));
            memset(parent,0,sizeof(parent));
            memset(sTime,0, sizeof(sTime) );
            memset(endTime,0, sizeof(endTime) );
            articulatedPointCount =0;
            time  =0;
            startV=0;
            endV=0;
            c="\0";

            while( scanf("%d%c" , &startV,&c /**debugging scanf("%d",&startV) **/ ) ==2 && startV  ){
                  if (c == '\n') continue; /** debugging not break; because some start node without ending with end node **/
                  while(  /** debugging not %d%c **/ scanf("%d%c",&endV,&c) ==2 && endV  ){
                      adjMatrix[startV][endV] =1;
                      adjMatrix[endV][startV] =1; /** debugging forget **/
                      if (c == '\n' /** debugging not "\n" but '\n' **/ ) break;
                  }
            }

            /** version 1 **/

            for(i=1;i<=n;i++){
                int treeSize = DFS(i);
                if (treeSize >1){
                    articulatedPointCount++;
                }
            }


            /** version 2**/
            /**
            for (i=1;i<=n;i++){
                memset(color,0,sizeof(color));
                if (i==1){
                    DFS_version2(2,1);
                }else{
                    DFS_version2(1,i);
                }

                if (isArticulatePoint() >0){
                    articulatedPointCount++;
                }
            }
            **/
            printf("%d\n",articulatedPointCount);

    }


    return 0;
}
