/******************************************************************/
/* txt2wav $A%F%-%9%H%G$B!<$A%?(B -> wav$A%U%)$B!<$A%^%C%H%G$B!<$A%?(B $BJQ49$A%W%m%0%i%`(B */
/*                                                                */
/* $A%G$B!<$A%?(B 16bit(-2^15 $A!+(B 2^15 - 1, -32768 $A!+(B +32767, $BL5$ARt(B = 0)    */
/* $A%G$B!<$A%?(B  8bit(    0 $A!+(B 2^8  - 1 ,     0 $A!+(B +225  , $BL5$ARt(B = 128)  */
/*                                                                */
/* $A%b%N%i%k(B(1ch)    $A%9%F%l%*(B(2ch)                                 */
/* data             Lch Rch                                       */
/* data             Lch Rch                                       */
/*  :                  :                                          */
/*                                                                */
/* 2000.11.13                                                     */
/* kondo@kk.iij4u.or.jp                                           */
/******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define PCM 1
#define BUFSIZE (1000)
#ifndef UCHAR_MAX
#define UCHAR_MAX (255)
#endif
#ifndef UCHAR_MIN
#define UCHAR_MIN (0)
#endif
#ifndef SHRT_MAX
#define SHRT_MAX (32767)
#endif
#ifndef SHRT_MIN
#define SHRT_MIN (-32768)
#endif
#ifndef _MAX_PATH
#define _MAX_PATH (255)
#endif

static char cmdname[] = "txt2wav";

void usage(void)
{
  fprintf(stderr,
          "\n"
          "ASCII $A%F%-%9%H%G$B!<$A%?(B ---> wav $A%U%)$B!<$A%^%C%H$BJQ49$A%W%m%0%i%`(B...\n"
          "$AJ9$$7=(B : %s [sampling_rate] <text_file> [wav_file]\n"
          "\t sampling_rate $A$O(B "
          "- $A$K$BB3$A$1$F%5%s%W%j%s%0%l$B!<$A%H(B($BC1$AN;(B Hz)$A$rV86($7$^$9(B\n"
          "\t (wav_file $A$NV86($,$BL5$A$$$B>l$A:O(B "
          "text_file.wav $A$N%U%!%$%k$,Ww3I$5$l$^$9(B)\n"
          "\n"
          "$A@}(B : %s -44100 filename.txt\n"
          "     (filename.txt $A$N%G$B!<$A%?$r(B 44.1kHz $A$G(B "
          "filename.wav $A$K$B=q$A$-3v$7$^$9(B)\n"
          "     %s -48000 filename.txt output.wav\n"
          "     (filename.txt $A$N%G$B!<$A%?$r(B 48kHz $A$G(B "
          "output.wav $A$K$B=q$A$-3v$7$^$9(B)\n"
          "\n"
          "$A%G$B!<$A%?$,(B 0$A!+(B255 $A$J$i$P(B 8bit wav $A$r3vA&$7$^$9(B\n"
          "$A$=$lRTMb$J$i$P(B 16bit wav $A$r3vA&$7$^$9(B\n"
          "( 8bit $A%G$B!<$A%?(B :      0$A!+(B  +255, $BL5$ARt(B=128)\n"
          "(16bit $A%G$B!<$A%?(B : -32768$A!+(B+32767, $BL5$ARt(B=  0)\n"
          "\n"
          "$A%F%-%9%H%G$B!<$A%?PNJ=(B : \n"
          "\t1ch($A%b%N%i%k(B)$A$N$B>l$A:O(B    2ch($A%9%F%l%*(B)$A$N$B>l$A:O(B\n"
          "\tdata                   Lch Rch\n"
          "\tdata                   Lch Rch\n"
          "\t  :                     :   :\n"
          "\n",
          cmdname, cmdname, cmdname);
  exit(1);
}

void openerror(char *filename)
{
  fprintf(stderr, "Can't open file: %s\n", filename);
  exit(1);
}

void formaterror(FILE *fp)
{
  fprintf(stderr, "File format error : %ld\n", ftell(fp));
  exit(1);
}

/*----------------*/
/* $A%U%!%$%kC{H!5C(B */
/*----------------*/
int getbasename(char *dest, char *src)
{
  int i, start, end, ret;

  i = -1;
  start = 0;
  end = 0;

  // $A%U%!%$%kC{$N$O$8$a$H$B=*$A$o$j$r$B8!$A3v(B
  while (src[++i]) {
    if (src[i] == '\\' || src[i] == ':') {
      start = i + 1;
      end = 0;
    }
    if (src[i] == '.') {
      end = i;
    }
  }
  if (end == 0) {
    end = i;
  }

  // $A%U%!%$%kC{$,SP$k$B>l$A:O(B
  if (start < end) {
    for (i = 0; i < end; i++) {
      dest[i] = src[i];
    }
    dest[i] = '\0';
    ret = 1;
  }
  else {
    dest[0] = '\0';
    ret = 0;
  }

  return ret;
}

