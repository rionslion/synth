/******************************************************************/
/* txt2wav �e�L�X�g�f�[�^ -> wav�t�H�[�}�b�g�f�[�^ �ϊ��v���O���� */
/*                                                                */
/* �f�[�^ 16bit(-2^15 �` 2^15 - 1, -32768 �` +32767, ���� = 0)    */
/* �f�[�^  8bit(    0 �` 2^8  - 1 ,     0 �` +225  , ���� = 128)  */
/*                                                                */
/* ���m����(1ch)    �X�e���I(2ch)                                 */
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
          "ASCII �e�L�X�g�f�[�^ ---> wav �t�H�[�}�b�g�ϊ��v���O����...\n"
          "�g���� : %s [sampling_rate] <text_file> [wav_file]\n"
          "\t sampling_rate �� "
          "- �ɑ����ăT���v�����O���[�g(�P�� Hz)���w�肵�܂�\n"
          "\t (wav_file �̎w�肪�����ꍇ "
          "text_file.wav �̃t�@�C�����쐬����܂�)\n"
          "\n"
          "�� : %s -44100 filename.txt\n"
          "     (filename.txt �̃f�[�^�� 44.1kHz �� "
          "filename.wav �ɏ����o���܂�)\n"
          "     %s -48000 filename.txt output.wav\n"
          "     (filename.txt �̃f�[�^�� 48kHz �� "
          "output.wav �ɏ����o���܂�)\n"
          "\n"
          "�f�[�^�� 0�`255 �Ȃ�� 8bit wav ���o�͂��܂�\n"
          "����ȊO�Ȃ�� 16bit wav ���o�͂��܂�\n"
          "( 8bit �f�[�^ :      0�`  +255, ����=128)\n"
          "(16bit �f�[�^ : -32768�`+32767, ����=  0)\n"
          "\n"
          "�e�L�X�g�f�[�^�`�� : \n"
          "\t1ch(���m����)�̏ꍇ    2ch(�X�e���I)�̏ꍇ\n"
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
/* �t�@�C�����擾 */
/*----------------*/
int getbasename(char *dest, char *src)
{
  int i, start, end, ret;

  i = -1;
  start = 0;
  end = 0;

  // �t�@�C�����̂͂��߂ƏI�������o
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

  // �t�@�C�������L��ꍇ
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
/* ASCII �t�@�C���̍s�� */
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
/* �ʎq���r�b�g���m�F */
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

  // �f�[�^�� 0�`255 �Ȃ�� 1byte(8bit)
  return 1;
}

