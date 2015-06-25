/**
    press button's move                  graph array[3][3]
 (-1,-1)  | (-1,0)  | (-1,1)             (0,0)  | (0,1)  | (0,2)
 -----------------------    mapping  -----------------------
 (0,-1)   | (0,0)   | (0,1)     ==>     (1,0)  | (1,1)  | (1,2)
 -----------------------             -----------------------
 (1,-1)   | (1,0)   | (1,1)            (2,0)  | (2,1)  | (2,2)
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N 5
#define MIN 100

int row=0;
int column=0;
int move[9][2]={ {-1,1},{0,1},{1,1},{-1,0},{0,0},{1,0},{-1,-1},{0,-1},{1,-1} };
int ans[N*N];
int pressBtns[N*N];
int pressBtnsTimes=0;
int minPressBtnsTimes=MIN;
char graph[N][N];
int state[N][N];
int testCase =1;

int init(){
    memset(ans,-1,sizeof(ans) );
    memset(pressBtns,-1,sizeof(pressBtns) );
    memset(graph,'0',sizeof(graph) );
    memset(state,0 /** debugging not -1 **/,sizeof(state) );
    pressBtnsTimes =0;
    minPressBtnsTimes=MIN;
}

int branchAndBound(int x){
     /**  if (x <2) return 1;   /** debugging to make branch to little **/
    int i=0;
    int j=0;
    for (i=0;i< x ;i++ ){ /** (x-2) --> x, debugging to make branch to little **/
        for (j=0;j<column;j++){
            if (state[i][j] ==0 ){
                return 0;
            }
        }
    }
    return 1;
}


void pressButtons(int x,int y, int * lightenNum){

     int i=0;
     int new_x=0;
     int new_y=0;
     int mappingGX=0;
     int mappingGY=0;

     for (i=0;i<9;i++){
          new_x = x+ move[i][0];
          new_y = y+ move[i][1];
          mappingGX = 1+ move[i][0];
          mappingGY = 1+ move[i][1];

          if (new_x>=0 && new_x< row && new_y>=0 && new_y< column && graph[mappingGX][mappingGY]=='*' ){
              if (state[new_x][new_y] ==1 ){
                  state[new_x][new_y] =0;
                  (*lightenNum)--;
              }else{
                   state[new_x][new_y] =1;
                  (*lightenNum)++;
              }
          }
     }

}

/** following Right , but Time exceeding in OJ **/
/** 先一路按button，慢 **/
void dfs_right_IN_uva(int x, int y, int lightenNum ){
    int i=0;

     if( lightenNum == (row*column) ){
       if(pressBtnsTimes< minPressBtnsTimes){
          minPressBtnsTimes = pressBtnsTimes;
          for (i=0;i<minPressBtnsTimes;i++ ){
              ans[i]=pressBtns[i];
          }
       }
       return;
     }


     if (x >2  && state[x-2][y] ==0){
     	 return;
     }

     if ( x == row){ /** beyond row **/
         return;
     }

     if (y == column){


        int result = branchAndBound(x);
        if (!result){
            return;
        }

        /** when beyond column, need to run next row **/
        dfs_right_IN_uva(x+1,0,lightenNum);
        return;
     }

     pressButtons(x,y, &lightenNum);  /** 先按 button**/ /** show (x,y) be pressed , lighten state **/
     pressBtns[ pressBtnsTimes++] = x*column+y+1; /** convert (x,y) to button identified NO. **/

     dfs_right_IN_uva(x, y+1,lightenNum);  /** trace down to (x,y+1) lighten state **/

     pressButtons(x,y, &lightenNum);  /** for show (x,y) be pressed , lighten state **/
     pressBtnsTimes--;
     dfs_right_IN_uva(x, y+1,lightenNum);

}

/**  先不按button ，直到走不下去再做dfs，快!!  **/
void dfs_right_IN_uva_2(int x, int y, int lightenNum){
    int i=0;

     if( lightenNum == (row*column) ){
       if(pressBtnsTimes< minPressBtnsTimes){
          minPressBtnsTimes = pressBtnsTimes;
          for (i=0;i<minPressBtnsTimes;i++ ){
              ans[i]=pressBtns[i];
          }
       }
       return;
     }

     if ( x == row){
         return;
     }

     if (y == column){
        int result = branchAndBound(x);
        if (!result){
            return;
        }
        dfs_right_IN_uva_2(x+1,0,lightenNum);
        return;
     }

     dfs_right_IN_uva_2(x, y+1,lightenNum);  /** 先不按 button**/ /// trace down to (x,y+1) lighten state
     pressButtons(x,y, &lightenNum);  /// show (x,y) be pressed , lighten state
     pressBtns[ pressBtnsTimes++] = x*column+y+1; /// convert (x,y) to button identified NO.


     dfs_right_IN_uva_2(x, y+1,lightenNum);
     pressButtons(x,y, &lightenNum);  /// for show (x,y) be pressed , lighten state
     pressBtnsTimes--;
}
/** above Right , but Time exceeding in OJ **/




