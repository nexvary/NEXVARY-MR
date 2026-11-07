// Linux product worker supervisor. No Qt, shell or privilege changes.
// A dedicated subreaper survives product crash to clean its owned descendants.
// This is not protection from a hostile same-user worker killing the supervisor.
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <poll.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {
volatile sig_atomic_t stopping=0;
void stopSignal(int){stopping=1;}
using Clock=std::chrono::steady_clock;
std::vector<long> namespaceIds(const std::string &path,long *parent=nullptr,long expectedParent=0){
 std::ifstream status(path);std::string line;std::vector<long> ids;
 while(std::getline(status,line)){
  if(parent&&line.rfind("PPid:",0)==0){std::istringstream value(line.substr(5));value>>*parent;if(expectedParent&&*parent!=expectedParent)return {};}
  if(line.rfind("NSpid:",0)==0){std::istringstream values(line.substr(6));long id=0;while(values>>id){if(id<=0||id>2147483647L||ids.size()>=32)return {};ids.push_back(id);}}
 }
 return ids;
}
std::vector<pid_t> children(bool &valid,const std::vector<long> &self){
 // procfs can be mounted from an ancestor PID namespace. Never signal its raw
 // identifiers. Translate NSpid and verify direct-parent ownership each batch.
 std::ifstream input("/proc/self/task/"+std::to_string(self.front())+"/children");
 valid=true;std::vector<pid_t> result;long id=0;
 const auto add=[&](long candidate,bool mustBeOwned){
  long parent=0;const auto ids=namespaceIds("/proc/"+std::to_string(candidate)+"/status",&parent,self.front());
  if(parent!=self.front()){if(mustBeOwned)valid=false;return;}
  if(ids.size()<self.size()||result.size()>=4096){valid=false;return;}
  result.push_back(static_cast<pid_t>(ids[self.size()-1]));
 };
 if(!input.is_open()){
  // Some kernels omit task/children. Stream bounded PID metadata, filter PPid
  // before retaining namespace IDs; signal no unrelated enumerated process.
  DIR *proc=opendir("/proc");if(!proc){valid=false;return result;}
  unsigned scanned=0;
  while(auto *entry=readdir(proc)){
   char *end=nullptr;const long candidate=std::strtol(entry->d_name,&end,10);
   if(!end||*end||candidate<=0||candidate>2147483647L)continue;
   if(++scanned>65536){valid=false;break;}
   add(candidate,false);
  }
  closedir(proc);return result;
 }
 while(input>>id){
  if(id<=0||id>2147483647L||result.size()>=4096){valid=false;break;}
  add(id,true);
 }
 if(!input.eof())valid=false;
 return result;
}
bool receipt(const char *path,bool complete,unsigned observed,unsigned reaped,unsigned refused,bool parentLost,int rootCode){
 const int fd=open(path,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC|O_NOFOLLOW,0600);if(fd<0)return false;
 char data[600];const int size=std::snprintf(data,sizeof data,"{\"schema\":\"nexvary-process-cleanup/v1\",\"mechanism\":\"linux-subreaper-groups\",\"cleanupComplete\":%s,\"observedAtShutdown\":%u,\"reaped\":%u,\"signalFailures\":%u,\"parentPipeLost\":%s,\"rootExitCode\":%d,\"cgroupUsed\":false}\n",complete?"true":"false",observed,reaped,refused,parentLost?"true":"false",rootCode);
 const bool ok=size>0&&static_cast<size_t>(size)<sizeof data&&write(fd,data,static_cast<size_t>(size))==size&&fsync(fd)==0;close(fd);return ok;
}
}
int main(int argc,char **argv){
 if(argc<4||std::string(argv[2])!="--")return 125;
 const auto self=namespaceIds("/proc/self/status");
 if(self.empty()||self.back()!=getpid())return 125;
 if(prctl(PR_SET_CHILD_SUBREAPER,1,0,0,0)!=0)return 125;
 struct sigaction action{};action.sa_handler=stopSignal;sigemptyset(&action.sa_mask);
 if(sigaction(SIGTERM,&action,nullptr)!=0||sigaction(SIGINT,&action,nullptr)!=0)return 125;
 struct sigaction reap{};reap.sa_handler=SIG_DFL;sigemptyset(&reap.sa_mask);
 if(sigaction(SIGCHLD,&reap,nullptr)!=0)return 125;
 pollfd beforeLaunch{STDIN_FILENO,POLLIN|POLLHUP,0};
 if(poll(&beforeLaunch,1,0)>0&&beforeLaunch.revents)return receipt(argv[1],true,0,0,0,true,125)?125:126;
 const auto supervisor=getpid();
 const pid_t root=fork();if(root<0)return 125;
 if(root==0){
  if(setpgid(0,0)!=0||prctl(PR_SET_PDEATHSIG,SIGKILL,0,0,0)!=0||getppid()!=supervisor)_exit(125);
  struct sigaction defaults{};defaults.sa_handler=SIG_DFL;sigemptyset(&defaults.sa_mask);
  sigaction(SIGTERM,&defaults,nullptr);sigaction(SIGINT,&defaults,nullptr);
  const int nullInput=open("/dev/null",O_RDONLY|O_CLOEXEC);if(nullInput<0||dup2(nullInput,STDIN_FILENO)<0)_exit(125);if(nullInput!=STDIN_FILENO)close(nullInput);
  execvp(argv[3],argv+3);_exit(127);
 }
 bool rootDone=false,parentLost=false,cleaning=false,complete=false,valid=true;
 int rootCode=125;unsigned observed=0,reaped=0,refused=0;
 auto deadline=Clock::time_point::max();
 for(;;){
  if(!cleaning){
   pollfd control{STDIN_FILENO,POLLIN|POLLHUP,0};const int polled=poll(&control,1,10);
   if(polled>0&&(control.revents&(POLLIN|POLLHUP|POLLERR|POLLNVAL))){parentLost=true;stopping=1;}
   if(polled<0&&errno!=EINTR){stopping=1;valid=false;}
  }
  // Single-threaded: never reap between inventory/ownership check and kill.
  int status=0;pid_t done=0;
  while((done=waitpid(-1,&status,WNOHANG))>0){++reaped;if(done==root){rootDone=true;rootCode=WIFEXITED(status)?WEXITSTATUS(status):128+WTERMSIG(status);}}
  const bool noChildren=done<0&&errno==ECHILD;
  if(done<0&&!noChildren&&errno!=EINTR)valid=false;
  if(stopping||rootDone){
   if(!cleaning){cleaning=true;deadline=Clock::now()+std::chrono::milliseconds(2000);}
   bool inventory=true;const auto owned=children(inventory,self);valid=valid&&inventory;
   if(owned.size()>observed)observed=static_cast<unsigned>(owned.size());
   if(owned.empty()&&noChildren){complete=valid;break;}
   for(const auto id:owned)if(kill(id,SIGSTOP)!=0&&errno!=ESRCH)++refused;
   if(!rootDone&&kill(-root,SIGKILL)!=0&&errno!=ESRCH)++refused;
   for(const auto id:owned)if(kill(id,SIGKILL)!=0&&errno!=ESRCH)++refused;
   if(Clock::now()>=deadline)break;
   poll(nullptr,0,10);
  }
 }
 if(!receipt(argv[1],complete,observed,reaped,refused,parentLost,rootCode))return 125;
 return complete?rootCode:125;
}
