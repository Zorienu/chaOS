#pragma once
#include "../fileSystem/File.h"

class Device : public File {
  public:
    virtual bool isDevice() const override { return true; }

  protected:
    Device() {};
};
