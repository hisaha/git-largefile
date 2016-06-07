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

#define BUFSIZE	(1024*16)

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
  
  //path+=ASSET_DIR;
  //path+=PATH_SEPARATOR;
  path+=hexdigest.substr(0,2);
  if(path.rfind(PATH_SEPARATOR)!=path.size()-1)
    path+=PATH_SEPARATOR;
  path+=hexdigest.substr(2,2);
  if(path.rfind(PATH_SEPARATOR)!=path.size()-1)
    path+=PATH_SEPARATOR;
  path+=hexdigest.substr(4);

  return path; 
}

std::string rstrip(std::string& p1, std::string& p2)
{
  std::string path=p1;
  
  if(path.rfind(PATH_SEPARATOR)==path.size()-1)
    return path.substr(0,path.size()-1);
  
  return path; 
}

std::string join(std::string& p1, std::string& p2)
{
  std::string path=p1;
  
  if(path.rfind(PATH_SEPARATOR)!=path.size()-1)
    path+=PATH_SEPARATOR;
  
  path+=p2;  
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
  
  while(updated<content.size())
  {
    SHA1_Update(c,content[updated].data,content[updated].bytes);
    updated+=1;
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

void cat(std::string& fname)
{
  int i;
  unsigned char buf[BUFSIZE];
  FILE* f=fopen(fname.c_str(),"rb");
  FILE* o=fdopen(fileno(stdout),"wb");
  setvbuf(o,NULL,_IOFBF,1024*1024*32);
  for (;;)
  {
    i=fread(buf,sizeof(unsigned char),BUFSIZE,f);
    if (i <= 0) break;
    fwrite(buf,sizeof(unsigned char),i,o);
  }
  fclose(f);
}


#include "cmdline.h"

void store()
{
  std::string hexdigest=sha1(stdin);
  std::string cache_path = get_cache_path(hexdigest);
  std::string cache_dir_path=dirname(const_cast<char*>(cache_path.c_str()));
  std::string local_path = join(ASSET_DIR,cache_path);

  if(exists(local_path.c_str())==0)
  {
    std::string d=dirname(const_cast<char*>(local_path.c_str()));
    char cmd[1024];
    sprintf(cmd,"mkdir -p \"%s\" > /dev/null 2>&1",d.c_str());
    system(cmd);

    //printf("writing\n");
    FILE* o=fopen(local_path.c_str(),"wb");
    setvbuf(o,NULL,_IOFBF,1024*1024*32);
    for(int i=0;i<content.size();i++)
    {
      fwrite(content[i].data,1,content[i].bytes,o);
    }
    fclose(o);
    //printf("wrote\n");
  }

   char cmd[4096];
   sprintf(cmd,"%s -%s \"%s\" \"%s\"  > /dev/null 2>&1",RSYNC.c_str(),RSYNC_OPT.c_str(),local_path.c_str(),join(RSYNC_MOD,cache_dir_path).c_str());
   //system(cmd);

   //printf(cmd);

   printf("%s",hexdigest.c_str());
}

void load()
{
  int i=0;
  char buf[1024];
  std::vector<char> contents;
  for (;;)
  {
    i=fread(buf,sizeof(char),1024,stdin);
    if (i <= 0) break;
    for(int j=0;j<i;j++)
        contents.push_back(buf[j]);  
  }

  contents[40]='\0';
  std::string hexdigest = &contents[0];
  
  std::string cache_path = get_cache_path(hexdigest);
  std::string cache_dir_path=dirname(const_cast<char*>(cache_path.c_str()));
  std::string local_path = join(ASSET_DIR,cache_path);

  //call rsync
  char cmd[4096];
  sprintf(cmd,"%s -%s \"%s\" \"%s\"  > /dev/null 2>&1",RSYNC.c_str(),RSYNC_OPT.c_str(),join(RSYNC_MOD,cache_path).c_str(),join(ASSET_DIR,cache_dir_path).c_str());
  system(cmd);
  //printf(cmd);

  cat(local_path);
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

  ASSET_DIR=p.get<std::string>("assetdir");
  RSYNC_MOD=p.get<std::string>("rsyncmod");
  RSYNC_OPT=p.get<std::string>("rsyncopt");
  RSYNC=p.get<std::string>("rsync");

  if(p.get<std::string>("mode") == "store")
  {
    store();
  }
  else if(p.get<std::string>("mode") == "load")
  {
    load();
  }

  return 0;
}
