#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LEN 26     /** debugging 25 -> 26 25, run time error !! I guess letter containing 25 letter but need extra one bit for ending character**/

char names[LEN][LEN];
int isVisit[LEN];
int map[LEN][LEN];
int person=0;

int search(char name[],int *len){
    int i=0;
    int value =0;
    for(i=0;i<(*len) /**debugging,person**/ ;i++){
        value = strcmp(names[i],name);
        if (value ==0) return i;
    }


    int idx = (*len);
    /** names[(*len)] = name;   debugging **/
    strcpy(names[ (*len)],name); /** debugging **/

    (*len) = (*len) +1;
    return idx;
}

int main()
{
    

    int line=0;
    int i=0;
    int j=0;
    int k=0;
    char n1[LEN];
    char n2[LEN];
    int testcase=1;
    while( scanf("%d %d",&person,&line) ==2 && person>0 && line>0 ){

          memset(names,'0',sizeof(names));
          memset(isVisit,0,sizeof(isVisit));
          memset(map,0,sizeof(map));

          i=line;
          int len=0;
          int a1=0;
          int b1=0;

          while(i--){
             scanf("%s %s",&n1,&n2);
             a1 = search(n1,&len);
             b1 = search(n2,&len);
             map[a1][b1]=1;
             map[a1][a1]=1;
             map[b1][b1]=1;
          }

          for(k=0;k<person;k++){
                for(i=0;i<person;i++){
                    for(j=0;j<person;j++){
                        if ( map[i][j] == 1 || ( map[i][k]&& map[k][j] ) ){
                            map[i][j]=1;
                        }
                    }
                }
          }

          if (testcase != 1){
              printf("\n");
          }

          printf("Calling circles for data set %d:\n",testcase);

          for (i=0;i<person;i++){
               if (isVisit[i] ==1 ) continue;
               isVisit[i]=1;
               printf("%s",names[i]);


               for (j=i+1;j<person;j++){
                   /*** debugging fatal error!!.
                   if (isVisit[j]==1) continue;
                   isVisit[j]=1;
                   if (map[i][j] ==1 && map[j][i]==1 ){
                       printf(", %s",names[j]);
                   }**/

                   if (map[i][j] ==1 && map[j][i]==1 ){
                        if (isVisit[j]==1) continue;
                        isVisit[j]=1;
                        printf(", %s",names[j]);
                    }
               }
               printf("\n");
          }



          testcase++;
    }

    return 0;
}
