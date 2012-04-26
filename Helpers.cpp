#include "Helpers.h"

#include "itkBresenhamLine.h"

namespace Helpers
{

void IndicesToBinaryImage(std::vector<itk::Index<2> > indices, itk::ImageRegion<2> imageRegion, UnsignedCharImageType::Pointer image)
{
  // Blank the image
  image->SetRegions(imageRegion);
  image->Allocate();
  image->FillBuffer(0);

  // Set the pixels of indices in list to 255
  for(unsigned int i = 0; i < indices.size(); i++)
    {
    image->SetPixel(indices[i], 255);
    }
}

std::vector<itk::Index<2> > PolyDataToPixelList(vtkPolyData* polydata)
{
  // Convert vtkPoints to indices
  std::vector<itk::Index<2> > linePoints;
  for(vtkIdType i = 0; i < polydata->GetNumberOfPoints(); i++)
    {
    itk::Index<2> index;
    double p[3];
    polydata->GetPoint(i,p);
    index[0] = round(p[0]);
    index[1] = round(p[1]);
    linePoints.push_back(index);
    }

  // Compute the indices between every pair of points
  std::vector<itk::Index<2> > allIndices;
  for(unsigned int linePointId = 1; linePointId < linePoints.size(); linePointId++)
    {
    itk::Index<2> index0 = linePoints[linePointId-1];
    itk::Index<2> index1 = linePoints[linePointId];
    // Currently need the distance between the points for Bresenham (pending patch in Gerrit)
    itk::Point<float,2> point0;
    itk::Point<float,2> point1;
    for(unsigned int i = 0; i < 2; i++)
      {
      point0[i] = index0[i];
      point1[i] = index1[i];
      }
    float distance = point0.EuclideanDistanceTo(point1);
    itk::BresenhamLine<2> line;
    std::vector<itk::Offset<2> > offsets = line.BuildLine(point1-point0, distance);
    for(unsigned int i = 0; i < offsets.size(); i++)
      {
      allIndices.push_back(index0 + offsets[i]);
      }
    }

  return allIndices;
}

std::pair<itk::Index<2>, itk::Index<2> > IntersectLineWithMask(std::vector<itk::Index<2> > line, UnsignedCharImageType::Pointer mask, bool &hasInteriorLine)
{
  // We consider the hole to be non-zero pixels of the mask. We want to find where the line enters the mask, and where it leaves the mask.
  // This function assumes that the line starts outside the mask. Nothing is assumed about where the line ends (if it ends inside the mask, then there is no interior line).
  // 'line' is an ordered vector of indices.
  // We assume the hole is convex. Nothing will break if it is not, but the line that is computed goes "through" the mask, but may
  // actually not be entirely contained within the hole if the hole is not convex.
  
  std::pair<itk::Index<2>, itk::Index<2> > interiorLine; // (start pixel, end pixel)

  unsigned int startPoints = 0;
  unsigned int endPoints = 0;
  
  // Loop over the pixels in the line. If one of them is outside the mask and its neighbor is inside the mask, this is an intersection.
  for(unsigned int i = 0; i < line.size() - 1; i++) // loop to one before the end because we use the current and current+1 in the loop
    {
    if(mask->GetPixel(line[i]) == 0 && mask->GetPixel(line[i+1]) != 0) // Found entry point
      {
      interiorLine.first = line[i]; // We want to save the outside/valid/non-hole point. This is the first point (i) in the 'exit' case.
      startPoints++;
      }
      
    if(mask->GetPixel(line[i]) != 0 && mask->GetPixel(line[i+1]) == 0) // Found exit point
      {
      interiorLine.second = line[i+1]; // We want to save the outside/valid/non-hole point. This is the second point (i+1) in the 'exit' case.
      endPoints++;
      }
    }

  // If there is exactly one entry and exactly 1 exit point, the interior line is well defined
  if(startPoints == 1 && endPoints == 1)
    {
    hasInteriorLine = true;
    }
  else
    {
    hasInteriorLine = false;
    }

  // This is only valid if hasInteriorLine is true
  return interiorLine;
}

} // end namespace