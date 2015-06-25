#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROW 50
#define COLUMN 20
#define SMALLEST_TEN 10
#define SELL_PRICE  10
#define TOTAL_COLUMN  ROW*COLUMN +1

int maxProfits[ROW];
int numItems[ROW];
int rowItems=0;
int totalProfit = 0;


typedef struct ItemStrcture{
    int number;
    int buyCost;
    int profit;
}Item;

Item items[ROW][COLUMN];
int buyNumber[ROW][TOTAL_COLUMN];

int compareItem(const void *a, const void *b){
    Item c = *(Item*) a;
    Item d = *(Item*) b;

    if (c.profit == d.profit){
        return c.number - d.number;
    }else{
        return d.profit - c.profit;
    }
}

int main(){

   #if !defined(ONLINE_JUDGE)
      freopen("812.in","r",stdin);
      /** freopen("error_testcase.txt","r",stdin); **/
      freopen("output.txt","w",stdout);
   #endif

   int i_row=0;
   int i_item=0;
   int price=0;
   int numberBuy =0;
   int i=0;
   int no=1;
   while( scanf("%d",&rowItems) != EOF && rowItems ){

        if (no != 1){
          /** debugging beofore, testcase should has a blank line**/
          printf("\n");
        }

        i_row =0 ;
        while (i_row < rowItems){

            i_item =0;
            scanf("%d",&numItems[i_row]);

            maxProfits[i_row] = -100000;
            while (i_item < numItems[i_row]){
                price=0;
                scanf("%d",&price);
                items[i_row][i_item].buyCost = price;
                items[i_row][i_item].number =  i_item +1;

                if (i_item ==0){
                    items[i_row][i_item].profit =  SELL_PRICE -items[i_row][i_item].buyCost;
                }else{
                    items[i_row][i_item].profit =  items[i_row][i_item-1].profit + SELL_PRICE -items[i_row][i_item].buyCost ;
                }

                if (items[i_row][i_item].profit > maxProfits[i_row]){
                    maxProfits[i_row] = items[i_row][i_item].profit;
                }

                i_item++;
            }

            qsort( &items[i_row][0],numItems[i_row], sizeof(Item), compareItem);
            i_row++;
        }


        i_row =0;
        memset( buyNumber,0, sizeof(buyNumber));
        totalProfit =0;
        while (i_row < rowItems){

            if (maxProfits[i_row] <0){

                if (i_row == 0){ /** debugging before **/
                    buyNumber[i_row][0] =1; /** debugging before **/
                    i_row++;  /** debugging before **/
                    continue;
                }

                int j=0;
                int buyCount=0;
                for (j=0 /** debugging before, not start from 1**/ ;j<TOTAL_COLUMN && buyCount <SMALLEST_TEN; j++ ){
                    if (buyNumber[i_row-1][j] != 1 ) continue;
                    buyNumber[i_row][j] =1;
                    buyCount++;
                }

                /** debugging before for first row is negative number**/
                if (buyCount ==0){
                    buyNumber[i_row][0]=1;
                }

            }else if ( maxProfits[i_row] >=0 ){
                totalProfit = totalProfit +maxProfits[i_row];
                numberBuy =0;
                for (i_item =0; i_item< numItems[i_row] && items[i_row][i_item].profit == maxProfits[i_row]  && numberBuy<SMALLEST_TEN;i_item++){

                     numberBuy++;

                     if (i_row == 0){
                        buyNumber[i_row][ items[i_row][i_item].number] =1;


                        if (maxProfits[i_row] ==0 ){ /** debugging before **/
                             buyNumber[i_row][0] =1; /** debugging before **/
                        }

                        continue;
                     }

                     int numberBuy2=0;
                     for (i=0 /** debugging before, not start from 1**/;i<TOTAL_COLUMN && numberBuy2 < SMALLEST_TEN ;i++){
                        if (buyNumber[i_row -1][i] != 1 ){
                            continue;
                        }

                        /** debugging before, when maxProfits == zero,
                        there are two choices both getting the item and not getting the item.
                        **/
                        if (maxProfits[i_row] ==0){
                            buyNumber[i_row][i]=1; /** not getting the item **/
                        }


                        buyNumber[i_row][i+ items[i_row][i_item].number ] =1;
                        numberBuy2++;
                     }
                }

                /** debugging beofre  when maxProfit ==0  &&  numberBuy==0 **/
                if (maxProfits[i_row] ==0 && numberBuy ==0){
                    buyNumber[i_row][0]=1;
                }

            }

            i_row++;

        }


        printf("Workyards %d\n",no);
        printf("Maximum profit is %d.\n",totalProfit);
        printf("Number of pruls to buy:");


        numberBuy =0;
        for (i=0 /**debug before, when MAX profits is zero. I should print 0 as output also. **/ ; i<TOTAL_COLUMN && numberBuy < SMALLEST_TEN ;i++){
            if (  buyNumber[i_row -1][i] != 1 ) continue;
            printf(" %d",i);
            numberBuy++;
        }

        if (numberBuy ==0){
            printf(" 0");
        }

        printf("\n");

        no++;


    }


    return 0;
}
