#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

double calculatedFormula(double x, int p, int q, int r, int s, int t, int u){
    double value = p * exp(-x) + q * sin(x)+ r * cos(x) + s * tan(x) + t* x*x+ u;
    return value;
}

/*
  case: 0 0 0 0 -1 1
  Error
        my answer= No solution
        correct answer = 1.0
*/


int main()
{
    #if !defined(ONLINE_JUDGE)
      freopen("10341.in","r",stdin);
      freopen("output.txt","w",stdout);
    #endif

    int p,  q,  r,  s,  t,  u;

    double upper, lower, mid;


    double calculatedUpper, calculatedMid, calculatedLower;

    bool hasResult = true;

    while ( scanf("%d %d %d %d %d %d",&p, &q,&r,&s,&t,&u) != EOF ){

        upper =1.0;
        lower =0.0;
        hasResult = true;

        while( (upper - lower) >= 1e-9 ){
            mid = (upper+ lower ) /2 ;

            calculatedUpper= calculatedFormula(upper, p,q,r,s,t,u);
            calculatedMid = calculatedFormula(mid, p,q,r,s,t,u);
            calculatedLower = calculatedFormula(lower, p,q,r,s,t,u);

            if ( calculatedUpper * calculatedLower >0 ){
                hasResult = false;
                break;
            }

            /** error
            if (calculatedMid * calculatedUpper <0 ){
                lower = mid;
            }else{
                upper = mid;
            }*/


           if ((calculatedMid * calculatedLower) >0 ){
			  lower = mid;
           }else{
		     upper = mid;
           }

        }

        if ( ! hasResult ){
                /*
            printf("No solution, upper=%lf, lower=%lf\n",upper,lower);
            */
            printf("No solution\n");
        }else{
            /*
            printf("%.4lf , lower=%lf\n",upper,lower);
            */
            printf("%.4lf\n",upper );
        }
    }

    return 0;
}
