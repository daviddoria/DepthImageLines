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

#include "MainWindow.h"

// Custom
#include "Helpers.h"

// Submodules
#include "PTXTools/PTXReader.h"

// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// QT
#include <QFileDialog>

// VTK
#include <vtkCamera.h> // to potentially flip the image
#include <vtkCellArray.h> // to create the polyline
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLineSource.h>
#include <vtkPolyLine.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkVertexGlyphFilter.h>

MainWindow::MainWindow(QWidget *parent)
{
  // Setup the GUI and connect all of the signals and slots
  setupUi(this);

  // Setup Qt signals/slots
  connect( this->mnuOpenPTX, SIGNAL( triggered() ), this, SLOT(mnuOpenPTX_triggered()) );
  connect( this->mnuLoadMask, SIGNAL( triggered() ), this, SLOT(mnuLoadMask_triggered()) );
  connect( this->btnSaveDepths, SIGNAL( clicked() ), this, SLOT(btnSaveDepths_clicked()) );

  this->CameraUp[0] = 0;
  this->CameraUp[1] = 1;
  this->CameraUp[2] = 0;

  this->BackgroundColor[0] = 0;
  this->BackgroundColor[1] = 0;
  this->BackgroundColor[2] = .5;

  // Instantiations
  this->Mask = NULL;
  
  this->ImageActor = vtkSmartPointer<vtkImageActor>::New();
  this->PointCloudActor = vtkSmartPointer<vtkActor>::New();

  this->Image = vtkSmartPointer<vtkImageData>::New();
  this->PointCloud = vtkSmartPointer<vtkPolyData>::New();
  this->PointCloudMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

  this->Line2D = vtkSmartPointer<vtkPolyData>::New();
  this->Line2DMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->Line2DActor = vtkSmartPointer<vtkActor>::New();
  this->Line2DActor->GetProperty()->SetLineWidth(4);
  this->Line2DActor->GetProperty()->SetColor(0,1,0); // green

  this->Line3D = vtkSmartPointer<vtkPolyData>::New();
  this->Line3DMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->Line3DActor = vtkSmartPointer<vtkActor>::New();
  this->Line3DActor->GetProperty()->SetLineWidth(4);
  this->Line3DActor->GetProperty()->SetColor(0,1,0); // green

  this->LineHole3D = vtkSmartPointer<vtkPolyData>::New();
  this->LineHole3DMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LineHole3DActor = vtkSmartPointer<vtkActor>::New();
  this->LineHole3DActor->GetProperty()->SetLineWidth(4);
  this->LineHole3DActor->GetProperty()->SetColor(1,0,0); // red

  this->LineHole2D = vtkSmartPointer<vtkPolyData>::New();
  this->LineHole2DMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->LineHole2DActor = vtkSmartPointer<vtkActor>::New();
  this->LineHole2DActor->GetProperty()->SetLineWidth(4);
  this->LineHole2DActor->GetProperty()->SetColor(1,0,0); // red
  
  // Add renderers - we flip the image by changing the camera view up because of the conflicting conventions used by ITK and VTK
  this->LeftRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->LeftRenderer->GradientBackgroundOn();
  this->LeftRenderer->SetBackground(this->BackgroundColor);
  this->LeftRenderer->SetBackground2(1,1,1);
  this->LeftRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);
  this->qvtkWidgetLeft->GetRenderWindow()->AddRenderer(this->LeftRenderer);

  this->RightRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->RightRenderer->GradientBackgroundOn();
  this->RightRenderer->SetBackground(this->BackgroundColor);
  this->RightRenderer->SetBackground2(1,1,1);
  this->RightRenderer->GetActiveCamera()->SetViewUp(this->CameraUp);
  this->qvtkWidgetRight->GetRenderWindow()->AddRenderer(this->RightRenderer);

  // Setup left interactor style
  this->ScribbleInteractorStyle = vtkSmartPointer<vtkInteractorStyleScribble>::New();
  this->qvtkWidgetLeft->GetInteractor()->SetInteractorStyle(this->ScribbleInteractorStyle);
  //this->ScribbleInteractorStyle->StrokeUpdated.connect(boost::bind(&MainWindow::StrokeUpdatedSlot, this, _1));
  this->ScribbleInteractorStyle->AddObserver(this->ScribbleInteractorStyle->ScribbleEvent,
                                             this, &MainWindow::StrokeUpdatedSlot);
  // Setup right interactor style
  vtkSmartPointer<vtkInteractorStyleTrackballCamera> trackballStyle =
    vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  this->qvtkWidgetRight->GetInteractor()->SetInteractorStyle( trackballStyle );

}

