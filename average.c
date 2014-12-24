#include <stdio.h>

#define POINT 8

int main(void){
  int n[POINT];
  int i,t;

  for(i=0; i<POINT; i++){
    n[i] = 0;
  }

  scanf("%d",&t);
  while(t!=EOF){    
    for(i=0; i<POINT; i++){
      n[i] += t/POINT;
    }
    printf("%d\n",n[0]);    
    for(i=1; i<POINT; i++){
      n[i-1] = n[i];
    }
    n[POINT-1] = 0;
    scanf("%d",&t);   
  }
  return 0;
}
