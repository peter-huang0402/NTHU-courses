#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STREET_N 2000
#define JUNCTION_N  50
#define IS_DEBUG 0


typedef struct EdgeStructure{
    int sNode;
    int eNode;
    int edge;
} Edge;

Edge adjList_Edges[2 * STREET_N];  /** 因為edge 有編號，且走時，如有多條round trip可以走完
                                    , 要以編號小的edge先走 **/
int edgeN =0;

int deg[JUNCTION_N];
int nodes[JUNCTION_N];

int isVisit[STREET_N];

int ans[STREET_N];
int ansN=0;

int firstNode =0;

void addEdge(int sNode,int eNode, int edge){
    adjList_Edges[edgeN].sNode = sNode;
    adjList_Edges[edgeN].eNode= eNode;
    adjList_Edges[edgeN].edge = edge;

    edgeN++;
}


int compareEdge(const void * a, const void * b){
    Edge aE = *(Edge *)a;
    Edge bE = *(Edge *)b;

    if (aE.sNode == bE.sNode){
        return aE.edge - bE.edge;
    }else{
       return aE.sNode - bE.sNode;
    }

}


int isConnectted(){
  int i=0;
  int value=0;
  for (i=1;i<JUNCTION_N;i++ ){
      if (deg[i] <=0 ) continue;
      value = deg[i] % 2;
      if (value >0 ){
        return -1;
      }
  }
  return 1;

}


void findNodeIndexInEdges(){
    memset( nodes , -1 , sizeof(nodes) );
    int i=0;
    int sNode=0;


    for (i=0;i<edgeN;i++ ){
        sNode= adjList_Edges[i].sNode ;
        if ( nodes[sNode] >=0  ) continue;
        nodes[sNode] =i;
    }
}


int solve(){

    qsort( &adjList_Edges[0] , edgeN, sizeof(Edge) ,  &compareEdge );

    findNodeIndexInEdges();

    if (isConnectted() <0 ) return -1;;

    ansN =0;
    dfs(firstNode);

    # if IS_DEBUG
       printf("\n");
    # endif // IS_DEBUG


    return 1;
}

void dfs(int node){
   # if IS_DEBUG
      printf("%d->",node);
   # endif // IS_DEBUG

   int nodeSize = deg[node];
   int i=0;

   int indexInEdges = nodes[node];
   int edge=0;
   int endNode=0;
   int startNode =0;

   for (i=0;i<nodeSize;i++){

       Edge e = adjList_Edges[indexInEdges+i];
       edge = e.edge;

       if ( isVisit[edge] ==1  ) continue;

       isVisit[edge] =1;
       endNode = e.eNode;
       dfs(endNode);

       ans[ansN++] = edge;

   }

}


int main()
{

     #if !defined(ONLINE_JUDGE)

       ///freopen("error_input.txt","r",stdin);
       freopen("302.in","r",stdin);
       freopen("output.txt","w",stdout);
    #endif


    int i=0;

    int sNode;
    int eNode;
    int edge;

    while(1){

        i=0;
        sNode =0;
        eNode =0;
        edge =0;



        edgeN =0;
        firstNode = -1 ;
        memset( deg , 0 , sizeof(deg) );
        memset( isVisit , 0 , sizeof(isVisit) );
        memset( ans , 0 , sizeof(ans) );
        memset( adjList_Edges , 0 , sizeof(Edge) );

        while( scanf("%d%d",&sNode,&eNode) ==2 && sNode >0 && eNode >0 ){
            if (firstNode <0){
                if (sNode < eNode){
                    firstNode = sNode;
                }else{
                    firstNode = eNode;
                }
            }
            scanf("%d",&edge);
            addEdge( sNode,eNode,edge);
            addEdge( eNode,sNode,edge);
            deg[sNode]++;
            deg[eNode]++;

             i++;
        }

        if (i==0){ /** for the end case **/
            break;
        }

        if ( solve() ==1 ){

            for ( i = ansN - 1; i >= 0; i--) {
                if (i ==0){
                    printf("%d",ans[i]);
                }else{
                    printf("%d ",ans[i]);
                }
            }
            printf("\n\n");
        }else{
            printf("Round trip does not exist.\n\n");
        }

    }

    return 0;
}