/*----------------------*/
/* ASCII $A%U%!%$%k$NPPJ}(B */
/*----------------------*/
unsigned long filesize(FILE *fp)
{
  int c;
  long count = 0;

  rewind(fp);

  while ((c = getc(fp)) != EOF) {
    if (c == '\n') {
      count++;
    }
  }

  return count;
}

/*--------------------*/
/* $AA?WS;/%S%C%HJ}$B3NG'(B */
/*--------------------*/
short bytecheck(FILE *fp)
{
  int data;
  char buf[BUFSIZE];

  rewind(fp);

  while (fscanf(fp, "%s", buf) != EOF) {
    data = strtol(buf, (char **) NULL, 10);

    if (UCHAR_MAX < data) {
      return 2;
    }
    else if (data < UCHAR_MIN) {
      return 2;
    }
  }

  // $A%G$B!<$A%?$,(B 0$A!+(B255 $A$J$i$P(B 1byte(8bit)
  return 1;
}

/*--------------*/
/* $A%A%c%s%M%kJ}(B */
/*--------------*/
short chcheck(FILE *fp)
{
  int a, b;
  short ch;
  char buf[BUFSIZE];
  char s[2];

  rewind(fp);
  fgets(buf, BUFSIZE, fp);
  ch = sscanf(buf, "%d %d %1s", &a, &b, s);

  if (ch == 1 || ch == 2) {
    ;
  }
  else {
    fprintf(stderr, "channel must be 1 or 2\n");
    formaterror(fp);
  }

  return ch;
}

/*------------------------------*/
/* $A%G$B!<$A%?%*$B!<$A%P$B!<$A%U%m$B!<$A%A%'%C%/(B */
/*------------------------------*/
short datacheck(short data, short max, short min)
{
  // $A%*$B!<$A%P$B!<$A%U%m$B!<$A$9$k$B>l$A:O(B + $A$+(B - $A$r1mJ>(B
  if (data > max) {
    fprintf(stderr, "+");
    data = max;
  }
  if (data < min) {
    fprintf(stderr, "-");
    data = min;
  }

  return data;
}

/*--------------------------------------*/
/* 16 bit $A%G$B!<$A%?(B -32768$A!+(B32767 ($BL5$ARt(B 0) */
/* 8 bit $A%G$B!<$A%?(B 0$A!+(B255 ($BL5$ARt(B 128)       */
/*--------------------------------------*/
long datawrite(unsigned short ch, unsigned short byte,
               FILE *txt, FILE *wav)
{
  short left, right;
  int n;
  unsigned long count = 0;
  char s[2], buf[BUFSIZE];

  rewind(txt);

  while ((fgets(buf, BUFSIZE, txt) != NULL)) {
    n = sscanf(buf, "%hd %hd %1s", &left, &right, s);

    if (feof(txt)) {
      break;
    }
    if (ch != n) {
      formaterror(txt);
    }
    // $A%b%N%i%k(B
    else if (ch == 1) {
      // 8 $A%S%C%H(B
      if (byte == 1) {
        datacheck(left, UCHAR_MAX, UCHAR_MIN);
        fputc(left, wav);
        count++;
      }
      // 16 $A%S%C%H(B
      else if (byte == 2) {
        datacheck(left, SHRT_MAX, SHRT_MIN);
        fwrite(&left, sizeof(short), 1, wav);
        count++;
      }
    }
    // $A%9%F%l%*(B
    else if (ch == 2) {
      // 8 $A%S%C%H(B
      if (byte == 1) {
        datacheck(left, UCHAR_MAX, UCHAR_MIN);
        datacheck(right, UCHAR_MAX, UCHAR_MIN);
        fputc(left, wav);
        fputc(right, wav);
        count++;
      }
      // 16 $A%S%C%H(B
      else if (byte == 2) {
        datacheck(left, SHRT_MAX, SHRT_MIN);
        datacheck(right, SHRT_MAX, SHRT_MIN);
        fwrite(&left, sizeof(short), 1, wav);
        fwrite(&right, sizeof(short), 1, wav);
        count++;
      }
    }
    else {
      formaterror(txt);
    }
  }

  return count;
}

