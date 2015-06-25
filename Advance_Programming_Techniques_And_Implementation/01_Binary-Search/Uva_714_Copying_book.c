#include <stdio.h>
int books;
int scribers;
long long maxPage;
long long pagesSum;
long long pageReference;


int findAvailablePagesForScriber(int *pages, long long mid){
   int i=0;
   long long currentPages=0;
   int tempScribers = 0;

   for (;i<books;i++){

       if (tempScribers>= scribers ) return -1; /** error code , debug before **/

       if ((currentPages + pages[i]) > mid){
            currentPages = pages[i];
            tempScribers++;
       }else{
            currentPages = currentPages + pages[i];
       }
   }

   /** error codes
   if ( (scribers - tempScribers) ==1) return 1;

   return -1;
   **/

   if ( tempScribers >= scribers) return -1;

   return 1;
}


long long binarySearch(int *pages){
    long long lower = maxPage;
    long long upper = pagesSum;
    long long mid =  0;

    while( upper > lower){
         mid = (lower+upper)/2;

        if ( findAvailablePagesForScriber(pages,mid ) >0 ){
            upper = mid;
        }else{
            lower = mid+1;
        }
    }
    return upper;
}


void output(int *pages, int *pageMarkIndex){
   int i=books-1;
   int unusedScribers =scribers-1; /** error code , debug before**/
   long long currentPages = 0;

   for (;i>=0;i--){
        pageMarkIndex[i]=0; /* initialize it*/


        /** error logic , because forget some situation ==>
             paging limit=4
            correct 1 / 2 / 3  1 / 3 2
            wrong   1 / 2 / 3 / 1 3 2

        if (unusedScribers > 0 && unusedScribers >= i ){ ///error code , debug before
            pageMarkIndex[i]=1;
            unusedScribers--;
        }else{

            if (currentPages+pages[i] >= pageReference ){
                if(currentPages+ pages[i] == pageReference ){
                    currentPages =0;
                    pageMarkIndex[i]=1;
                }else{
                    currentPages = pages[i];
                     pageMarkIndex[i+1]=1; /// error code , debug before
                }
                unusedScribers--;
            }else{
                currentPages = currentPages+ pages[i];
            }
        }


        if (currentPages+pages[i] >= pageReference ){
                if(currentPages+ pages[i] == pageReference ){
                    currentPages =0;
                    pageMarkIndex[i]=1;
                }else{
                    currentPages = pages[i];
                     pageMarkIndex[i+1]=1; /// error code , debug before
                }
                unusedScribers--;
        }else if (unusedScribers >0 && unusedScribers == i){
            pageMarkIndex[i]=1;
             unusedScribers--;
        }else{
            currentPages = currentPages+ pages[i];
        }**/


        currentPages = currentPages + pages[i];
		if ( currentPages > pageReference){

            /** error case , for debugging **/
            if (pageMarkIndex[i+1] && unusedScribers >0 && unusedScribers >=i){
                pageMarkIndex[i]=1;  /** key points **/
            }else{
                pageMarkIndex[i+1] =1;
            }

            unusedScribers = unusedScribers -1 ;
			currentPages = pages[i];
		}else if (currentPages == pageReference && unusedScribers >0){
		    currentPages =0;
			pageMarkIndex[i]=1;
			unusedScribers = unusedScribers -1 ;
		}else if (unusedScribers >0 && unusedScribers >=i){
		   pageMarkIndex[i]=1;
		   unusedScribers = unusedScribers -1;
		}



   }


   for (i=0;i<books ; i++){
       if (pageMarkIndex[i]){
         printf("/ ");
      }

      if (i == (books-1)){
        printf("%d", pages[i]);
      }else{
        printf("%d ", pages[i]);
      }
   }

   printf("\n");


}

int main()
{
    #if !defined(ONLINE_JUDGE)
      /** freopen("A.714_INPUT_0.txt","r",stdin); **/
      /**  freopen("error_case.txt","r",stdin); **/
      freopen("714.in","r",stdin);
      freopen("output.out","w",stdout);
    #endif
    int totalCases =0;

    scanf("%d",&totalCases);

    while( (totalCases--) >0 ){

        scanf("%d %d",&books,&scribers);

        /** error case => cannot get correct input page
        long long pages[books];
        **/

        int pages[books];

        int pageMarkIndex[books];
        int i=0;
        maxPage = 0;
        pagesSum =0;
        pageReference = 0;

        while(i < books){
            scanf("%d",&pages[i]);
            pagesSum = pagesSum + pages[i];
            if (pages[i] > maxPage){
                maxPage = pages[i];
            }
            i++;
        }

        pageReference =binarySearch(&pages[0]);
        /** printf("print case=%d, pageRefeference=%lld \n",(totalCases+1),pageReference); **/

        output(&pages[0],&pageMarkIndex[0]);


    }


    return 0;
}
