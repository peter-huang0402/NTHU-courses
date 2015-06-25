#include <stdio.h>
#include <stdlib.h>

int ans[14];

int findJoseph(int m, int k){
    int next[26];
    int pre=0,now=0;
    int i=0;
    int personLeft = 0;

    for (i=0;i<= (k-1);i++){
        next[i]=i+1;
    }

    next[(k-1)]=0; now = 0; pre=(k-1); personLeft = k;

    int targetP=0;
    int step=0;
    int j=0;

    for (i=0;i<=((k/2)-1) ;i++){ 
    	
        targetP = (m -1) % personLeft;   /** speed up, otherwise, I should go form 1 to m, to know which 
                                             location I will arrival. **/
        step=0 ; /** debugging forgetting init it **/


        for (j=now; step<= (targetP-1); pre=j, j=next[j] ){
            step++;
        }

        next[pre] = next[j];  /** debugging next[pre] = j; **/
        now = next[j];    /** debugging   now = j;   **/
        personLeft--;     /** debugging  **/

        if( j <= ((k/2)-1) ){
            return  -1;
        }
    }

     return 1;

}




int findJosephStart1(int m, int k){
    int next[27];
    int pre=0,now=0;
    int i=0;
    int personLeft = 0;

    for (i=1;i<=k;i++){
        next[i]=i+1;
    }

    next[k]=1; now = 1; pre=k; personLeft = k;

    int targetP=0;
    int step=1;
    int j=0;

    for (i=1;i<=(k/2);i++){
        targetP = m % personLeft;
        step=1 ; /** debugging forgetting init it **/

        if (targetP ==0) {
                targetP = personLeft;  /** debugging targetP = k;  **/
        }

        for (j=now; step<= (targetP-1); pre=j, j=next[j] ){
            step++;
        }

        next[pre] = next[j];  /** debugging next[pre] = j; **/
        now = next[j];    /** debugging   now = j;   **/
        personLeft--;     /** debugging  **/

        if( j <= (k/2) ){
            return  -1;
        }
    }

     return 1;

}




int main()
{

    #if !defined(ONLINE_JUDGE)
      freopen("305.in","r",stdin);
      freopen("output.txt","w",stdout);
    #endif

    int k=0;
    int m=0;
    int i=0;

    for(i=1 /** debugging i=2 **/;i<=13;i++){
        k= i*2;
        m=i+1;

        while (1){
             /** m++;  debugging **/
             /**
             if (m==3 && (k==4 || k ==5)) {
                int a=1;
             }
             **/
             if ( findJoseph(m,k) >0 ) {  /** debugging for  if ( findJoseph(m,k) ){} **/
                break;
            }
            m++;
        }
        ans[i] =m;
    }


    int n=0;
    while (scanf("%d",&n) != EOF && n ){
        printf("%d\n",ans[n]);
    }


    return 0;
}
