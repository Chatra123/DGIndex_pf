#include "global.h"


//
//�f�o�b�O�p  ����
// 
//  -i "E:\n3s.ts" -o "E:\n3s.ts" -ia 4 -fo 0 -yr 2 -om 2 -nodialog -limit 10.0
//
//


//
//�p�C�v�����̏�����
//parse_cli��������������Ɏ��s���邱�ƁB
//
int Initialize_pf()
{
  //initialize
  IsClosed_stdin = true;
  GetExtraData_fromStdin = false;

  if (Mode_Stdin &&
    Initialize_stdin() == 1)
  {
    //�G���[
    remove(StdinHeadFile_Path);
    return 1;                            //�v���Z�X�I��
  }

  timeFlushD2VFile = time(NULL);
  tickReadSize_speedlimit = 0;
  tickBeginTime_speedlimit = system_clock::now();
  SpeedLimit = SpeedLimit_CmdLine * 1024 * 1024;    // Byte/sec  <--  MiB/sec


  //�f�o�b�O�p�̃��O
  {
    Enable_pfLog = false;    //�@true  false

#ifdef _DEBUG
    Enable_pfLog = true;
    Logger_Initilaize();

    char log[256] = "";
    sprintf(log, "%s Mode_NoDialog = %d", log, Mode_NoDialog);
    Logging_ts(log);
#endif
  }

  return 0;
}


//
//Initialize_stdin
//
int Initialize_stdin()
{
  // -i "filepath"�@-pipe �@�����Ɏw�肳�ꂽ��v���Z�X�I���B
  if (NumLoadedFiles != 1) return 1;

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
  double filesize = StdinHeadFile_Size_CmdLine;
  filesize = (10 < filesize) ? filesize : 10;              //greater than 10 MiB
  StdinHeadFile_Size = (int)(filesize * 1024 * 1024);

  //buff
  char *stdinHeadBuff = new char[StdinHeadFile_Size];      //�擪�����ۑ��p�̃o�b�t�@
  memset(stdinHeadBuff, '\0', StdinHeadFile_Size);

  //
  //Fill stdinHeadBuff
  //  �W�����͂���f�[�^���o��
  //
  int curBuffSize = 0;                                     //�Ǎ��σT�C�Y

  while (curBuffSize < StdinHeadFile_Size)
  {
    int tickDemandSize = StdinHeadFile_Size - curBuffSize; //�v���T�C�Y
    int readsize = _read(fdStdin, stdinHeadBuff + curBuffSize, tickDemandSize);

    if (readsize == -1)                                    //fail to connect
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      continue;
    }
    else if (readsize == 0)
      return 1;		                                         //end of stream. too small source.

    curBuffSize += readsize;
  }

  IsClosed_stdin = false;
  if (curBuffSize == 0) return 1;		                       //fail to read stdin. exit.



  //
  //windows��temp�t�H���_��DGI_pf_tmp_00000_1���쐬���A�X�g���[���擪�����t�@�C���ɕۑ�����B
  //  fd�ŊJ���ƏI�����ɍ폜�ł��Ȃ������BFILE*�ŏ�������ŕ���B
  //

  //����
  //  file pointer
  FILE *pf_tmp;

  //tmp file name
  DWORD pid = GetCurrentProcessId();
  int rnd = rand() % (1000 * 1000);
  std::string basename = "DGI_pf_tmp_" + std::to_string(pid) + std::to_string(rnd) + "_";
  StdinHeadFile_Path = _tempnam(NULL, basename.c_str());
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

  //Infile��StdinStreamFile���Z�b�g
  strcpy(Infilename[0], StdinHeadFile_Path);
  Infile[0] = fdStdinHeadFile;

  //release buff
  if (stdinHeadBuff != NULL)
  {
    delete[] stdinHeadBuff;
    stdinHeadBuff = NULL;
  }

  return 0;
}