void MainWindow::btnSaveDepths_clicked()
{
  //std::cout << "3D line has " << LineHole3D->GetNumberOfPoints() << " points." << std::endl;

  double point0[3];
  this->LineHole3D->GetPoints()->GetPoint(0, point0);

  double point1[3];
  this->LineHole3D->GetPoints()->GetPoint(1, point1);
  

  typedef itk::Point<float, 3> PointType;

  PointType p0;
  for(unsigned int i = 0; i < 3; i++)
    {
    p0[i] = point0[i];
    }

  PointType p1;
  for(unsigned int i = 0; i < 3; i++)
    {
    p1[i] = point1[i];
    }

  typedef itk::Vector<float, 3> VectorType;
  VectorType v = p1 - p0;
  v.Normalize();
    
  std::vector<itk::Index<2> > pixels = Helpers::PolyDataToPixelList(this->LineHole2D);
  std::vector<float> distances;

  PointType origin;
  for(unsigned int i = 0; i < 3; i++)
    {
    origin[i] = 0;
    }
    
  for(unsigned int i = 0; i < pixels.size(); i++)
    {
    PointType point = p0 + v * static_cast<float>(i)/static_cast<float>(pixels.size());
    distances.push_back(origin.EuclideanDistanceTo(point));
    }

  FloatImageType::Pointer image = FloatImageType::New();
  image->SetRegions(this->ptxImage.GetFullImage()->GetLargestPossibleRegion());
  image->Allocate();
  image->FillBuffer(-1);

  for(unsigned int i = 0; i < pixels.size(); i++)
    {
    image->SetPixel(pixels[i], distances[i]);
    }

  typedef  itk::ImageFileWriter< FloatImageType  > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName("depths.mhd");
  writer->SetInput(image);
  writer->Update();
}

void MainWindow::StrokeUpdatedSlot(vtkObject* caller, long unsigned int eventId, void* callData)
{
  // This function is called when the ScribbleInteractorStyle emits its Updated signal
  //std::cout << "Stroke updated." << std::endl;
  vtkSmartPointer<vtkPolyData> pathPolydata = vtkSmartPointer<vtkPolyData>::New();

  VTKHelpers::PathFromPoints(this->ScribbleInteractorStyle->GetSelection(), pathPolydata);
  
  // Setup and draw the 2D line
  this->Line2D->ShallowCopy(pathPolydata);
  this->Line2DMapper->SetInputData(this->Line2D);
  this->Line2DActor->SetMapper(this->Line2DMapper);
  this->LeftRenderer->AddActor(this->Line2DActor);

  // Create 3D line from 2D line
  std::vector<itk::Index<2> > pixels = Helpers::PolyDataToPixelList(pathPolydata);
  std::cout << "There are " << pixels.size() << " points in the path." << std::endl;

  // Insert valid points into a vtkPoints object from which to create a PolyLine
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for(unsigned int i = 0; i < pixels.size(); i++)
    {
    PTXPixel fullPixel = this->ptxImage.GetFullImage()->GetPixel(pixels[i]);

    if(fullPixel.Valid)
      {
      points->InsertNextPoint(fullPixel.X, fullPixel.Y, fullPixel.Z);
      }
    }

  // Create the line (a vtkPolyLine)
  vtkSmartPointer<vtkPolyLine> polyLine =
    vtkSmartPointer<vtkPolyLine>::New();
  polyLine->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
  for(unsigned int i = 0; i < static_cast<unsigned int>(points->GetNumberOfPoints()); i++)
    {
    polyLine->GetPointIds()->SetId(i,i);
    }

  vtkSmartPointer<vtkCellArray> cells =
    vtkSmartPointer<vtkCellArray>::New();
  cells->InsertNextCell(polyLine);

  // Construct a polydata from the points and the polyline
  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  polydata->SetPoints(points);
  polydata->SetLines(cells);

  // Setup and draw the 3D line
  this->Line3D->ShallowCopy(polydata);
  this->Line3DMapper->SetInputData(this->Line3D);
  this->Line3DActor->SetMapper(this->Line3DMapper);
  this->RightRenderer->AddActor(this->Line3DActor);
  this->Refresh();

  if(this->Mask)
    {
    bool hasInteriorLine;
    std::pair<itk::Index<2>, itk::Index<2> > interiorLine = Helpers::IntersectLineWithMask(pixels, this->Mask, hasInteriorLine);
    std::cout << "Has interior line? " << hasInteriorLine << std::endl;
  
    bool endPointsValid = true;
  
    if(hasInteriorLine)
      {
      PTXPixel pixel1 = this->ptxImage.GetFullImage()->GetPixel(interiorLine.first);
      double p1[3] = {pixel1.X, pixel1.Y, pixel1.Z};
      if(!pixel1.Valid)
        {
        endPointsValid = false;
        }
        
      PTXPixel pixel2 = this->ptxImage.GetFullImage()->GetPixel(interiorLine.second);
      double p2[3] = {pixel2.X, pixel2.Y, pixel2.Z};
      if(!pixel2.Valid)
        {
        endPointsValid = false;
        }
      
      vtkSmartPointer<vtkLineSource> lineSource3D =
        vtkSmartPointer<vtkLineSource>::New();
      lineSource3D->SetPoint1(p1);
      lineSource3D->SetPoint2(p2);
      lineSource3D->Update();

      vtkSmartPointer<vtkLineSource> lineSource2D =
        vtkSmartPointer<vtkLineSource>::New();
      double p1_2D[3] = {interiorLine.first[0], interiorLine.first[1], 0};
      double p2_2D[3] = {interiorLine.second[0], interiorLine.second[1], 0};
      lineSource2D->SetPoint1(p1_2D);
      lineSource2D->SetPoint2(p2_2D);
      lineSource2D->Update();

      std::cout << "End points valid? " << endPointsValid << std::endl;
      
      if(endPointsValid)
        {
        this->LineHole3D->ShallowCopy(lineSource3D->GetOutput());
        this->LineHole3DMapper->SetInputData(this->LineHole3D);
        this->LineHole3DActor->SetMapper(this->LineHole3DMapper);
        this->RightRenderer->AddActor(this->LineHole3DActor);

        this->LineHole2D->ShallowCopy(lineSource2D->GetOutput());
        this->LineHole2DMapper->SetInputData(this->LineHole2D);
        this->LineHole2DActor->SetMapper(this->LineHole2DMapper);
        this->LeftRenderer->AddActor(this->LineHole2DActor);
      
        this->Refresh();
        }
      }
    }
}

