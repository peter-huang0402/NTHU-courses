#include <stdio.h>
#include <stdlib.h>
#define MAX 0x7fffffff
#define LEN 101

int village;
int castle ;
int road;
int maxDistance;
int times;
int map[LEN][LEN];

int jumping_Visit[LEN][LEN];
int  jumping_Distance[LEN][LEN];

int f[15][LEN]; 
int visit[15][LEN];


int testCase =1;

main()
{
    # if !defined(ONLINE_JUDGE)
       ///freopen("input.txt","r",stdin);
       freopen("10269.in","r",stdin);
       freopen("output.txt","w",stdout);
    # endif

    int num;
    int t;
    int i,j;
    int s,e,path;
    int m;
    scanf(" %d",&num);

    for(t=0;t<num ;t++){
    
        scanf(" %d %d %d %d %d",&village,&castle,&road,&maxDistance,&times);

        for(i=1;i<=(village+castle);i++){
            for(j=1;j<=(village+castle);j++){
                map[i][j] = MAX;                
            }
        }

        for(m=0;m<road;m++){
            scanf(" %d %d %d",&s,&e,&path);
            map[s][e] = path;
            map[e][s] = path;           
        }


        
        findAllJumpingEdge_By_Dijkstra();    
            
        findShortestPath_By_Dijkstra();
        
        printf("%d\n",f[times][(village+castle)]);
        testCase++;
    }
}



void findAllJumpingEdge_By_Dijkstra(){
    int i=1; int j=0;
    for(i=0;i<LEN;i++){
        for(j=0;j<LEN;j++){
            jumping_Distance[i][j]=MAX; 
            jumping_Visit[i][j]=0;
        }
    }

    for (i=1 ;i<=(village+castle);i++){ 
        findJumpingEdgeByDijkstra_FromVillageToCastle_Or_FromVilageToVillage(i);
    }
}

void findJumpingEdgeByDijkstra_FromVillageToCastle_Or_FromVilageToVillage(int s){

     int i=1; int j=0;
     int distance = MAX;
     int vertex=-1;
     
     jumping_Distance[s][s]= 0; 


     for (i=1;i<=(village+castle);i++ ){

         distance = MAX;
         for(j=1;j<=(village+castle);j++ ){
             if (jumping_Distance[s][j] == MAX) continue; 
             if (jumping_Visit[s][j]) continue;

             if ( s != j ){                
                if (j > village){                      
                      continue;
                }
              }

             if (distance > jumping_Distance[s][j] ){  
                   distance = jumping_Distance[s][j]; 
                   vertex = j;
             }
         }

         if (vertex ==-1 ) continue;

         jumping_Visit[s][vertex]=1;

         for(j=1;j<=(village+castle);j++){
              if (map[vertex][j] == MAX) continue;
              if (jumping_Visit[s][j]) continue;
              if ( jumping_Distance[s][j] > jumping_Distance[s][vertex]+map[vertex][j] ){
                   jumping_Distance[s][j] = jumping_Distance[s][vertex]+map[vertex][j];                   
              }
          }

          vertex = -1;
     }

}


void findShortestPath_By_Dijkstra()
{
    int i=0;
    int j=0;
    int k=0;
    int path=0;
    int vertex=0;

    for(i=0;i<=times;i++){
        for(j=0;j<=(village+castle);j++){  
            f[i][j] = MAX;
            visit[i][j]=0;
        }
    }

    for(k=0;k<=times;k++){
        f[k][1]=0;
        visit[k][1]=1; 

        for(i=1;i<=(village+castle);i++){  
            
            if (visit[k][i]==0 && map[1][i] !=MAX ){
                if ( f[k][i]> map[1][i] ){
                    f[k][i] =map[1][i]; 
                }
            }

            if(jumping_Distance[1][i]<=maxDistance){  
                f[k+1][i] = 0;
            }
        }

        for(i=1;i<=(village+castle);i++){
            path = MAX;

            for(j=1;j<=(village+castle);j++){

                if(visit[k][j]==1) continue;
                if ( path > f[k][j]){
                    path = f[k][j];
                    vertex = j;
                }
            }

            visit[k][vertex]=1;

            for(j=1;j<=(village+castle);j++){

                if(visit[k][j]==0 && map[vertex][j]!=MAX){
                    if (f[k][j] > (f[k][vertex]+map[vertex][j])  ){
                        f[k][j] = f[k][vertex]+map[vertex][j];
                        int a=1;
                    }
                }

                if(jumping_Distance[vertex][j]<= maxDistance){
                    if (f[k+1][j] > f[k][vertex] ){
                        f[k+1][j]  =  f[k][vertex];
                        int a=1;
                    }
                }
            }
        }
    }
}

