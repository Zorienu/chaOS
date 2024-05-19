#pragma once
#include <stdint.h>


class Inode {
public:
  Inode();
  Inode(Inode &&) = default;
  Inode(const Inode &) = default;
  Inode &operator=(Inode &&) = default;
  Inode &operator=(const Inode &) = default;
  ~Inode();

private:
  
};

