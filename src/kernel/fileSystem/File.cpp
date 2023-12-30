#include "File.h"
#include "FileDescription.h"

FileDescription* File::open() {
  return FileDescription::create(*this);
}

void File::close() {

}
