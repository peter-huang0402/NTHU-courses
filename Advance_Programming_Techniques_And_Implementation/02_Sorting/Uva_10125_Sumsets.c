#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Set{
    int a;
    int b;
    long long int sum;
} SumSet;

int n=0;
int elements[1000];
SumSet sumSets[1000000];  /* 1000 * 1000 */

int compareInt(const void *a , const void *b ){
    int c= *(int *)  a;
    int d= *(int *)  b;
    return  (c - d);  /** check if overflow **/
}

int compareSumset(const void *a,const void *b){
     SumSet sumSetA = *(SumSet *) a;
     SumSet sumSetB = *(SumSet *) b;

    return (sumSetA.sum - sumSetB.sum);
}

int binarySearchForLowerBound(long long int findX, int sumSetsAmount){

    int lower=0;
    int upper=sumSetsAmount -1;
    int mid =0;
    while (upper > lower){  /** debug for lower_bound  > **/
        mid = (lower+upper)/2;

        if (sumSets[mid].sum >= findX){
            upper = mid;
        }else{
            lower = mid +1;
        }
    }

    if (sumSets[upper].sum == findX){
        return upper;
    }else{
        return -1;
    }

}


int testBinarySearchForLowerBound(int findX, int sumSetsAmount, int array[]){
    printf("findx=%d, length=%d \n",findX, sumSetsAmount);

    int lower=0;
    int upper=sumSetsAmount -1;
    int mid =0;
    while (upper > lower){
        mid = (lower+upper)/2;
        printf("u=%d, m=%d, l=%d\n",upper,mid,lower);
        
        if (array[mid] < findX){
            lower = mid+1;
        }else{
            upper = mid;
        }       
    }

    if (array[upper] == findX){
        return upper;
    }else{
        return -1;
    }
}

int testBinarySearchForUpperBound(int findX, int sumSetsAmount, int array[]){
    printf("findx=%d, length=%d \n",findX, sumSetsAmount);

    int lower=0;
    int upper=sumSetsAmount -1;
    int mid =0;
    while (upper > (lower+1) ){ /** key points!! for debugging before!!  not >= **/
        mid = (lower+upper)/2;
        printf("u=%d, m=%d, l=%d\n",upper,mid,lower);

        if (array[mid] > findX){
            upper = mid -1;	            
        }else{
            lower = mid;
        }
    }

    if (array[upper] == findX){ /** for debugging before **/
        return upper;
    }else if (array[lower] == findX){  /** for debugging before **/
        return lower;
    }else{
        return -1;
    }

}


int mainAAA(){
    int aa[]={1,2,2,2,2,2,2,2,2,3} ;
    int findX = 2;
    int index =testBinarySearchForLowerBound(findX,sizeof(aa)/sizeof(int),aa);
    printf("lower bound index=%d when find=%d \n",index,findX);

    index =testBinarySearchForUpperBound(findX,sizeof(aa)/sizeof(int),aa);
    printf("upper bound index=%d when find=%d \n",index,findX);
}

int main()
{
    #if !defined(ONLINE_JUDGE)
        freopen("10125.in","r",stdin);
        freopen("output.txt","w",stdout);
    #endif

    int i=0,j=0;
    int number =0;
    int sumSetsAmount = 0;

    int d=0;
    int c=0;
    long long int dSubstractC =0;
    int lowerBoundIndex = 0;
    bool findD = false;


    while(scanf("%d",&n) !=EOF && n ){

        i=0;
        while(i <n){
            scanf("%d",&number);
            elements[i]=number;
            i++;
        }

        qsort(&elements[0], n /** sizeof(elements)/sizeof(int) for debugging **/,sizeof(int),compareInt);


        sumSetsAmount=0;
        for(i=0;i<n;i++){
            /**  sumSets[sumSetsAmount].a=elements[i];   for debugging **/
            for (j=i+1; j<n;j++){
                if (i == j ) continue;
                sumSets[sumSetsAmount].a=elements[i]; /** for debugging **/
                sumSets[sumSetsAmount].b=elements[j];
                sumSets[sumSetsAmount].sum = sumSets[sumSetsAmount].a + sumSets[sumSetsAmount].b;
                sumSetsAmount++;
            }
        }
        qsort(&sumSets[0],sumSetsAmount  /** sizeof(sumSets)/sizeof(SumSet)  for debugging **/ , sizeof(SumSet),compareSumset);

        findD = false;

        for(i=n-1;i>=0 && (!findD) ;i--){
            /** for(j=0;j<=(n-1) && (!findD) ;j++ ){ **/  /** for debugging for getting wrong answer**/	
            for (j= i-1;j>=0 && (!findD) ;j--){
                if (i == j) continue;
                d = elements[i];
                c= elements[j];
                dSubstractC = d - c;
                lowerBoundIndex = binarySearchForLowerBound( dSubstractC,sumSetsAmount );
                if (lowerBoundIndex == -1) continue;

                int k = lowerBoundIndex;
                while( k<sumSetsAmount && sumSets[k].sum == dSubstractC && (!findD) ){
                  if(sumSets[k].a != c && sumSets[k].a !=d && sumSets[k].b !=c && sumSets[k].b !=d ){
                      printf("%d\n",d);
                      findD = true;
                  }
                  k++;
                }
            }
        }

        if (!findD) printf("no solution\n");

    }


    return 0;
}
