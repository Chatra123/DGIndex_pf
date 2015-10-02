#include "global.h"


//
//�p�C�v�����̏�����
//parse_cli��������������Ɏ��s����B
//
int Initialize_pf()
{
  //initialize
  IsClosed_stdin = true;
  GetExtraData_fromStdin = false;

  if (Mode_Stdin && Initialize_stdin() == 1)
  {
    //�G���[
    remove(StdinHeadFile_Path);
    return 1;                            //�v���Z�X�I��
  }

  timeFlushD2VFile = time(NULL);
  tickReadSize_speedlimit = 0;
  tickBeginTime_speedlimit = system_clock::now();
  SpeedLimit = SpeedLimit_CmdLine * 1024 * 1024;

  //�f�o�b�O
  Enable_pfLog = false;    //�@true  false

  pf_gop_Log[0] = '\0';

  min_gop_idx = INT32_MAX;
  max_gop_idx = INT32_MIN;

  return 0;
}


//
//Initialize_stdin
//
int Initialize_stdin()
{
  // -i "filepath"�@-pipe �@�����Ɏw�肳�ꂽ��v���Z�X�I���B
  if (NumLoadedFiles != 1) return 1;
  NumLoadedFiles = 0;

  //
  //Input
  //
  _setmode(_fileno(stdin), _O_BINARY);
  fdStdin = _fileno(stdin);

  //
  //StdinHeadFile
  //  �W�����͂̐擪�����t�@�C���ɏ����o���A
  //  �t�@�C���ɂ��邱�Ƃ�seek�ɑΉ�����B
  //size
  double gt_10MB = StdinHeadFile_Size_CmdLine;
  gt_10MB = (10 < gt_10MB) ? gt_10MB : 10;                 //greater than 10 MiB
  StdinHeadFile_Size = (int)(gt_10MB * 1024 * 1024);

  //buff
  char *stdinHeadBuff = new char[StdinHeadFile_Size];      //�擪�����ۑ��p�̃o�b�t�@
  memset(stdinHeadBuff, '\0', StdinHeadFile_Size);

  //
  //Fill stdinHeadBuff
  //  �W�����͂���f�[�^���o��
  //
  int curBuffSize = 0;                       //�Ǎ��σT�C�Y

  IsClosed_stdin = true;                     //�^�C���A�E�g����p
  time_t timeReadPipe_begin = time(NULL);    //�^�C���A�E�g����p

  while (curBuffSize < StdinHeadFile_Size)
  {
    int tickDemandSize = StdinHeadFile_Size - curBuffSize;     //�v���T�C�Y
    int readsize = _read(fdStdin, stdinHeadBuff + curBuffSize, tickDemandSize);

    if (readsize == -1)                                        //fail to connect
    {
      //�W�����͂���f�[�^������܂őҋ@�A�R�O�O�b�o�߂ŏI��
      //��x�ڑ����m�F������^�C���A�E�g�͂��Ȃ�
      if (IsClosed_stdin && time(NULL) - timeReadPipe_begin < 300)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        continue;
      }
      else
        return 1;					                                 //time out.
    }
    else if (readsize == 0)
      return 1;		                                         //end of stream. too small source.

    curBuffSize += readsize;
    IsClosed_stdin = false;
  }

  if (curBuffSize == 0) return 1;		                       //fail to read stdin. exit.

  StdinHeadFile_Size = curBuffSize;


  //
  //windows��temp�t�H���_��DGI_pf.tmp���쐬���A�擪�����t�@�C���ɕۑ�����B
  //  fd�ŊJ���ƏI�����ɍ폜�ł��Ȃ������BFILE*�ŏ�������ŕ���B
  //

  //����
  //  file pointer
  FILE *pf_tmp;

  //tmp name
  StdinHeadFile_Path = _tempnam(NULL, "DGI_pf.tmp");
  if (StdinHeadFile_Path == NULL)
    return 1;

  pf_tmp = fopen(StdinHeadFile_Path, "wb");
  if (pf_tmp == NULL)
    return 1;

  //write data
  size_t canwrite = fwrite(stdinHeadBuff, StdinHeadFile_Size, 1, pf_tmp);
  if (canwrite == 0)
    return 1;
  fclose(pf_tmp);

  //�Ǎ�
  //  file descriptor�ōăI�[�v��
  fdStdinHeadFile = _open(StdinHeadFile_Path, _O_RDONLY | _O_BINARY);
  if (fdStdinHeadFile == -1)
    return 1;

  strcpy(Infilename[NumLoadedFiles], StdinHeadFile_Path);          //d2v�t�@�C���R�s�ڂ̃t�@�C����
  Infile[NumLoadedFiles] = fdStdinHeadFile;                        //���̓t�@�C����StdinStreamFile��
  NumLoadedFiles = 1;

  //release buff
  if (stdinHeadBuff != NULL)
  {
    delete[] stdinHeadBuff;
    stdinHeadBuff = NULL;
  }

  return 0;
}





