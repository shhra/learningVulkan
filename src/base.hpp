#ifndef BASE_H_
#define BASE_H_

#include <vulkan/vulkan.h>

// Report and handle the error
#include <iostream>
#include <stdexcept>

// Get the EXIT_SUCCESS and EXIT_FAILURE macros
#include <cstdlib>

// This is a typical vulkan application where vulkan objects are stored.
// The code is straight forward that provides function to load the bare
// vulkan objects.
class VkApp {

public:
  void run() {
    init();
    loop();
    clean();
  }

private:
  // Allocate resources
  void init() {}

  // Update the graphical elements.
  void loop() {}

  // Free the allocated resources
  void clean() {}
};

#endif // BASE_H_
