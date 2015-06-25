#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N_LEN 30001
#define M_LEN 500001
#define DEBUG_ENABLE 0
int adjList[N_LEN];
int isVisit[N_LEN];
int nodes[M_LEN*2];
int next[M_LEN*2];
int n=0;
int nodeIndex=0;


/** for bfs **/
int queue[M_LEN];   /** for duplicate nodes in bfs version2 **/
int queueIndex = 0;
int bfsIndex =0;


void addEdge(int sV, int eV){
    int index= adjList[sV];

    if (index <0){
        adjList[sV]=nodeIndex;
        nodes[nodeIndex]=eV;
        nodeIndex++;
    }else{
        /**  Fatal bug, debugging for long long time
        index = next[index];
        while(index >=0){
             index =next[index];
        }**/

        while ( next[index] >=0){
            index = next[index];
        }

        next[index]=nodeIndex;
        nodes[nodeIndex]=eV;
        nodeIndex++;
    }
}

int dfsVisit(int v){
    #if DEBUG_ENABLE
       printf("%d ->",v);
    #endif // DEBUG_ENABLE

    isVisit[v] = 1;
    int nodeCount =1; /** debugging not 0 is 1 **/

    int index = adjList[v];

    while(index >=0){
        int node =nodes[index];

        if (isVisit[node] ==0 ){
            nodeCount = nodeCount + dfsVisit(node);
        }

        index = next[index];
    }

    return nodeCount;
}

int DFS(){
    int i=0;
    int maxTreeSize = -1;
    int treeSize=0;
    for(i=1;i<=n;i++){
        if (isVisit[i] != 0) continue;
        treeSize = dfsVisit(i);
        if (treeSize > maxTreeSize){
            maxTreeSize = treeSize;
        }
         #if DEBUG_ENABLE
        printf("\n");
        #endif
    }
    return  maxTreeSize;
}

int main()
{
    #if !defined(ONLINE_JUDGE)
       freopen("input.txt","r",stdin);
       ///freopen("10608.in","r",stdin);

       freopen("output.txt","w",stdout);
    #endif

    int testCase=0;

    /** while( scanf("%d",&testCase) !=EOF && testCase-- ){  debugging **/

    scanf("%d",&testCase);
    while( testCase--){

         memset(isVisit,0,sizeof(isVisit));
         memset(adjList,-1,sizeof(adjList));
         memset(nodes,-1,sizeof(nodes));
         memset(next,-1,sizeof(next));

         int m=0;
         int sV=0;
         int eV=0;
         scanf("%d %d",&n,&m);
         /** while(scanf("%d%d",&n,&m) !=EOF  && m--  ){  debugging **/

          while(m--){   /** debugging **/
             scanf("%d %d",&sV,&eV);
             addEdge(sV,eV);
             addEdge(eV,sV);
          }


         int size = DFS();
         printf("%d\n",size);

         #if DEBUG_ENABLE

            memset(queue,-1,sizeof(queue));
            queueIndex =0;
            bfsIndex =0;
            printf("BFS version1=============================\n");

            BFS_version1(n);
            printf("\n");

            memset(queue,-1,sizeof(queue));
            queueIndex =0;
            bfsIndex =0;
            printf("BFS version2=============================\n");
            BFS_version2(n);
            printf("\n");
          #endif
    }



    return 0;
}

void BFS_version1(int nodeNum){
    int i;
    memset(isVisit,0,sizeof(isVisit));
    #if DEBUG_ENABLE
       printf("BFS :\n");
    #endif // DEBUG_ENABLE

    for (i=1;i<=nodeNum;i++){
       if (isVisit[i] == 1) continue;
       queue[queueIndex++]=i;

       while(bfsIndex<queueIndex){
           bfsVisit_version1(queue[bfsIndex++]);
       }
       #if DEBUG_ENABLE
          printf("\n");
       #endif // DEBUG_ENABLE
    }

}

void bfsVisit_version1(int visitNode){
     #if DEBUG_ENABLE
     printf("%d ->",visitNode);
     #endif // DEBUG_ENABLE

    isVisit[visitNode]=1;
    int visitNodeIndex = adjList[visitNode];
    int nodeID =-1;
    while (visitNodeIndex >=0){
        nodeID= nodes[visitNodeIndex];
        if (isVisit[nodeID] != 1){
           queue[queueIndex++]=nodeID;
            isVisit[nodeID]=1; /** key **/
        }
        visitNodeIndex = next[visitNodeIndex];
    }
}


void BFS_version2(int nodeNum){
    int i;
    memset(isVisit,0,sizeof(isVisit));
    #if DEBUG_ENABLE
       printf("BFS :\n");
    #endif // DEBUG_ENABLE
    for (i=1;i<=nodeNum;i++){
       if (isVisit[i] == 1) continue;
       queue[queueIndex++]=i;

       while(bfsIndex< queueIndex){
           int node =  queue[bfsIndex];
           if (isVisit[node]  ==0){
                bfsVisit_version2(queue[bfsIndex]);
           }
           bfsIndex++;  /** debugging key!! **/
       }
      #if DEBUG_ENABLE
          printf("\n");
       #endif // DEBUG_ENABLE
    }

}

void bfsVisit_version2(int visitNode){
     #if DEBUG_ENABLE
        printf("%d ->",visitNode);
     #endif // DEBUG_ENABLE

    isVisit[visitNode]=1;
    int visitNodeIndex = adjList[visitNode];
    int nodeID =-1;
    while (visitNodeIndex >=0){
        nodeID= nodes[visitNodeIndex];
        if (isVisit[nodeID] != 1){
           queue[queueIndex++]=nodeID;
            /** isVisit[nodeID]=1;  debugging **/
        }
        visitNodeIndex = next[visitNodeIndex];
    }
}





