#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NOT_PATH 0x07ffffff

int adjMatrix[500][500];
int parent[500];
int isVisited[500];
int distance[500];

float rowMatrix[500][500];
int n=0;



int initDijkstra(){
    int i=0;
    int j=0;
    for(i=1;i<=n;i++){
        distance[i]= NOT_PATH;  /** debugging, not set to be zero but to be largest.!!**/
        for(j=1;j<=n;j++){
            adjMatrix[i][j]=NOT_PATH;
        }
    }

    memset(parent, -1, sizeof(parent) );
    memset(isVisited, 0, sizeof(isVisited) );
    /** memset(distance, 0, sizeof(distance) ); debugging for setting distance to be largest **/
}

void relax(int from, int to){
    if ( isVisited[to] == 0 && adjMatrix[from][to] !=NOT_PATH && distance[to] > distance[from] + adjMatrix[from][to] ){ /** debugging is larger than ex: distance[to] > distance[from] + adjMatrix[from][to] **/
          distance[to] = distance[from] + adjMatrix[from][to];
          parent[to]=from;
    }
}

void dijkstra(int sV){
   parent[sV]=sV;
   distance[sV] =0;

   int i=0;
   int j=0;
   int path=NOT_PATH;

   for (i=1;i<=n;i++){

      path = NOT_PATH;
      int vertex =0;

      for (j=1;j<=n;j++){
          if( isVisited[j] == 0 /** debugging forgetting isVisit[j]==0 **/&& distance[j] < path){
              path= distance[j];
              vertex = j;
          }
      }

      isVisited[vertex]=1; /** debugging not isVisited[i]=1; **/

      for (j=1;j<=n;j++){
         /** if (isVisited[j] ==0){  could be trimmed off , because I have been checked it in relax action**/
            relax(vertex,j);
         /** }  **/
      }
   }

}



int main()
{
    #if !defined(ONLINE_JUDGE)
       freopen("318.in","r",stdin);
      ///freopen("input.txt","r",stdin);
      freopen("output.txt","w",stdout);
    #endif

    int row=0;
    int i=0;
    int j=0;
    int testcase =1;

    while( scanf("%d %d",&n,&row) ==2 && n>0 /** debugging row could be zero**/  ){


         initDijkstra();
         int line = row;
         while(line--){
             int s=0;
             int e=0;
             int len=0;
             scanf("%d %d %d",&s,&e,&len);
             adjMatrix[s][e] = len;
             adjMatrix[e][s] = len;
             adjMatrix[s][s] =0;
             adjMatrix[e][e] =0;
         }

         dijkstra(1);

         int maxV=0;
         int maxV_path = 0;
         for (i=1;i<=n;i++){  /** debugging start node is 1 **/
            if (distance[i] > maxV_path ){
                maxV = i;
                maxV_path = distance[maxV];
            }
         }

         int sV =0 , eV =0;
         memset(rowMatrix,0,sizeof(0));

         float temPath=0;
         float finishTime = 0;
         for (i=1;i<=n;i++){ /** debugging start node is 1 **/
            for(j=1;j<=n;j++){ /** debugging start node is 1 **/
                if ( adjMatrix[i][j] ==0 || adjMatrix[i][j] ==NOT_PATH ) continue;  /** debugging !=zero and NOT_PATH**/


                int whichPath =0;
                if (distance[i] > distance[j] ){  /** debugging not >= but > for distinguishing startV and endV **/
                    whichPath = 0;

                    temPath =  (adjMatrix[i][j] - (distance[i] - distance[j])) / 2.0 ;  /** debugging  should path/2.0 not path/2  **/
                    rowMatrix[i][j] = distance[i] + temPath;
                }else{
                    whichPath = 1;
                    temPath =  (adjMatrix[i][j] - (distance[j] - distance[i]) ) / 2.0; /** debugging  should path/2.0 not path/2  **/
                    rowMatrix[i][j] = distance[j] + temPath;
                }

                if (finishTime < rowMatrix[i][j]  ){
                    finishTime = rowMatrix[i][j] ;
                    if (whichPath ==0){
                        sV = j;  /** debugging, once distance being larger should be put on endV. smaller one puts on startV. **/
                        eV = i;
                    }else{
                        sV = i;  /** debugging, once distance being larger should be put on endV. smaller one puts on startV. **/
                        eV = j;
                    }
                }
            }
         }

         if (n>0 && row==0 ){  /** debugging ,because node start from 1 when no path exists**/
            maxV =1;         /** debugging ,because node start from 1 when no path exists**/
         }

         printf("System #%d\n",testcase++ ) ;
         if (distance[maxV] >= finishTime ){ /** debugging not > but >= , for node & edge taking the same time, node in priority**/
             printf("The last domino falls after %d.0 seconds, at key domino %d.\n",distance[maxV],maxV);
         }else if ( distance[maxV] < finishTime ){
             printf("The last domino falls after %.1f seconds, between key dominoes %d and %d.\n", finishTime,sV,eV);
         }

         printf("\n");




    }



    return 0;
}
