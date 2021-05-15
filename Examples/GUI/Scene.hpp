#pragma once
// Qt5
#include <QObject>
// Internal
#include "../../include/IViewer.hpp"


class QGLViewer;

class Scene : public QObject, public ORB_SLAM2::IViewer {
Q_OBJECT

public:
  Scene(const QGLViewer *const v);

  void draw();

private:
  const QGLViewer *const m_pViewer;

};
