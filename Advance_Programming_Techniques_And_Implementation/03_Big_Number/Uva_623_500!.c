#include <stdio.h>
#include <stdlib.h>
#include <string.h>
# define LENGTH 2637

/** 500!-1000! =>  (10^3)500 => 10^1500 => 1501   , 500! gives 1135-digit number
    1!-1000! => 1501 +1135 = 2636 **/

int ans[LENGTH];
int ansLength =0;
int num1[LENGTH];
int num1Length =0;

int num2[4];
int num2Length =0;


int version1_Main()
{
    #if !defined(ONLINE_JUDGE)
        freopen("623.in","r",stdin);
        freopen("output.txt","w",stdout);
    #endif
    int n=0;

    while (scanf("%d",&n) !=EOF /** debugging before, && n **/ ){

       memset(num1,0,sizeof(num1));
       num1[0]=1;
       num1Length=1; /** debugging before **/


       memset(ans,0,sizeof(ans));
       ansLength=0;

       int i=1;
       for(i=1 /** debugging before, start with 1 because 1! **/;i<=n;i++){
           if (i != 1){
              copyAnsToNum1();
           }
           convertIntToBigNumber(i);
           multiplyNum1Num2();
       }

       printf("%d!\n",n);
       if (n== 0) printf("1"); /** debugging before, 0!=1 **/

       i=0;
       for (i=ansLength-1;i>=0;i--){
          printf("%d", ans[i]);
       }
       printf("\n");
    }

    return 0;
}

void convertIntToBigNumber(int n){
    memset(num2,0,sizeof(num2));

    num2Length=0;
    while(n /**debugging before **/){
       num2[num2Length] = n%10;
       n= n/10;
       num2Length++;
    }
}

int multiplyNum1Num2(){
    int i_num2 =0;
    int j=0;
    int carry=0;
    int value =0;
    for (i_num2=0; i_num2<num2Length; i_num2++ ){
        if ( num2[i_num2] == 0 ) continue;
        carry=0;

        for (j=0;j< num1Length;j++){

            value = (num1[j] * num2[i_num2])+carry;
            value = ans[i_num2+j]+ value; /**debugging before, forgetting adding ans value **/
            ans[i_num2+j] =  value % 10;
            carry = value / 10;
            ansLength = i_num2+j+1;
        }


        while (carry){  /**debugging before**/
            ans[ansLength++]= carry%10;  /**debugging before**/
            carry = carry/10;   /**debugging before**/
        }

        if (ansLength > LENGTH ){
           printf("### overflow") ;
        }
    }
}

void copyAnsToNum1(){
   memset(num1,0,sizeof(num1));
   num1Length = ansLength;
   int i=0;
   for (i=0;i<ansLength;i++){
        num1[i]=ans[i];
   }

   memset(ans,0,sizeof(ans));
   ansLength=0;
}

/** version 1 has too high computing time complexity, so that I will get
    Time limit exceed.
**/

int answer[LENGTH];
int answerLength =0;

int main()
{
    #if !defined(ONLINE_JUDGE)
        freopen("623.in","r",stdin);
        freopen("output.txt","w",stdout);
    #endif
    int n=0;

    while (scanf("%d",&n) !=EOF /** debugging before, && n **/ ){

       memset(answer,0,sizeof(answer));
       answer[0]=1;     /** debugging before, because 1!=1 **/
       answerLength=1; /** debugging before **/

       int i=1;
       for(i=1 ;i<=n;i++){
           multiply(i);
       }

       printf("%d!\n",n);

       i=0;
       for (i=answerLength-1;i>=0;i--){
          printf("%d", answer[i]);
       }
       printf("\n");
    }

    return 0;
}

void multiply(int n){
    int i=0;
    int value =0;
    int carry=0;
    for(i=0;i<answerLength;i++){
        value = (answer[i]* n )+ carry ; /** debugging before**/
        answer[i]=value%10;
        carry = value / 10;
    }


    while (carry){
       answer[answerLength++] = carry % 10;
       carry = carry / 10;
    }
}

