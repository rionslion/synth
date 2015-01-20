#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POINT_MAX 44100
#define SAMPLING_RATE 44100

int n[POINT_MAX];
int point, start, finish, s;

int main(int argc, char *argv[]){
  int i, t, f;
  double s_sec, f_sec;

  if(argc != 4){
    printf("Usage: a.out point start_time finish_time\n");
    exit(1);
  }

  point = atoi(argv[1]);
  s_sec = atof(argv[2]);
  f_sec = atof(argv[3]);

  start = SAMPLING_RATE * s_sec;
  finish = SAMPLING_RATE * f_sec;

  for(i=0; i<POINT_MAX; i++){
    n[i] = 0;
  }

  s = 0;
  f = scanf("%d",&t);
  while(f!=EOF){
    s++;
    if(s < start || s > finish+point) printf("%d\n",t);
    else{
      for(i=0; i<point; i++){
	n[i] += t/point;
      }
      printf("%d\n",n[0]);    
      for(i=1; i<point; i++){
	n[i-1] = n[i];
      }
      n[point-1] = 0;
    }
      
    f = scanf("%d",&t);   
  }
  return 0;
}