void dfs_PressFirst_IN_NTHU_UPDATE_03(int x, int y, int lightenNum , int pressNum){

     if (y == column){
          /** speed up to bound more branch,
            if mark  int result = branchAndBound(x);
            it will still pass in  nthu ON-line J!!
         **/
         int result = branchAndBound(x);
         if (!result){
                return;
         }

         x++;
         y=0;
     }


     int i=0;
     if ( pressNum > minPressBtnsTimes){
        return ;
     }



     if (x < row){


        pressButtons(x,y, &lightenNum);  /** show (x,y) be pressed , lighten state **/
        pressBtns[ pressNum] = (x*column)+y+1; /** convert (x,y) to button identified NO. **/
        dfs_PressFirst_IN_NTHU_UPDATE_03(x,y+1,lightenNum, pressNum+1);  /** trace down to (x,y+1) lighten state **/

        pressButtons(x,y, &lightenNum);
        dfs_PressFirst_IN_NTHU_UPDATE_03(x,y+1,lightenNum, pressNum);
     }else{

        int i=0;
        int j=0;
        for (i=0; i< row; i++ ){
            for (j=0; j<column; j++){
                if (state[i][j] ==0  ){   /// debugging not -1
                    return 0;
                }
            }
        }

        /**  因為folow 先按button 且 由數字小的button
         做，所以最短的一定是 造index 排 **/

        if (pressNum < minPressBtnsTimes){
            minPressBtnsTimes = pressNum;
            for( i=0; i<minPressBtnsTimes; i++){
                ans[i] = pressBtns[i];
            }
        }
     }
}




void dfs_PressFirst_IN_NTHU_UPDATE_02(int x, int y, int lightenNum , int pressNum){

     if (y == column){
          /** speed up to bound more branch,
            if mark  int result = branchAndBound(x);
            it will still pass in  nthu ON-line J!!
         **/
         int result = branchAndBound(x);
         if (!result){
                return;
         }

         x++;
         y=0;
     }


     int i=0;
     if ( pressNum > minPressBtnsTimes){
        return ;
     }


     int index = x*column +y ;

     if( index == (row*column) ){

        int i=0;
           int j=0;
           for (i=0;i< row;i++ ){
                for (j=0;j<column;j++){
                    if (state[i][j] ==0  ){  /// debugging not -1
                        return 0;
                    }
                }
            }

          /**  因為folow 先按button 且 由數字小的button
           做，所以最短的一定是 造index 排 **/

         if (pressNum < minPressBtnsTimes){
             minPressBtnsTimes = pressNum;
             for( i=0;i<minPressBtnsTimes;i++){
                ans[i] = pressBtns[i];
             }
        }
       return;
     }


     /**
      if ( x == row){ //// beyond row
         return;
     }**/

     /**
     if (x >=2  && state[x-2][y] ==0){
     	 return;
     }**/



     pressButtons(x,y, &lightenNum);  /** show (x,y) be pressed , lighten state **/
     pressBtns[ pressNum] = (x*column)+y+1; /** convert (x,y) to button identified NO. **/
     dfs_PressFirst_IN_NTHU_UPDATE_02(x,y+1,lightenNum, pressNum+1);  /** trace down to (x,y+1) lighten state **/

     pressButtons(x,y, &lightenNum);
     dfs_PressFirst_IN_NTHU_UPDATE_02(x,y+1,lightenNum, pressNum);

}



