#pragma once

namespace ORB_SLAM2 {

class System;

struct ISaveTrajectory {

virtual ~ISaveTrajectory() noexcept = default;

[[nodiscard]] virtual bool save(std::ofstream &outputStream, const System &system) const = 0;

[[nodiscard]] virtual bool saveKeyFrame(std::ofstream &outputStream, const System &system) const = 0;

};

} // namespace ORB_SLAM2

