// STL
#include <functional>
#include <unordered_map>
// Internal
#include "IEvent.hpp"


class Dispatcher final {
public:
  using SlotType = std::function<void(const IEvent&)>;

  void subscribe(DescriptorType descriptor, SlotType slot);

  void post(const IEvent& event) const;

private:
  std::unordered_map<DescriptorType, std::vector<SlotType>> mObservers;

};

