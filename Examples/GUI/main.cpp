// Qt5
#include <QApplication>
// Internal
#include "mainwindow.hpp"


int main(int argc, char* argv[]) {
  QApplication application{argc, argv};

  MainWindow window;
  window.show();

  return QApplication::exec();
}

