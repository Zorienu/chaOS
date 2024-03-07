#include <kernel/filesystem/File.h>
#include <kernel/filesystem/FileDescription.h>

FileDescription* File::open() {
  return FileDescription::create(*this);
}

void File::close() {

}
