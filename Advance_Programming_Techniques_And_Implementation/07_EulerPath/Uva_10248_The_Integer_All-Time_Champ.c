#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int oneToNine[10];
char ans[500];
int outDegree[10];
int inDegree[10];
int ansLen=0;

int adjMatrix[10][10];


/**
   做dfs時，我並沒有每次進call DSF時，先找哪個node　outdegree大的node，做dfs, 
   而是只有一開始就找outdegree大的，以後的recusive dfs就不判斷node 的outdgree 哪個大，
   這作法會出錯，因為他很可能先跑的　end node，他的outdgree 比indegree 少。而沒有邊
   
   而程式會對，是因為，我是由node 小到大，補邊。所以imbance 的node (end node), 一定是
   在最後一個的node. 所以離開的點，會最後才會被走到，這就是為何，eulerian path
   , 可以正確走完的原因。       
**/
void makeEulerianPath222(){
    /**
     EulerianPath:
     make start to be outDegree - inDegree=1;
          end   to be inDegree  -  outDegree=1;
     and other nodes are the same degree in inDegree and outDegree
    **/
    int i,j=0;
    int imBalance=0;
    for(i=0;i<10;i++){
       if (inDegree[i] != outDegree[i]){
           imBalance = imBalance+ abs( inDegree[i] - outDegree[i] );
       }
    }

    i=0,j=0;  /** debugging i,j=0  ; i without assigning zero **/

    for(  ;i<10 && (imBalance >2) ;i++){

        for(/** fatal error without resetting j**/ j=0 ;j<10 && (imBalance >2) ;j++){

            while ( (inDegree[i] > outDegree[i]) && (inDegree[j] < outDegree[j]) && (imBalance >2) ){
                adjMatrix[i][j]++;
                outDegree[i]++;
                inDegree[j]++;
                imBalance = imBalance -2;  /** key for debugging for long long time**/
            }

        }
    }


}


void makeEulerianPath(){
        int in_outDegree[10];  /** 0:draw 1:in>out 2:out>in  **/
        int inLargerOut =0;
        int inLargerOutArray[10];
        int outLargerIn =0;
        int outLargerInArray[10] ;

        int imBalance =0;
        int i=0;
        for (i=0;i<10;i++){
            if (inDegree[i] == outDegree[i]){
                in_outDegree[i]=0;
            }else if (inDegree[i] > outDegree[i]){
                in_outDegree[i] = 1;
                inLargerOutArray[inLargerOut]=i;
                inLargerOut++;
                imBalance = imBalance+ (inDegree[i] -  outDegree[i]);
            }else{
                in_outDegree[i] = 2;
                outLargerInArray[outLargerIn]=i;
                outLargerIn++;
                imBalance =  imBalance +(outDegree[i] - inDegree[i]);
            }
        }

         i=0;
        int j=0;

        while (imBalance > 2){

            int in=inLargerOutArray[i];
            int out=outLargerInArray[j];

            if ( inDegree[in] == outDegree[in] ){
                i++;
                in=inLargerOutArray[i];
            }

            if ( inDegree[out] == outDegree[out] ){
                j++;
                out=outLargerInArray[j];
            }

            adjMatrix[in][out]++;
            outDegree[in]++;
            inDegree[out]++;
            imBalance = imBalance -2; /** key!! debugging when increase 1 edge
                                          imbalance will make 2 node more balance**/

        }
}

int findStartNode(){
    int i=0;
    int sNode =-1;
    for(;i<10;i++){
        if ( outDegree[i] > inDegree[i] ){
            sNode = i;
            return sNode;
        }
    }

    i=0;
    for(;i<10;i++){
        if (inDegree[i]>0 || outDegree[i]>0 ){   /** key for special case. ex: 13 -> 13 only **/
            sNode =i;
            return sNode;
        }
    }

    return -1; /** debugging forgetting return negative number , take long long time
                   debugging when 1->1, no any path!!  **/


}

void dfs(int sNode){
    int i=0;
    for (;i<10;i++){
        if ( adjMatrix[sNode][i] >0 ){
            adjMatrix[sNode][i] --;
            outDegree[sNode ]--;
            inDegree[i]--;
            dfs(i);
            ans[ansLen++] = i + '0';
        }
    }
}


int main()
{
    #if !defined(ONLINE_JUDGE)
       freopen("10248.in","r",stdin);
      ///freopen("input.txt","r",stdin);
      freopen("output.txt","w",stdout);
    #endif

    int s,e=0;

    while(scanf("%d%d",&s,&e) ==2 && s>0 && e >0 ){  /** debugging for unlimited loop scanf("%d%d",&s,&e) != EOF **/
        memset(oneToNine,0,sizeof(oneToNine) );
        memset(ans,'\0',sizeof(ans));
        memset(adjMatrix,0,sizeof(adjMatrix) );
        memset(outDegree, 0, sizeof(outDegree) );
        memset(inDegree, 0, sizeof(inDegree) );
        ansLen=0;


        int i=0;

        for(i=s; i<=e;i++){
            if (i<10){
                oneToNine[i]=1;
            }else{
               int x= i/10;
               int y= i%10;
               adjMatrix[x][y]++;
               outDegree[x]++;
               inDegree[y]++;
            }
        }

        makeEulerianPath();
        int sNode = findStartNode();

        if (sNode >=0){  /** debugging when 1->1, no any path , should not go through dfs !!! **/

            dfs(sNode);
            ans[ansLen++] = sNode + '0';

            i=ansLen-1;

            int node=0;
            for(;i>=0;i--){
                node = ans[i] -'0';
                printf("%d",node );
                oneToNine[ node ] =0; /** debugging for avoiding display the same digit from 1 to 9 **/
            }

        }

        for(i=1;i<10;i++){
            if (oneToNine[i] ==1 ){
                printf("%d",i );
            }
        }

        printf("\n");

    }



    return 0;
}
