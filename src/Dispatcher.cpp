// Internal
#include "Dispatcher.hpp"


void Dispatcher::subscribe(DescriptorType descriptor, SlotType slot) {
  mObservers[descriptor].push_back(std::move(slot));
}

void Dispatcher::post(const IEvent &event) const {
  const auto type = event.type();

  // Ignore events for which we do not have an observer (yet).
  if(mObservers.find(type) == mObservers.end()) {
    return;
  }

  const auto &observers = mObservers.at(type);

  for(auto &&observer : observers) {
    observer(event);
  }
}
