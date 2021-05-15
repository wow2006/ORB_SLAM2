// STL
#include <cmath>
// QGLViewer
#include <QGLViewer/qglviewer.h>
// Internal
#include "Scene.hpp"


Scene::Scene(const QGLViewer *const pViewer)
  : m_pViewer{pViewer} {
  // Connect the viewer signal to our draw slot.
  connect(pViewer, SIGNAL(drawNeeded()), this, SLOT(draw()));
}

void Scene::draw() {
  m_pViewer->drawAxis();
  glPushMatrix();
  {
    glRotatef(90, 1, 0, 0);
    m_pViewer->drawGrid();
  }
  glPopMatrix();
}

