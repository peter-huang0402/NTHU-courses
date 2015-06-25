#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define  MAX_LENGTH 1001

int ans[MAX_LENGTH];
int ansLength=0;
int num1[MAX_LENGTH];
int num1Length=0;
int num2[MAX_LENGTH];
int num2Length=0;

int tempCompute[MAX_LENGTH];
int tmpCmpLength=0;

char valueStr[MAX_LENGTH];
int valStrLength=0;

int value[MAX_LENGTH];
int valueIndex=0;

int compareBigNumber(int *a, int *b, int aLength, int bLength){
    if (aLength > bLength){
        return 1;
    }else if( aLength < bLength){
        return -1;
    }else{
        int i=0;
        for(i=aLength-1;i>=0;i--){ /** debugging before , not start form 0, should start with tail**/
            if ( (a[i] - b[i]) >0 ){
                return 1;
            }else if ((a[i]- b[i]) <0 ){
                return -1;
            }
        }
        return 0;
    }
}

void guessNum2(int n){
     memset(num2,0,sizeof(num2));
     num2Length =0;

     int i=0;
     for (i=tmpCmpLength-1;i>=0;i--){ /** debugging before , not start form 0, should start with tail**/
        num2[i+1]=tempCompute[i];
        num2Length++;
     }
     num2[0]=n;
     num2Length++;

     int value=0;
     int carry =0;
     for(i=0;i<num2Length;i++){
        value = num2[i]*n+carry;
        num2[i] = value %10;
        carry = value / 10;
     }

     while (carry){
        num2[num2Length] = carry %10;
        num2Length++;
        carry = carry / 10;
     }
}


int mainAAA(){
    num1[5];
    num2[4];
    num1Length=5;
    num2Length =4;
    num1[0]=5;
    num1[1]=8;
    num1[2]=0;
    num1[3]=1;
    num1[4]=1;

    num2[0]=4;
    num2[1]=4;
    num2[2]=2;
    num2[3]=9;

    num1SubtractNum2();
}

void num1SubtractNum2(){
     int i=0;
     int carry =0;
     int value=0;
     for (i=0;i<num1Length;i++){
         value = num1[i]-num2[i] + carry;
         if (value <0){
            /** carry = carry-1;  debugging before **/
            carry = -1;
            value = value +10;
         }else{
             carry =0;
         }
         num1[i] = value;
     }


     /** debugging before, because no update new num1's length**/
     int length=0;
     for(i=0;i<num1Length;i++){
        if (num1[i]){
            length=i+1;
        }
     }
     num1Length = length;
     /** debugging before, because no update new num1's length**/

}

void addTempCompute(int n){
    int i=0;
    for (i=tmpCmpLength-1 ;i>=0 ;i--){ /** debugging before , not start form 0, should start with tail**/
        tempCompute[i+1] = tempCompute[i];
    }
    tempCompute[0]=n+n;
    tmpCmpLength++;

    i=0;
    int carry=0;
    int value =0;
    for (i=0;i<tmpCmpLength;i++){
        value = (tempCompute[i]+ carry);
        tempCompute[i] = value %10;
        carry = value / 10;
    }

    while (carry){
        tempCompute[tmpCmpLength++] = carry %10;
        carry = carry /10;
    }

}

int main()
{
    #if !defined(ONLINE_JUDGE)
        freopen("10023.in","r",stdin);
        freopen("output.txt","w",stdout);
    #endif

    int n=0,i=0;
    bool isFirst = true;
    int temp=0;
    int guessNum = 0;

    /** debugging before
        while(scanf("%d",&n) !=EOF  && n--  ){
    **/
    scanf("%d",&n);

    while( n--){

        if( !isFirst){
             printf("\n");
        }

        isFirst = false;
        memset(ans,0,sizeof(ans));
        memset(num1,0,sizeof(num1));
        memset(num2,0,sizeof(num2));
        memset(tempCompute,0,sizeof(tempCompute));
        memset(valueStr,'\0',sizeof(valueStr));
        memset(value,0,sizeof(value));
        ansLength=0;
        num1Length=0;
        num2Length=0;
        tmpCmpLength=0;
        valStrLength=0;
        valueIndex=0;

         scanf("%s", &valueStr[0]);
         valStrLength = strlen(valueStr);

         for (i=valStrLength-1;i>=0;i--){
             value[valStrLength-i-1] = valueStr[i]-'0';
         }
         valueIndex= valStrLength-1;

         while(valueIndex >=0 ){
            /** forgetting moving original string backward **/
            for (i=num1Length-1;i>=0;i--){
                 num1[i+1]= num1[i];
            }
            num1[0] = value[valueIndex];
            num1Length++;
            valueIndex--;

            if ( (valueIndex % 2) ==0 /** debugging before ==0 **/ ){
                /** num1Length++; => should be added latter, debugging before ==0 **/
                for (i=num1Length-1; i>=0 ;i--){ /**  debugging before, copying value should start from tail **/
                /** for(i=0;i<num1Length;i++){   **/
                    num1[i+1]= num1[i];
                }
                num1Length++; /** debugging before **/
                num1[0]=value[valueIndex];
                valueIndex--;
            }

            for(i=0;i<=9;i++){

                guessNum2(i);
                temp = compareBigNumber(&num1[0],&num2[0],num1Length, num2Length);

                guessNum =i; /** debugging before for 0=-1 && 9 =10 corner case!**/

                if (temp ==0){
                    break;
                }else if (temp <0){

                    if (i==0) { /** debugging before **/
                        guessNum =0; /** debugging before **/
                    }else{
                        /** i = i-1;   fatal error , debugging defore **/
                        guessNum = i-1;
                    }
                    break;
                }
            }

            /** guessNum = i;  debugging before => causing guessNum =10 **/
            ans[ansLength++]= guessNum;

            guessNum2(guessNum);
            num1SubtractNum2();
            addTempCompute(guessNum);

            /**  "valueIndex--;" debugging before, should not control valueIndex again **/
         }

         for(i=0;i<ansLength;i++){
            printf("%d",ans[i]);
         }
         printf("\n");



    }

    return 0;
}