/*--------------------*/
/* $A%5%s%W%j%s%0%l$B!<$A%H(B */
/*--------------------*/
unsigned long sampling(void)
{
  unsigned long sr = 0;
  char buf[BUFSIZE];

  while (sr == 0) {
    fprintf(stderr, "sampling rate [Hz] : ");
    fgets(buf, BUFSIZE, stdin);
    sr = labs(strtoul(buf, (char **) NULL, 10));
  }

  return sr;
}

/*----------------*/
/* $A8w$B<o$A%Q%i%a$B!<$A%?(B */
/*----------------*/
unsigned long parameter(FILE *fp,
                        unsigned long *points,
                        unsigned short *channel,
                        unsigned short *byte)
{
  // $A%G$B!<$A%?J}(B
  *points = filesize(fp);

  // $AA?WS;/%P%$%HJ}(B 1Byte = 8bit, 2Byte = 16bit
  *byte = bytecheck(fp);

  // $A%A%c%s%M%kJ}(B
  *channel = chcheck(fp);

  return (unsigned long) (*points * *byte * *channel);
}

/*----------------------*/
/* wav $A%U%!%$%k$B=q$A$-3v$7(B */
/*----------------------*/
void wavwrite(char *datafile, char *wavfile, unsigned long sr)
{
  unsigned long points, file_size, count, var_long;
  unsigned short ch, bytes, var_short;
  char s[4];
  FILE *fp1, *fp2;

  // $A%F%-%9%H%U%!%$%k(B
  if ((fp1 = fopen(datafile, "r")) == NULL) {
    openerror(datafile);
  }

  file_size = parameter(fp1, &points, &ch, &bytes);
  printf("%s : %ld Hz sampling, %d bit, %d channel\n",
         wavfile, sr, bytes * 8, ch);

  // WAV $A%U%!%$%k(B
  if ((fp2 = fopen(wavfile, "wb")) == NULL) {
    openerror(wavfile);
  }

  // RIFF $A%X%C%@(B
  s[0] = 'R';
  s[1] = 'I';
  s[2] = 'F';
  s[3] = 'F';
  fwrite(s, 1, 4, fp2);
  // $A%U%!%$%k%5%$%:(B
  var_long = file_size + 36;
  fwrite(&var_long, sizeof(long), 1, fp2);
  printf("  file size (header + data) = %ld [Byte]\n", var_long);

  // WAVE $A%X%C%@(B
  s[0] = 'W';
  s[1] = 'A';
  s[2] = 'V';
  s[3] = 'E';
  fwrite(s, 1, 4, fp2);

  // chunkID (fmt $A%A%c%s%/(B)
  s[0] = 'f';
  s[1] = 'm';
  s[2] = 't';
  s[3] = ' ';
  fwrite(s, 1, 4, fp2);
  // chunkSize (fmt $A%A%c%s%/$N%P%$%HJ}(B $BL505=L(B wav $A$O(B 16)
  var_long = 16;
  fwrite(&var_long, sizeof(long), 1, fp2);
  // wFromatTag ($BL505=L(B PCM = 1)
  var_short = PCM;
  fwrite(&var_short, sizeof(short), 1, fp2);
  // dwChannels ($A%b%N%i%k(B = 1, $A%9%F%l%*(B = 2)
  fwrite(&ch, 2, 1, fp2);
  printf("  PCM type                  = %hu\n", var_short);
  // dwSamplesPerSec ($A%5%s%W%j%s%0%l$B!<$A%H(B(Hz))
  fwrite(&sr, sizeof(long), 1, fp2);
  printf("  sampling rate             = %ld [Hz]\n", sr);
  // wdAvgBytesPerSec (Byte/$ACk(B)
  var_long = bytes * ch * sr;
  fwrite(&var_long, sizeof(long), 1, fp2);
  printf("  Byte / second             = %ld [Byte]\n", var_long);
  // wBlockAlign (Byte/$A%5%s%W%k(B*$A%A%c%s%M%k(B)
  var_short = bytes * ch;
  fwrite(&var_short, sizeof(short), 1, fp2);
  printf("  Byte / block              = %hu [Byte]\n", var_short);
  // wBitsPerSample (bit/$A%5%s%W%k(B)
  var_short = bytes * 8;
  fwrite(&var_short, sizeof(short), 1, fp2);
  printf("  bit / sample              = %hu [bit]\n", var_short);

  // chunkID (data $A%A%c%s%/(B)
  s[0] = 'd';
  s[1] = 'a';
  s[2] = 't';
  s[3] = 'a';
  fwrite(s, 1, 4, fp2);
  // chunkSize ($A%G$B!<$A%?$BD9(B Byte)
  fwrite(&file_size, 4, 1, fp2);
  printf("  file size (data)          = %ld [Byte]\n", file_size);

  // $A%X%C%@(B (44 Byte) $B=q$A$-$B9~$A$_$3$3$^$G(B
  if (ftell(fp2) != 44) {
    fprintf(stderr, "%s : wav header write error\n", wavfile);
    exit(1);
  }

  // waveformData ($A%G$B!<$A%?(B) $B=q$A$-$B9~$A$_(B
  count = datawrite(ch, bytes, fp1, fp2);

  // $A%G$B!<$A%?$B=q$A$-$B9~$A$_$3$3$^$G(B
  if (count != points) {
    fprintf(stderr, "%s : data write error\n", wavfile);
    exit(1);
  }

  fclose(fp1);
  fclose(fp2);

  printf("  %ld points write\n", count);
  printf("%s : %ld bytes (%g sec)\n",
         wavfile, count * bytes * ch + 44, (double) count / sr);
}

