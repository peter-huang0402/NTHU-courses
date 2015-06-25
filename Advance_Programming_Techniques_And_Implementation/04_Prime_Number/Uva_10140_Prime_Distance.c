#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
# define MAX_LENGTH 50001 /** (50000 * 50000) >= 2,147,483,647 **/

char isPrime[MAX_LENGTH];
int primeList[MAX_LENGTH];
int pNum =0;

int main()
{
    #if !defined(ONLINE_JUDGE)
        freopen("10140.in","r",stdin);
        freopen("output.txt","w",stdout);
    #endif


    int i=0;
    int j=0;

    memset(isPrime,1 /**debugging before, not '1' **/,sizeof(isPrime));

    isPrime[0]=0;
    isPrime[1]=0;


    for (i=0;i<MAX_LENGTH; i++){
        if (isPrime[i] ==0) continue;
        primeList[pNum++] = i;
        for (j=i+i;j<MAX_LENGTH;j=j+i /** debugging += but not =+**/ ){
            isPrime[j /** debugging, not i**/ ] =0;
        }
    }


    int lower=0;
    int upper=0;

    int min1Number=0;
    int min2Number=0;
    int minDistance=999999;

    int max1Number=0;
    int max2Number=0;
    int maxDistance=-1;


    int previousPrime=0;


    while ( scanf("%d %d",&lower,&upper) != EOF){

        /** debugging forgetting initialize them**/
        previousPrime=0;
        min1Number=0;
        min2Number=0;
        minDistance=999999;
        max1Number=0;
        max2Number=0;
        maxDistance=-1;


        i=lower;

        if (i<2){  /**debugging before, when lower starts with 1**/
            i=2;
        }

        for ( ;i<=upper;i++){

            if (i <0) break; /** debugging for, overflow i will become negative number**/

            bool isP = true;
            if (i< MAX_LENGTH-1){
                if ( isPrime[i] ==0){
                        isP=false;
                }
            }else{
                for(j=0;j<pNum &&  (primeList[j] * primeList[j]) <=i /**debugging before, not primeList[j] * primeList[j]) < i**/ ;j++ ){
                    if (i% primeList[j] ==0 ){
                        isP =false;
                        break;
                    }
                }
            }
            if (!isP) continue;


            if (previousPrime ==0){
                previousPrime = i;
            }else{

                if ( (i - previousPrime) < minDistance){
                    min1Number = previousPrime;
                    min2Number = i;
                    minDistance = i - previousPrime;
                }

                if (  (i - previousPrime) > maxDistance ){
                     max1Number = previousPrime;
                     max2Number = i;
                     maxDistance = i - previousPrime;
                }
                previousPrime = i; /** debugging before,so stupid!! take long long time **/
            }

            /*** above code, could be generalized as following

             if ( (i - previousPrime) < minDistance){
                    min1Number = previousPrime;
                    min2Number = i;
                    minDistance = i - previousPrime;
                }

                if (  (i - previousPrime) > maxDistance ){
                     max1Number = previousPrime;
                     max2Number = i;
                     maxDistance = i - previousPrime;
                }
                previousPrime = i;

            **/


        }


        if (  maxDistance ==-1){  /*** debugging before, if distance the same, we display the same distance between min distance and max distance.
                                     update if (  (minDistance == maxDistance) || maxDistance ==-1){ **/
            printf("There are no adjacent primes.\n");
        }else{
            printf("%d,%d are closest, %d,%d are most distant.\n",min1Number, min2Number, max1Number, max2Number);
        }


    }



    return 0;
}