/*--------------*/
/* �`�����l���� */
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
/* �f�[�^�I�[�o�[�t���[�`�F�b�N */
/*------------------------------*/
short datacheck(short data, short max, short min)
{
  // �I�[�o�[�t���[����ꍇ + �� - ��\��
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
/* 16 bit �f�[�^ -32768�`32767 (���� 0) */
/* 8 bit �f�[�^ 0�`255 (���� 128)       */
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
    // ���m����
    else if (ch == 1) {
      // 8 �r�b�g
      if (byte == 1) {
        datacheck(left, UCHAR_MAX, UCHAR_MIN);
        fputc(left, wav);
        count++;
      }
      // 16 �r�b�g
      else if (byte == 2) {
        datacheck(left, SHRT_MAX, SHRT_MIN);
        fwrite(&left, sizeof(short), 1, wav);
        count++;
      }
    }
    // �X�e���I
    else if (ch == 2) {
      // 8 �r�b�g
      if (byte == 1) {
        datacheck(left, UCHAR_MAX, UCHAR_MIN);
        datacheck(right, UCHAR_MAX, UCHAR_MIN);
        fputc(left, wav);
        fputc(right, wav);
        count++;
      }
      // 16 �r�b�g
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
/* �T���v�����O���[�g */
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
/* �e��p�����[�^ */
/*----------------*/
unsigned long parameter(FILE *fp,
                        unsigned long *points,
                        unsigned short *channel,
                        unsigned short *byte)
{
  // �f�[�^��
  *points = filesize(fp);

  // �ʎq���o�C�g�� 1Byte = 8bit, 2Byte = 16bit
  *byte = bytecheck(fp);

  // �`�����l����
  *channel = chcheck(fp);

  return (unsigned long) (*points * *byte * *channel);
}

/*----------------------*/
/* wav �t�@�C�������o�� */
/*----------------------*/
void wavwrite(char *datafile, char *wavfile, unsigned long sr)
{
  unsigned long points, file_size, count, var_long;
  unsigned short ch, bytes, var_short;
  char s[4];
  FILE *fp1, *fp2;

  // �e�L�X�g�t�@�C��
  if ((fp1 = fopen(datafile, "r")) == NULL) {
    openerror(datafile);
  }

  file_size = parameter(fp1, &points, &ch, &bytes);
  printf("%s : %ld Hz sampling, %d bit, %d channel\n",
         wavfile, sr, bytes * 8, ch);

  // WAV �t�@�C��
  if ((fp2 = fopen(wavfile, "wb")) == NULL) {
    openerror(wavfile);
  }

  // RIFF �w�b�_
  s[0] = 'R';
  s[1] = 'I';
  s[2] = 'F';
  s[3] = 'F';
  fwrite(s, 1, 4, fp2);
  // �t�@�C���T�C�Y
  var_long = file_size + 36;
  fwrite(&var_long, sizeof(long), 1, fp2);
  printf("  file size (header + data) = %ld [Byte]\n", var_long);

  // WAVE �w�b�_
  s[0] = 'W';
  s[1] = 'A';
  s[2] = 'V';
  s[3] = 'E';
  fwrite(s, 1, 4, fp2);

  // chunkID (fmt �`�����N)
  s[0] = 'f';
  s[1] = 'm';
  s[2] = 't';
  s[3] = ' ';
  fwrite(s, 1, 4, fp2);
  // chunkSize (fmt �`�����N�̃o�C�g�� �����k wav �� 16)
  var_long = 16;
  fwrite(&var_long, sizeof(long), 1, fp2);
  // wFromatTag (�����k PCM = 1)
  var_short = PCM;
  fwrite(&var_short, sizeof(short), 1, fp2);
  // dwChannels (���m���� = 1, �X�e���I = 2)
  fwrite(&ch, 2, 1, fp2);
  printf("  PCM type                  = %hu\n", var_short);
  // dwSamplesPerSec (�T���v�����O���[�g(Hz))
  fwrite(&sr, sizeof(long), 1, fp2);
  printf("  sampling rate             = %ld [Hz]\n", sr);
  // wdAvgBytesPerSec (Byte/�b)
  var_long = bytes * ch * sr;
  fwrite(&var_long, sizeof(long), 1, fp2);
  printf("  Byte / second             = %ld [Byte]\n", var_long);
  // wBlockAlign (Byte/�T���v��*�`�����l��)
  var_short = bytes * ch;
  fwrite(&var_short, sizeof(short), 1, fp2);
  printf("  Byte / block              = %hu [Byte]\n", var_short);
  // wBitsPerSample (bit/�T���v��)
  var_short = bytes * 8;
  fwrite(&var_short, sizeof(short), 1, fp2);
  printf("  bit / sample              = %hu [bit]\n", var_short);

  // chunkID (data �`�����N)
  s[0] = 'd';
  s[1] = 'a';
  s[2] = 't';
  s[3] = 'a';
  fwrite(s, 1, 4, fp2);
  // chunkSize (�f�[�^�� Byte)
  fwrite(&file_size, 4, 1, fp2);
  printf("  file size (data)          = %ld [Byte]\n", file_size);

  // �w�b�_ (44 Byte) �������݂����܂�
  if (ftell(fp2) != 44) {
    fprintf(stderr, "%s : wav header write error\n", wavfile);
    exit(1);
  }

  // waveformData (�f�[�^) ��������
  count = datawrite(ch, bytes, fp1, fp2);

  // �f�[�^�������݂����܂�
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
/* ���C���֐� */
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
      // �T���v�����O���[�g�w�肠��
      if (argv[i][0] == '-') {
        sr = labs(strtoul(&argv[i][1], (char **) NULL, 10));
        ac = argc - 2;
        optc = 1;
      }
    }
  }
  else
    usage();

  // �R�}���h�`�F�b�N(�I�v�V�����ȊO�̈���)
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

  // �T���v�����O���[�g
  if (sr == 0) {
    sr = sampling();
  }

  wavwrite(txtfile, wavfile, sr);

  return 0;
}
