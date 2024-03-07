#include <kernel/filesystem/FileDescription.h>
#include <kernel/filesystem/File.h>

FileDescription* FileDescription::create(File& file) {
  return new FileDescription(file);
}

FileDescription::FileDescription(File& file) : _file(file) {};
