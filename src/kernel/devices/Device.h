#pragma once
#include <kernel/fileSystem/File.h>

class Device : public File {
  public:
    virtual bool isDevice() const override final { return true; }

  protected:
    Device() {};
};
