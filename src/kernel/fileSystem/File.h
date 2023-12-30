#pragma once
#include <stddef.h>
#include <stdint.h>

class FileDescription;

class File {
  public:
    virtual FileDescription* open(int options);
    virtual void close();

    virtual bool canRead (FileDescription&) const = 0;
    virtual bool canWrite (FileDescription&) const = 0;

    // TODO: use ssize_t instead
    virtual size_t read(FileDescription&, uint8_t*, size_t) const = 0;
    virtual size_t write(FileDescription&, const uint8_t*, size_t) const = 0;

    virtual bool isInode() const { return false; }
    virtual bool isDevice() const { return false; }
    virtual bool isTTY() const { return false; }
    virtual bool isMasterPty() const { return false; }
    virtual bool isCharacterDevice() const { return false; }

  protected:
    File() {};
};
