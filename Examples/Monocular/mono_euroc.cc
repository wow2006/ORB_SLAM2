/**
 * This file is part of ORB-SLAM2.
 *
 * Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
 * For more information see <https://github.com/raulmur/ORB_SLAM2>
 *
 * ORB-SLAM2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ORB-SLAM2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
 */
// Internal
#include <System.hpp>

using namespace std;

static bool LoadImages(const string &strImagePath, const string &strPathTimes, vector<string> &vstrImages, vector<double> &vTimeStamps) {
  std::ifstream fTimes(strPathTimes);
  if(!fTimes.is_open()) {
    spdlog::error("Can not open \"{}\"", strPathTimes);
    return false;
  }

  vTimeStamps.reserve(5000);
  vstrImages.reserve(5000);

  while(!fTimes.eof()) {
    std::string line;
    std::getline(fTimes, line);

    if(!line.empty()) {
      std::stringstream ss;
      ss << line;
      vstrImages.push_back(strImagePath + "/" + ss.str() + ".png");
      double t;
      ss >> t;
      vTimeStamps.push_back(t / 1e9);
    }
  }
  return true;
}

static void createLogger() {
  spdlog::sink_ptr pConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  pConsoleSink->set_level(spdlog::level::trace);

  spdlog::sink_ptr pFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/mono_euroc.txt", true);
  pFileSink->set_level(spdlog::level::trace);

  spdlog::sinks_init_list pSinkList = { pConsoleSink, pFileSink };

  auto pLogger = std::make_shared<spdlog::logger>("multi_sink", pSinkList);
  pLogger->set_level(spdlog::level::trace);

  spdlog::set_default_logger(pLogger);
}

int main(int argc, char **argv) {
  if(argc != 5) {
    std::cerr << "\nUsage: ./mono_tum path_to_vocabulary path_to_settings path_to_image_folder path_to_times_file\n";
    return EXIT_FAILURE;
  }

  createLogger();

  // Retrieve paths to images
  std::vector<string> vstrImageFilenames;
  std::vector<double> vTimestamps;
  if(!LoadImages(string(argv[3]), string(argv[4]), vstrImageFilenames, vTimestamps)) {
    return EXIT_FAILURE;
  }

  const int nImages = vstrImageFilenames.size();

  if(nImages <= 0) {
    spdlog::error("ERROR: Failed to load images");
    return 1;
  }

  // Create SLAM system. It initializes all system threads and gets ready to process frames.
  ORB_SLAM2::System SLAM(argv[1], argv[2], ORB_SLAM2::System::MONOCULAR, true);

  // Vector for tracking time statistics
  std::vector<float> vTimesTrack;
  vTimesTrack.resize(nImages);

  spdlog::debug("-------");
  spdlog::debug("Start processing sequence ...");
  spdlog::debug("Images in the sequence: {}", nImages);

  // Main loop
  cv::Mat im;
  for(int ni = 0; ni < nImages; ni++) {
    // Read image from file
    im = cv::imread(vstrImageFilenames[ni], CV_LOAD_IMAGE_UNCHANGED);
    double tframe = vTimestamps[ni];

    if(im.empty()) {
      std::cerr << endl << "Failed to load image at: " << vstrImageFilenames[ni] << std::endl;
      return EXIT_FAILURE;
    }

    const auto t1 = std::chrono::steady_clock::now();
    // Pass the image to the SLAM system
    SLAM.trackMonocular(im, tframe);

    const auto t2 = std::chrono::steady_clock::now();

    const double ttrack = std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();

    vTimesTrack[ni] = ttrack;

    // Wait to load the next frame
    double T = 0;
    if(ni < nImages - 1) {
      T = vTimestamps[ni + 1] - tframe;
    } else if(ni > 0) {
      T = tframe - vTimestamps[ni - 1];
    }

    if(ttrack < T) {
      usleep((T - ttrack) * 1e6);
    }
  }

  // Stop all threads
  SLAM.Shutdown();

  // Tracking time statistics
  std::sort(vTimesTrack.begin(), vTimesTrack.end());
  float totaltime = 0;
  for(int ni = 0; ni < nImages; ni++) {
    totaltime += vTimesTrack[ni];
  }
  spdlog::debug("-------");
  spdlog::debug("median tracking time: {}", vTimesTrack[nImages / 2]);
  spdlog::debug("mean tracking time: ", totaltime / nImages);

  // Save camera trajectory
  SLAM.save("KeyFrameTrajectory.txt", ORB_SLAM2::System::SaveFormat::TUM);

  return EXIT_SUCCESS;
}

