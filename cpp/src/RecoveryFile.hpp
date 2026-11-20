#pragma once
#include <QFile>
#include <sys/stat.h>
#ifdef Q_OS_WIN
#include <io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif
namespace mr::recovery {
// Verify the opened descriptor as well as the path. POSIX nonblocking/no-follow
// flags prevent a path replaced by a FIFO/symlink from trapping a worker in open().
inline bool openRegularInput(QFile &file) {
#ifdef Q_OS_WIN
 if(!file.open(QIODevice::ReadOnly))return false;
 struct _stat64 info{};
 if(_fstat64(file.handle(),&info)!=0||(info.st_mode&_S_IFMT)!=_S_IFREG){file.close();return false;}
 return true;
#else
 const auto path=QFile::encodeName(file.fileName());
 const int descriptor=::open(path.constData(),O_RDONLY|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK);
 if(descriptor<0)return false;
 struct stat info{};
 if(::fstat(descriptor,&info)!=0||!S_ISREG(info.st_mode)){::close(descriptor);return false;}
 if(!file.open(descriptor,QIODevice::ReadOnly,QFileDevice::AutoCloseHandle)){::close(descriptor);return false;}
 return true;
#endif
}
}
