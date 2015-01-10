/* mixer.c */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
  FILE *f1, *f2;
  int flag1, flag2;
  double n1, n2;

  /* file load */
  if(argc != 3){
    printf("Usage: a.out file1 file2\n");
    exit(1);
  }

  f1 = fopen(argv[1], "r");
  if(f1 == NULL){
    printf("cannot open %s.\n",argv[1]);
    exit(1);
  }

  f2 = fopen(argv[2], "r");
  if(f2 == NULL){
    printf("cannot open %s.\n",argv[2]);
    exit(1);
  }
  
  /* add and putout */
  flag1 = 0;
  flag2 = 0;

  while(1){
    if(flag1 != EOF) flag1 = fscanf(f1, "%lf", &n1);
    else n1 = 0;

    if(flag2 != EOF) flag2 = fscanf(f2, "%lf", &n2);
    else n2 = 0;
    printf("%d\n",(int)n1+n2);

    if(flag1 == EOF && flag2 == EOF) break;
  }

  return 0;
}