/**************/
/* $A%a%$%s$B4X$AJ}(B */
/**************/
int main(int argc, char *argv[])
{
  int ac = 0, optc = 0;
  unsigned long sr = 0;
  char txtfile[_MAX_PATH], wavfile[_MAX_PATH];

  if (argc != 1) {
    int i;

    ac = argc - 1;
    for (i = 1; i < argc; i++) {
      // $A%5%s%W%j%s%0%l$B!<$A%HV86($"$j(B
      if (argv[i][0] == '-') {
        sr = labs(strtoul(&argv[i][1], (char **) NULL, 10));
        ac = argc - 2;
        optc = 1;
      }
    }
  }
  else
    usage();

  // $A%3%^%s%I%A%'%C%/(B($A%*%W%7%g%sRTMb$NR}J}(B)
  if (ac < 1) {
    usage();
  }
  else if (ac == 1) {
    char basename[_MAX_PATH];

    strcpy(txtfile, argv[optc + 1]);
    getbasename(basename, argv[optc + 1]);
    sprintf(wavfile, "%s.wav", basename);
  }
  else if (ac == 2) {
    strcpy(txtfile, argv[optc + 1]);
    strcpy(wavfile, argv[optc + 2]);
  }
  else {
    usage();
  }

  // $A%5%s%W%j%s%0%l$B!<$A%H(B
  if (sr == 0) {
    sr = sampling();
  }

  wavwrite(txtfile, wavfile, sr);

  return 0;
}
