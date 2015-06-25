#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 52
#define NO_PATH 0x07ffffffffffffff

int adjMatrix[MAX][MAX];
int parent[MAX];
long long int distance[MAX]; /** debugging forgetting is long long int more than 10^9 = 2^32 **/
int isVisited[MAX];
int nLen=0;
char words[MAX];
int wordsID[MAX];

int findWords_22(char word){
    int i=0;
    for( ;i<MAX && wordsID[i] >=0 ;i++  ){
        if (words[i] == word){
            return i;
        }
    }
    return -1;
}

/** debubbging , get wrong answer because print smallest one A-n-d < a-n-d **/
int findWordsInSortedSequence_22(char word){

    int i=0;

    for (i=0;i<nLen;i++){
        int index = wordsID[i];
        char name = words[index];
        if (name == word){
            return i;
        }

    }

    return -1;
}

void initDijkstra(){
    int i=0;
    int j=0;
    for (i=0;i<MAX;i++){
        distance[i] = NO_PATH;
        for (j=0;j<MAX;j++){
                adjMatrix[i][j]=0;
        }
    }

    memset(parent,-1,sizeof(parent) );
    memset(isVisited,0,sizeof(isVisited) );

}

long long int findTotalItem(long long int item){

    long long int  value = item /19;
    int modValue = item % 19;

    if (modValue ==0){
        return 20 * value;
    }else{
        return 20*value + modValue + 1 ;
    }

}

void relax_22(int from, int to){

    int fromAdjMatrix = wordsID[from];  /** key!!**/
    int toAdjMatrix = wordsID[to];


   if (isVisited[to] ==0 && (adjMatrix[fromAdjMatrix][toAdjMatrix] != 0) ){
        char toWord = words[to];
        long long int cost =0;

        char fromWord = words[fromAdjMatrix]; /** key!! key!! debugging , char fromWord = words[from]; **/

        if ( fromWord >= 'A' && fromWord <= 'Z' ){
             cost = findTotalItem( distance[from] );
        }else{
            cost =distance[from] +1;
        }

        if (distance[to] >  cost ){
            distance[to] =  cost;
            parent[to] = from;
        }
    }
}


void dijkstra_22(char eV, long long int item /**debugging, int item **/){
   int s = findWordsInSortedSequence_22(eV);
   distance[s] = item; /** debugging distance[s] = 0;**/
   parent[s] = -1;

   int i=0;
   int j=0;
   long long int path = NO_PATH;  /** debugging not int , but long long int**/
   int vertex =-1;
   for ( i=0;i<nLen;i++){
       path = NO_PATH;
       vertex =-1;
       for(j=0;j<nLen;j++){
            if (isVisited[j] ==1 ) continue;
            if ( distance[j] <path ){
                vertex = j;
                path = distance[j];
            }
       }

       isVisited[vertex] = 1;

       for(j=0;j<nLen;j++){
           relax_22(vertex,j);
       }
   }

}

int compareWords_22(const void * a, const void * b){
     int aIdx =  *(int *)a;
     int bIdx =  *(int *)b;
     return words[aIdx] - words[bIdx];
}


int main_22()
{
    #if !defined(ONLINE_JUDGE)
      ///freopen("10537_1.in","r",stdin);
      freopen("10537_2.in","r",stdin);
      ///freopen("input.txt","r",stdin);
      freopen("output.txt","w",stdout);
    #endif

    int row=0;
    int testCase=1;
    while( scanf("%d",&row) == 1 && row>=0 ){
        nLen=0;
        memset( &words[0], '0', sizeof(words) );
        memset( &wordsID[0],-1, sizeof(wordsID) );
        initDijkstra();

        int line = row;
        char s= '0';
        char e= '0';
        int sIdx=-1;
        int eIdx=-1;
        while(  line-- ){
            /** scanf("%c %c",&s,&e);  should put an empty space before "%c %c" debugging **/
            scanf(" %c %c",&s,&e); /** debugging **/
            sIdx = findWords_22(s);
            eIdx = findWords_22(e);

            if (sIdx <0){
                sIdx = nLen++;
                words[sIdx]=s;
                wordsID[sIdx]=sIdx;
            }
            if (eIdx <0){
                eIdx = nLen++;
                words[eIdx]=e;
                wordsID[eIdx]=eIdx;
            }

            adjMatrix[sIdx][eIdx]=1;
            adjMatrix[eIdx][sIdx]=1;
            adjMatrix[sIdx][sIdx] =1;  /** debugging not 0, but 1 **/
            adjMatrix[eIdx][eIdx] =1;  /** debugging not 0, but 1 **/
        }

        qsort(&wordsID[0],nLen, sizeof(int), compareWords_22);
        long long int item = 0;
        char sV ='0';
        char eV ='0';
        scanf("%lld %c %c",&item,&sV,&eV);


        if (testCase == 3){
            int a=1;
        }

        dijkstra_22(eV, item);

        int sVIdx = findWordsInSortedSequence_22(sV);

        if (sVIdx >=0){  /** debugging for row ==0 **/

            printf("Case %d:\n",testCase);
            printf("%lld\n", distance[sVIdx]);
            int originalOrderInWords =wordsID[sVIdx];  /** key !!! key !!! **/

            /** int eVIdx = findWordsBySortedSequence(eV); debugging sV is end point, because trace back!! **/
            printf("%c",words[originalOrderInWords]);  /**  key !!! key !!!**/
            int parentIdx = parent[sVIdx];
            while( parentIdx != -1){
                originalOrderInWords = wordsID[parentIdx];   /**  key !!! key !!!**/
                printf("-%c",words[originalOrderInWords]);   /**  key !!! key !!!**/
                parentIdx = parent[parentIdx];
            }
            printf("\n");
        }else{
            /** debugging for row ==0 **/
            printf("Case %d:\n",testCase);
            printf("%lld\n",item);
            printf("%c",sV);
            printf("\n");
        }

        testCase++;
    }

    return 0;
}

