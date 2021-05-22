// Qt5
#include <QDir>
#include <QSettings>
#include <QValidator>
#include <QPushButton>
#include <QFileDialog>
// Internal
#include "dialogEuRoC.hpp"
#include "ui_dialogEuRoC.h"


class DirValidator final : public QValidator {
public:
  explicit DirValidator(QObject *pParent = nullptr) noexcept
    : QValidator(pParent) {}

  ~DirValidator() noexcept override {}

  State validate(QString &text, [[maybe_unused]] int &pos) const override {
    return (QDir(text).exists() ? QValidator::Acceptable : QValidator::Intermediate);
  }

};

DialogEuRoC::DialogEuRoC(DatasetEuRoC &dataset) noexcept
  : mDataset{dataset}, m_pUI{std::make_unique<Ui::Dialog>()} {
  m_pUI->setupUi(this);
  auto *pValidator = new DirValidator(this);
  m_pUI->camera0LineEdit->setValidator(pValidator);

  loadSettings();

  connect(m_pUI->camera0PathButton, &QToolButton::clicked,
          [this]() {
            const auto directory = QFileDialog::getExistingDirectory();
            if(!directory.isEmpty()) {
              m_pUI->camera0LineEdit->setText(directory);
            }
          });
  connect(m_pUI->camera1PathButton, &QToolButton::clicked,
          [this]() {
            const auto directory = QFileDialog::getExistingDirectory();
            if(!directory.isEmpty()) {
              m_pUI->camera0LineEdit->setText(directory);
            }
          });
  connect(m_pUI->settingPathButton, &QToolButton::clicked,
          [this]() {
            const auto filePath = QFileDialog::getOpenFileName();
            if(!filePath.isEmpty()) {
              m_pUI->yamlLineEdit->setText(filePath);
            }
          });
  connect(m_pUI->vocabPathButton,   &QToolButton::clicked,
          [this]() {
            const auto filePath = QFileDialog::getOpenFileName();
            if(!filePath.isEmpty()) {
              m_pUI->vocabularylineEdit->setText(filePath);
            }
          });
  connect(m_pUI->timeStampPathButton,   &QToolButton::clicked,
          [this]() {
            const auto filePath = QFileDialog::getOpenFileName();
            if(!filePath.isEmpty()) {
              m_pUI->timeStampLineEdit->setText(filePath);
            }
          });
}

void DialogEuRoC::loadSettings() {
  QSettings settings;

  m_pUI->camera0LineEdit->setText(settings.value("DialogEuRoC/cam0").toString());
  m_pUI->camera1LineEdit->setText(settings.value("DialogEuRoC/cam1").toString());
  m_pUI->yamlLineEdit->setText(settings.value("DialogEuRoC/yaml").toString());
  m_pUI->vocabularylineEdit->setText(settings.value("DialogEuRoC/vocabl").toString());
  m_pUI->timeStampLineEdit->setText(settings.value("DialogEuRoC/timestamp").toString());
}

DialogEuRoC::~DialogEuRoC() noexcept = default;

void DialogEuRoC::accept() {
  mDataset.cam0      = m_pUI->camera0LineEdit->text();
  mDataset.cam1      = m_pUI->camera1LineEdit->text();
  mDataset.yaml      = m_pUI->yamlLineEdit->text();
  mDataset.vocabl    = m_pUI->vocabularylineEdit->text();
  mDataset.timestamp = m_pUI->timeStampLineEdit->text();

  saveSettings();

  QDialog::accept();
}

void DialogEuRoC::saveSettings() const {
  QSettings settings;
  settings.setValue("DialogEuRoC/cam0",      mDataset.cam0);
  settings.setValue("DialogEuRoC/cam1",      mDataset.cam1);
  settings.setValue("DialogEuRoC/yaml",      mDataset.yaml);
  settings.setValue("DialogEuRoC/vocabl",    mDataset.vocabl);
  settings.setValue("DialogEuRoC/timestamp", mDataset.timestamp);
}

