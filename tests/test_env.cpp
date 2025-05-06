#include "../src/env/env.cpp"
#include <cassert>

void testFetchingHomeDirectory() {
  // Act
  char const *home_directory = fetchHomeDirectory();

  // Assert
  assert(home_directory != nullptr);
}

int main() { testFetchingHomeDirectory(); }
