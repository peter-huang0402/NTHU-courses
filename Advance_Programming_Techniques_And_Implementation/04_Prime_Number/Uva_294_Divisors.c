#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_LENGTH 40001   /***  40000 *40000 =  1,600,000,000 > 1,000,000,000 **/

int isPrime[MAX_LENGTH];

int main()
{
    #if !defined(ONLINE_JUDGE)
      freopen("294.in","r",stdin);
      freopen("output.txt","w",stdout);
    #endif

    int i=0;
    int j=0;

    /** memset(isPrime,1,sizeof(isPrime) ); forgetting setting by byte***/
     /** debugging before **/
     for (i=0;i<MAX_LENGTH /** debugging before <=**/;i++){
         isPrime[i]=1;
     }
     isPrime[0]=0,isPrime[1]=0;


    for (i=2;i<MAX_LENGTH/** debugging before <=**/;i++){
        if (!isPrime[i]) continue;
        for (j=i+i;j<=MAX_LENGTH;j+=i ){
            isPrime[j]=0;
        }
    }



    int n =0;
    int lower,upper=0;
    int sqrtNumber=0;
    int maxNum= -1;
    int divisors = -1;

    scanf("%d",&n);

    while (n--){
        scanf("%d %d",&lower,&upper);
        maxNum= -1;   /** debugging before, so stupid **/
        divisors = -1;  /** debugging before, so stupid **/

        for ( i=lower ;i<=upper; i++ ){

            sqrtNumber =  (int) sqrt( i+0.0001 );


            int tempDivisor =1;
            int x= i;
            int numX =0;
            for (j=2;j<=sqrtNumber;j++){
                if ( isPrime[j] && ( x % j ==0) /**debugging before **/ ){
                    numX =0;

                    do{
                   	 numX++;
                   	 x = x/j;
                    }while( (x%j) ==0);
                    tempDivisor = tempDivisor * (numX+1);
                    sqrtNumber = (int) sqrt( x+0.0001 );
                }
            }

            if (x >1){
                tempDivisor = tempDivisor *2;
            }

            if (divisors<tempDivisor){
                divisors=tempDivisor;
                maxNum=i;
            }
        }

        printf("Between %d and %d, %d has a maximum of %d divisors.\n",lower,upper,maxNum,divisors);

    }



    return 0;
}
