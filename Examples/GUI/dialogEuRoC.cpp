// Qt5
#include <QDir>
#include <QValidator>
// Internal
#include "dialogEuRoC.hpp"
#include "ui_dialogEuRoC.h"


class DirValidator final : public QValidator {
public:
  explicit DirValidator(QObject *pParent = nullptr) noexcept
    : QValidator(pParent) {}

  ~DirValidator() noexcept override {}

  State validate(QString &text, int &pos) const override {
    return (QDir(text).exists() ? QValidator::Acceptable : QValidator::Intermediate);
  }

};

DialogEuRoC::DialogEuRoC(DatasetEuRoC &dataset) noexcept : mDataset{dataset}, m_pUI{std::make_unique<Ui::Dialog>()} {
  m_pUI->setupUi(this);
  auto *pValidator = new DirValidator(this);
  m_pUI->camera0LineEdit->setValidator(pValidator);
}

DialogEuRoC::~DialogEuRoC() noexcept = default;

void DialogEuRoC::accept() {
  mDataset.cam0      = m_pUI->camera0LineEdit->text();
  mDataset.cam1      = m_pUI->camera1LineEdit->text();
  mDataset.yaml      = m_pUI->yamlLineEdit->text();
  mDataset.vocabl    = m_pUI->vocabularylineEdit->text();
  mDataset.timestamp = m_pUI->timeStampLineEdit->text();

  QDialog::accept();
}
