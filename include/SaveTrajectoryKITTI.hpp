// STL
#include "ISaveTrajectory.hpp"

namespace ORB_SLAM2 {

struct SaveTrajectoryKITTI final : ISaveTrajectory {
  SaveTrajectoryKITTI() noexcept;

  ~SaveTrajectoryKITTI() noexcept override;

  [[nodiscard]] bool save(std::ofstream &inputStream, const System& system) const override;

  [[nodiscard]] bool saveKeyFrame(std::ofstream &inputStream, const System& system) const override;

};

}  // namespace ORB_SLAM2

