#pragma once
// Qt5
#include <QDialog>


namespace Ui {
  class Dialog;
} // namespace Ui

struct DatasetEuRoC {
  QString vocabl;
  QString yaml;
  QString cam0;
  QString cam1;
  QString timestamp;
};

class DialogEuRoC final : public QDialog {
Q_OBJECT
public:
  explicit DialogEuRoC(DatasetEuRoC &dataset) noexcept;

  ~DialogEuRoC() noexcept;

  void accept() override;

private:
  DatasetEuRoC &mDataset;
  std::unique_ptr<Ui::Dialog> m_pUI;

};

