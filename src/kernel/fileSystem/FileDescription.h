#include "File.h"
#include <stddef.h>

/*
 * Wrapper for the File class
 */
class FileDescription {
  public:
    static FileDescription* create(File&);

    int close();
    
    // TODO: use ssize_t instead
    size_t read(uint8_t*, size_t);
    size_t write(const uint8_t*, size_t);

    bool canRead() const;
    bool canWrite() const;


  private: 
     File *_file;
     int _currentOffset { 0 };
};
