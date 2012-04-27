/*
Copyright (C) 2011 David Doria, daviddoria@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef MainWindow_H
#define MainWindow_H

// Custom
#include "ScribbleInteractorStyle/vtkInteractorStyleScribble.h"
#include "Types.h"

// Submodules
#include "PTXTools/PTXImage.h"

// Qt
#include <QMainWindow>
#include "ui_MainWindow.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageActor.h>
#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>

class MainWindow : public QMainWindow, public Ui::MainWindow
{
Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);

  //void StrokeUpdatedSlot(vtkPolyData*);
  void StrokeUpdatedSlot(vtkObject* caller, long unsigned int eventId, void* callData)
public slots:

  void mnuOpenPTX_triggered();
  void mnuLoadMask_triggered();
  void btnSaveDepths_clicked();
  
private:

  void Refresh();
  void CreateOutputs();
  
  // Options
  double CameraUp[3];
  double BackgroundColor[3];

  // Objects for the left renderer
  vtkSmartPointer<vtkRenderer> LeftRenderer;
  vtkSmartPointer<vtkInteractorStyleScribble> ScribbleInteractorStyle;
  vtkSmartPointer<vtkImageData> Image;
  vtkSmartPointer<vtkImageActor> ImageActor;

  // Objects for the right renderer
  vtkSmartPointer<vtkPolyData> PointCloud;
  vtkSmartPointer<vtkPolyDataMapper> PointCloudMapper;
  vtkSmartPointer<vtkActor> PointCloudActor;
  vtkSmartPointer<vtkRenderer> RightRenderer;

  // 2D Line objects
  vtkSmartPointer<vtkPolyData> Line2D;
  vtkSmartPointer<vtkPolyDataMapper> Line2DMapper;
  vtkSmartPointer<vtkActor> Line2DActor;

  // 3D Line objects
  vtkSmartPointer<vtkPolyData> Line3D;
  vtkSmartPointer<vtkPolyDataMapper> Line3DMapper;
  vtkSmartPointer<vtkActor> Line3DActor;

  // 3D Line-through-mask hole objects
  vtkSmartPointer<vtkPolyData> LineHole3D;
  vtkSmartPointer<vtkPolyDataMapper> LineHole3DMapper;
  vtkSmartPointer<vtkActor> LineHole3DActor;

  // 2D Line-through-mask hole objects
  vtkSmartPointer<vtkPolyData> LineHole2D;
  vtkSmartPointer<vtkPolyDataMapper> LineHole2DMapper;
  vtkSmartPointer<vtkActor> LineHole2DActor;

  // The reader and storage of the PTX file
  PTXImage ptxImage;

  // The mask
  UnsignedCharImageType::Pointer Mask;
};

#endif
