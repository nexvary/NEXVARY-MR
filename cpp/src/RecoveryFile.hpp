#pragma once
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUrl>
#include <sys/stat.h>
#ifdef Q_OS_WIN
#include <io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif
namespace mr::recovery {
// Shared path policy for local evidence/firmware inputs and explicit exports.
// A regular path may still reside on an OS-mounted network filesystem.
inline QString localFilePath(const QString &value) {
 if(value.isEmpty()||value.startsWith("\\\\")||value.startsWith("//"))return {};
 const auto safe=[](const QString &s){for(const auto c:s)if(c.unicode()<32||c.unicode()==127)return false;return true;};
 if(!safe(value))return {};
 const QUrl url(value);
 const auto path=url.isLocalFile()?url.toLocalFile():(url.scheme().isEmpty()||QFileInfo(value).isAbsolute()?value:QString());
 if(path.isEmpty()||!safe(path)||path.startsWith("\\\\")||path.startsWith("//"))return {};
#ifdef Q_OS_WIN
 if(path.mid(2).contains(QLatin1Char(':')))return {};
 static const QRegularExpression reserved(QStringLiteral("^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\\..*)?$"),QRegularExpression::CaseInsensitiveOption);
 if(reserved.match(QFileInfo(path).fileName()).hasMatch())return {};
#endif
 return QFileInfo(path).absoluteFilePath();
}

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

