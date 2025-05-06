
#include <stdexcept>

void process(int argc, char *argv[]);

class command_error : public std::runtime_error {
public:
  command_error(const char *message) : std::runtime_error(message) {}
};
