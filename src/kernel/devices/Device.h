#include "../fileSystem/File.h"

class Device : public File {
  public:
    virtual bool isDevice() const { return true; }
};
