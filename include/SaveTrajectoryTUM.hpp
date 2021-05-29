// STL
#include "ISaveTrajectory.hpp"

namespace ORB_SLAM2 {

struct SaveTrajectoryTUM final : ISaveTrajectory {

  SaveTrajectoryTUM() noexcept;

  ~SaveTrajectoryTUM() noexcept override;

  [[nodiscard]] bool save(std::ofstream &outputStream, const System &system) const override;

  [[nodiscard]] bool saveKeyFrame(std::ofstream &outputStream, const System &system) const override;

};

}  // namespace ORB_SLAM2
