#include "Spy.hh"


int main(int argc, char **argv) {
  Spy::init(argc, argv);

  Spy::getSpy()->run();
}
