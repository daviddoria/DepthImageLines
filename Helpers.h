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

#ifndef HELPERS_H
#define HELPERS_H

// Custom
#include "Types.h"

// STL
#include <vector>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>

// ITK
#include "itkIndex.h"

namespace Helpers
{

std::vector<itk::Index<2> > PolyDataToPixelList(vtkPolyData* polydata);

void IndicesToBinaryImage(std::vector<itk::Index<2> > indices, itk::ImageRegion<2> imageRegion, UnsignedCharImageType::Pointer image);

std::pair<itk::Index<2>, itk::Index<2> > IntersectLineWithMask(std::vector<itk::Index<2> > line, UnsignedCharImageType::Pointer mask, bool &hasInteriorLine);

// Convert an ITK image to a VTK image for display
template <typename TImage>
void ITKImageToVTKImage(typename TImage::Pointer image, vtkSmartPointer<vtkImageData> outputImage);

} // end namespace

#include "Helpers.txx"

#endif