void dfs_PressFirst_IN_NTHU_UPDATE_01(int x, int y, int lightenNum , int pressNum){
    int i=0;
     if ( pressNum > minPressBtnsTimes){
        return ;
     }


     int index = x*column +y ;

     if( index == (row*column) ){

        int i=0;
           int j=0;
           for (i=0;i< row;i++ ){
                for (j=0;j<column;j++){
                    if (state[i][j] ==0  ){  /// debugging not -1
                        return 0;
                    }
                }
            }

          /**  因為folow 先按button 且 由數字小的button
           做，所以最短的一定是 造index 排 **/

         if (pressNum < minPressBtnsTimes){
             minPressBtnsTimes = pressNum;
             for( i=0;i<minPressBtnsTimes;i++){
                ans[i] = pressBtns[i];
             }
        }
       return;
     }


      if ( x == row){ /** beyond row **/
         return;
     }

      /*** fatal error !!
        if (y == column){
            dfs(x+1,0 ,lightenNum, pressNum);
            return;
        }

        if (x >=2  && state[x-2][y] ==0){
            return;
        }
      **/


      if (y == column){
          /** speed up to bound more branch,
            if mark  int result = branchAndBound(x);
            it will still pass in  nthu ON-line J!!
         **/
            int result = branchAndBound(x);
            if (!result){
                return;
            }

        /** when beyond column, need to run next row **/
        ///dfs(x+1,0,lightenNum, pressNum);
        dfs_PressFirst_IN_NTHU_UPDATE_01(x+1,0 ,lightenNum, pressNum);
        return;
     }

     /**
     if (x >=2  && state[x-2][y] ==0){
     	 return;
     }**/



     pressButtons(x,y, &lightenNum);  /** show (x,y) be pressed , lighten state **/
     pressBtns[ pressNum] = (x*column)+y+1; /** convert (x,y) to button identified NO. **/
     dfs_PressFirst_IN_NTHU_UPDATE_01(x,y+1,lightenNum, pressNum+1);  /** trace down to (x,y+1) lighten state **/

     pressButtons(x,y, &lightenNum);
     dfs_PressFirst_IN_NTHU_UPDATE_01(x,y+1,lightenNum, pressNum);


    /**  compare both of them  !!!
       /// 先不按，直到走不下去
     dfs_Right_NTHU_Not_PressButton_Frist(x, y+1,lightenNum);
     pressButtons(x,y, &lightenNum);
     pressBtns[ pressBtnsTimes++] = x*column+y+1;
     dfs_Right_NTHU_Not_PressButton_Frist(x, y+1,lightenNum);

     pressButtons(x,y, &lightenNum);
     pressBtnsTimes--;


      /// 先不按，直到走不下去
     dfs_right2_IN_NTHU(x,y+1,lightenNum, pressNum);
     pressButtons(x,y, &lightenNum);
     pressBtns[ pressNum] = (x*column)+y+1;
     dfs_right2_IN_NTHU(x,y+1,lightenNum, pressNum+1 );

     pressButtons(x,y, &lightenNum);

     **/
}




void dfs_Right_NTHU_Not_PressButton_Frist(int x, int y, int lightenNum){
    int i=0;

    if ( pressBtnsTimes > minPressBtnsTimes){
        return ;
     }

     int index = x*column +y ; /** debugging not  x*column +y+1 **/

     if( index == (row*column) ){

        int i=0;
           int j=0;
           for (i=0;i< row;i++ ){
                for (j=0;j<column;j++){
                    if (state[i][j] ==0 /** debugging not -1 **/ ){
                        return 0;
                    }
                }
            }

         if (pressBtnsTimes ==minPressBtnsTimes){
            int isCurrentSmallerAns = 0;
            for( i=0;i<minPressBtnsTimes;i++){
                if ( ans[i] > pressBtns[i] ){
                    isCurrentSmallerAns =1;
                    break;
                }
            }

            if(isCurrentSmallerAns==1){
                 for( i=0;i<minPressBtnsTimes;i++){
                    ans[i] = pressBtns[i];
                }
            }

        }else if (pressBtnsTimes < minPressBtnsTimes){

            minPressBtnsTimes = pressBtnsTimes;
            for( i=0;i<minPressBtnsTimes;i++){
                ans[i] = pressBtns[i];
            }
        }

       return;
     }





     if ( x == row){ /** beyond row **/
         return;
     }


     /*** fatal error !! order different!!
        1. 這要先 在2 前面 ，不然答案會錯!!
        if (x >=2  && state[x-2][y] ==0){
            return;
        }
        2.
        if (y == column){
            dfs(x+1,0 ,lightenNum, pressNum);
            return;
        }
      **/

     if (y == column){

        int result = branchAndBound(x);  /** speed up very much **/
        if (!result){
            return;
        }
        /**when beyond column, need to run next row **/
        dfs_Right_NTHU_Not_PressButton_Frist(x+1,0,lightenNum);
        return;
     }

     /** add result = branchAndBound(x) will better than
         following.
     if (x >=2  && state[x-2][y] ==0){
     	 return;
     }**/


     /**  先不按，直到走不下去 **/
     dfs_Right_NTHU_Not_PressButton_Frist(x, y+1,lightenNum);
     pressButtons(x,y, &lightenNum);
     pressBtns[ pressBtnsTimes++] = x*column+y+1;

     dfs_Right_NTHU_Not_PressButton_Frist(x, y+1,lightenNum);
     pressButtons(x,y, &lightenNum);
     pressBtnsTimes--;

}


int main()
{
    /**
    #if !defined(ONLINE_JUDGE)
      freopen("input.txt","r",stdin);
      freopen("10318.in","r",stdin);
      freopen("output.txt","w",stdout);
    #endif
    **/


    while( scanf("%d %d",&row, &column)==2 && row && column ){
        init();
        int i=0;
        while (i<3){
            scanf("%s",&graph[i]);
            i++;
        }
        dfs_PressFirst_IN_NTHU_UPDATE_03(0,0,0,0);
        //dfs(0,0,0);

        printf("Case #%d\n",testCase);
        if (minPressBtnsTimes == MIN){
            printf("Impossible.\n");
        }else{
            printf("%d",ans[0]);
            for(i=1;i<minPressBtnsTimes;i++){
                printf(" %d",ans[i]);
            }
            printf("\n") ;
        }
        testCase++;

    }


    return 0;
}