typedef struct NodeStructure{
    char name;
    int order;
} Node;

Node nodes[MAX];

int findWords(char word){
    int i=0;
    for( ;i<nLen ;i++  ){
        char name = nodes[i].name;
        if ( name == word){
            return i;
        }
    }
    return -1;
}

void relax(int form, int to){


   Node fromNode = nodes[form];  /** debugging, when there are more than one route,
                                        print lexi-smallest one. ex: A-nd is smaller than a-nd**/
   Node toNode = nodes[to];

   int fromAdjMatrix = fromNode.order;
   int toAdjMatrix = toNode.order;


    /**
    if (isVisited[to] ==0 && (adjMatrix[from][to] != NO_PATH) && distance[to] > adjMatrix[from][to] + distance[from] ){
    debugging , because unknown the path's cost.. **/
   if ( isVisited[to] ==0 && (adjMatrix[fromAdjMatrix][toAdjMatrix] != 0) ){

        long long int cost =0;
        char fromWord = fromNode.name;
        if ( fromWord >= 'A' && fromWord <= 'Z' ){
        /** if ( toWord >= 'A' && toWord <= 'Z' ){
        /** debugging fromWord not toWord, because trace back form end to start and,
         because coming to entrance we need to pay entrance fee.
                                        taking a long long time**/

           cost = findTotalItem( distance[form] ); /** cost = findTotalItem( distance[fromAdjMatrix] );
                                                      debugging key!! make me confused **/

        }else{
            cost =distance[form] +1;
        }

        if (distance[to] >  cost ){
            /** distance[to] =  adjMatrix[from][to] + distance[from];  debugging **/

            distance[to] =  cost;
            parent[to] = form;
        }
    }
}


void dijkstra(char eV, long long int item /**debugging, int item **/){
   int s = findWords(eV);
   distance[s] = item; /** debugging distance[s] = 0;**/
   parent[s] = -1;

   int i=0;
   int j=0;
   long long int path = NO_PATH;  /** debugging not int , but long long int**/
   int vertex =-1;
   for ( i=0;i<nLen;i++){
       path = NO_PATH;
       vertex =-1;
       for(j=0;j<nLen;j++){
            if (isVisited[j] ==1 ) continue;
            if ( distance[j] <path ){
                vertex = j;
                path = distance[j];
            }
       }

       isVisited[vertex] = 1;

       for(j=0;j<nLen;j++){
           /**
           int node =  nodes[j].order;
           relax(vertex,node);  **/
           relax(vertex , j);
       }
       /** int a=1; **/
   }
    /** int b=1;**/
}

int compareWords(const void * aNode, const void * bNode){
     Node a =  *(Node *)aNode;
     Node b =  *(Node *)bNode;
     char aName = a.name;
     char bName = b.name;
     return aName - bName;
}

int main()
{
    #if !defined(ONLINE_JUDGE)
      ///freopen("input.txt","r",stdin);
      ///freopen("10537_1.in","r",stdin);
      freopen("10537_2.in","r",stdin);
      ///freopen("input.txt","r",stdin);
      freopen("output.txt","w",stdout);
    #endif

    int row=0;
    int testCase=1;
    while( scanf("%d",&row) == 1 && row>=0 ){
        nLen=0;
        memset( &nodes[0] , 0 , sizeof(nodes) );

        initDijkstra();

        int line = row;
        char s= '0';
        char e= '0';
        int sIdx=-1;
        int eIdx=-1;
        while(  line-- ){
            /** scanf("%c %c",&s,&e);  should put an empty space before "%c %c" debugging **/
            scanf(" %c %c",&s,&e); /** debugging **/
            sIdx = findWords(s);
            eIdx = findWords(e);

            if (sIdx <0){
                sIdx = nLen++;
                nodes[sIdx].name = s;
                nodes[sIdx].order = sIdx;
            }

            if (eIdx <0){
                eIdx = nLen++;
                nodes[eIdx].name = e;
                nodes[eIdx].order = eIdx;
            }

            adjMatrix[sIdx][eIdx]=1;
            adjMatrix[eIdx][sIdx]=1;
            adjMatrix[sIdx][sIdx] =1;  /** debugging not 0, but 1 **/
            adjMatrix[eIdx][eIdx] =1;  /** debugging not 0, but 1 **/
        }

        qsort(&nodes[0],nLen, sizeof(Node), compareWords);  /** debugging, when there are more than one route,
                                                                print lexi-smallest one. ex: A-nd is smaller than a-nd**/
        long long int item = 0;
        char sV ='0';
        char eV ='0';
        scanf("%lld %c %c",&item,&sV,&eV);

        /**
        if (testCase == 5){
            int a=1;
        }
        **/


        dijkstra(eV, item);

        int sVIdx = findWords(sV);

        if (sVIdx >=0){  /** debugging for row ==0 **/

            printf("Case %d:\n",testCase);
            printf("%lld\n", distance[sVIdx]);
            char name = nodes[sVIdx].name;
            /** int eVIdx = findWords(eV); debugging sV is end point, because trace back!! **/
            printf("%c",name);
            int parentIdx = parent[sVIdx];
            while( parentIdx != -1){
                name =  nodes[parentIdx].name;
                printf("-%c",name);
                parentIdx = parent[parentIdx];
            }
            printf("\n");
        }else{
            /** debugging for row ==0 **/
            printf("Case %d:\n",testCase);
            printf("%lld\n",item);
            printf("%c",sV);
            printf("\n");
        }

        testCase++;
    }




    return 0;
}