void MainWindow::mnuOpenPTX_triggered()
{
  // Get a filename to open
  QString filename = QFileDialog::getOpenFileName(this,
     "Open PTX File", "../../src/DepthImageLines/data", "PTX Files (*.ptx)");

  if(filename.isEmpty())
    {
    std::cout << "No file selected!" << std::endl;
    return;
    }

  // Read file
  this->ptxImage = PTXReader::Read(filename.toStdString());

  this->CreateOutputs();
  
  this->LeftRenderer->ResetCamera();
  this->RightRenderer->ResetCamera();

  this->Refresh();

}


void MainWindow::mnuLoadMask_triggered()
{
  // Get a filename to open
  QString filename = QFileDialog::getOpenFileName(this,
     "Load Mask", "../../src/DepthImageLines/data", "Binary Image Files (*.png)");

  if(filename.isEmpty())
    {
    std::cout << "No file selected!" << std::endl;
    return;
    }

  // Read file
  typedef itk::ImageFileReader<UnsignedCharImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(filename.toStdString());
  reader->Update();

  if(!this->Mask)
    {
    this->Mask = UnsignedCharImageType::New();
    }
  this->Mask->Graft(reader->GetOutput());
  
  // Apply the mask
  this->ptxImage.ApplyMask(this->Mask);

  this->CreateOutputs();
  this->Refresh();

}

void MainWindow::CreateOutputs()
{
  // Get and display the image
  PTXImage::RGBImageType::Pointer image = PTXImage::RGBImageType::New();
  this->ptxImage.CreateRGBImage(image);

  Helpers::ITKImageToVTKImage<PTXImage::RGBImageType>(image, this->Image);

  this->ImageActor->SetInputData(this->Image);
  this->LeftRenderer->AddActor(this->ImageActor);
  this->ScribbleInteractorStyle->InitializeTracer(this->ImageActor);

  // Get and display the point cloud
  this->ptxImage.CreatePointCloud(this->PointCloud);
  this->PointCloudMapper->SetInputData(this->PointCloud);
  this->PointCloudActor->SetMapper(this->PointCloudMapper);

  this->RightRenderer->AddActor(this->PointCloudActor);
}

void MainWindow::Refresh()
{
  this->LeftRenderer->Render();
  this->RightRenderer->Render();
  this->qvtkWidgetRight->GetRenderWindow()->Render();
  this->qvtkWidgetLeft->GetRenderWindow()->Render();
  this->qvtkWidgetRight->GetInteractor()->Render();
  this->qvtkWidgetLeft->GetInteractor()->Render();
}
