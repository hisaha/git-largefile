/* crypto/sha/sha1.c */
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>

#define SHA1_ASM
#include <openssl/sha.h>

#include <string>
#include <vector>
#include <iostream>
#include <memory>

#define BUFSIZE	(1024*64)

std::string ASSET_DIR = "";
std::string RSYNC_MOD = "";
std::string RSYNC_OPT = "";
std::string RSYNC = "";

struct contentchunk
{
void* data;
int bytes;
};

std::vector<struct contentchunk> content;
bool go=false;
int updated=0;

void do_fp(FILE *f);
void pt(unsigned char *md);
/* #ifndef _OSD_POSIX */
/* int read(int, void *, unsigned int); */
/* #endif */


#if defined(WIN32) || defined(_WIN32) 
#define PATH_SEPARATOR "\\" 
#else 
#define PATH_SEPARATOR "/" 
#endif 

std::string get_cache_path(std::string& hexdigest)
{
  std::string path="";
  
  path+=ASSET_DIR;
  path+=PATH_SEPARATOR;
  path+=hexdigest.substr(0,2);
  path+=PATH_SEPARATOR;
  path+=hexdigest.substr(2,2);
  path+=PATH_SEPARATOR;
  path+=hexdigest.substr(4);

  return path; 
}

int exists(const char *fname)
{
    FILE *file;
    if (file = fopen(fname, "r"))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

void *update_sha1(void *p) {
  SHA_CTX* c=(SHA_CTX*)p;
  
  while(go)
  {
    if(updated<content.size())
    {
        SHA1_Update(c,content[updated].data,content[updated].bytes);
        updated+=1;
    }
    else
    {
        usleep(1000);
    }
  }
}

std::string sha1(FILE *f)
{
  SHA_CTX c;
  unsigned char md[SHA_DIGEST_LENGTH];
  int i;
  unsigned char buf[BUFSIZE];

  SHA1_Init(&c);

  go=true;
  pthread_t pthread;
  pthread_create( &pthread, NULL, &update_sha1, &c);

  for (;;)
  {
    i=fread(buf,sizeof(unsigned char),BUFSIZE,f);
    if (i <= 0) break;
    
    unsigned char* tmp=new unsigned char[i];
    memcpy(tmp, buf, i);
    struct contentchunk chunk;
    chunk.data=tmp;
    chunk.bytes=i;
    content.push_back(chunk);
    //SHA1_Update(&c,buf,(unsigned long)i);
  }
  go=false;
  pthread_join(pthread, NULL);
  SHA1_Final(&(md[0]),&c);

  std::string ret="";
  char str[10];
  for (i=0; i<SHA_DIGEST_LENGTH; i++)
  {
    sprintf(str,"%02x",md[i]);
    ret+=str;
  }
  return ret;
}

#include "cmdline.h"

void store()
{
  /* std::string tmpfile=tmpnam (NULL); */
 
  std::string hexdigest=sha1(stdin);
  std::string cache_path = get_cache_path(hexdigest);

  if(!exists(cache_path.c_str()))
  {
    std::string d=dirname(const_cast<char*>(cache_path.c_str()));
    char cmd[1024];
    sprintf(cmd,"mkdir -p \"%s\" > /dev/null 2>&1",d.c_str());
    system(cmd);

    //FILE* o=fopen("wb");
    //for(int i=0;i<content.size();i++)
    //{
      
    //}


    /* sprintf(cmd,"mv \"%s\" \"%s\" > /dev/null 2>&1",tmpfile.c_str(),cache_path.c_str()); */
    /* system(cmd); */
  }

   //rsync(hexdigest);
   printf("%s\n",hexdigest.c_str());

}

int main(int argc, char *argv[])
{
  cmdline::parser p;
  p.add<std::string>("mode", 'm', "operation mode load|store", true);
  p.add<std::string>("assetdir", 'd', "path to asset directory", false, "~/.gitbig/default/");
  p.add<std::string>("rsyncmod", 'r', "path to rsync module like rsync://xxx.xxx.xxx.xxx/path/", true);
  p.add<std::string>("rsyncopt", 'o', "option to rsync", false, "avW");
  p.add<std::string>("rsync", 0, "path to rsync command", false, "rsync");
  p.add<std::string>("version", 0, "show version",false);

  p.add<std::string>("help", 0, "print help",false);

  if (!p.parse(argc, argv)||p.exist("help")){
    std::cout<<p.error_full()<<p.usage();
    return 0;
  }

  if (p.exist("assetdir"))
  {
    ASSET_DIR=p.get<std::string>("assetdir");
  }
  if (p.exist("rsyncmod"))
  {
    RSYNC_MOD=p.get<std::string>("rsyncmod");
  }
  if (p.exist("rsyncopt"))
  {
    RSYNC_OPT=p.get<std::string>("rsyncopt");
  }
  if (p.exist("rsync"))
  {
    RSYNC=p.get<std::string>("rsync");
  }


  if(p.get<std::string>("mode") == "store")
  {
    store();
  }
  else if(p.get<std::string>("mode") == "load")
  {
    //load();
  }

  return 0;
}
