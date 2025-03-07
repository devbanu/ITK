/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/


#include "itkCastImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkChangeInformationImageFilter.h"
#include "itkTestingComparisonImageFilter.h"
#include "itkTestingMacros.h"
#include <fstream>

using PixelType = float;
using myFloatImage = itk::Image<PixelType, 2>;
using ImageType = itk::Image<PixelType, 2>;
using ImagePointer = ImageType::Pointer;

namespace
{

// compare two images with an intensity tolerance
bool
SameImage(ImagePointer testImage, ImagePointer baselineImage)
{
  constexpr PixelType     intensityTolerance = .001;
  constexpr int           radiusTolerance = 0;
  constexpr unsigned long numberOfPixelTolerance = 0;

  using DiffType = itk::Testing::ComparisonImageFilter<ImageType, ImageType>;
  auto diff = DiffType::New();
  diff->SetValidInput(baselineImage);
  diff->SetTestInput(testImage);
  diff->SetDifferenceThreshold(intensityTolerance);
  diff->SetToleranceRadius(radiusTolerance);
  diff->UpdateLargestPossibleRegion();

  const unsigned long status = diff->GetNumberOfPixelsWithDifferences();


  if (status > numberOfPixelTolerance)
  {
    std::cout << "Number of Different Pixels: " << status << std::endl;
    return false;
  }

  return true;
}
} // namespace


int
itkGradientAnisotropicDiffusionImageFilterTest2(int argc, char * argv[])
{
  if (argc < 3)
  {
    std::cerr << "Missing parameters." << std::endl;
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv) << " InputImage OutputImage" << std::endl;
    return EXIT_FAILURE;
  }


  const itk::ImageFileReader<myFloatImage>::Pointer input = itk::ImageFileReader<myFloatImage>::New();
  input->SetFileName(argv[1]);

  // Create a filter
  const itk::GradientAnisotropicDiffusionImageFilter<myFloatImage, myFloatImage>::Pointer filter =
    itk::GradientAnisotropicDiffusionImageFilter<myFloatImage, myFloatImage>::New();
  filter->SetNumberOfIterations(10);
  filter->SetConductanceParameter(1.0f);
  filter->SetTimeStep(0.125f);

  filter->SetInput(input->GetOutput());

  using myUCharImage = itk::Image<unsigned char, 2>;
  const itk::CastImageFilter<myFloatImage, myUCharImage>::Pointer caster =
    itk::CastImageFilter<myFloatImage, myUCharImage>::New();
  caster->SetInput(filter->GetOutput());

  ITK_TRY_EXPECT_NO_EXCEPTION(caster->Update());


  // Generate test image
  const itk::ImageFileWriter<myUCharImage>::Pointer writer = itk::ImageFileWriter<myUCharImage>::New();
  writer->SetInput(caster->GetOutput());
  writer->SetFileName(argv[2]);

  ITK_TRY_EXPECT_NO_EXCEPTION(writer->Update());


  const myFloatImage::Pointer normalImage = filter->GetOutput();
  normalImage->DisconnectPipeline();

  // We now set up testing when the image spacing is not trivial 1 and
  // perform diffusion with spacing on
  using ChangeInformationType = itk::ChangeInformationImageFilter<myFloatImage>;
  auto changeInfo = ChangeInformationType::New();
  changeInfo->SetInput(input->GetOutput());
  myFloatImage::SpacingType spacing;
  spacing[0] = input->GetOutput()->GetSpacing()[0] * 100.0;
  spacing[1] = input->GetOutput()->GetSpacing()[1] * 100.0;
  changeInfo->SetOutputSpacing(spacing);
  changeInfo->ChangeSpacingOn();


  filter->SetInput(changeInfo->GetOutput());
  filter->UseImageSpacingOn();
  // need to adjust the time step to the number of iterations equates
  // to the same operation
  filter->SetTimeStep(100.0 * filter->GetTimeStep());

  ITK_TRY_EXPECT_NO_EXCEPTION(filter->Update());


  // the results with spacing should be about the same as without

  normalImage->CopyInformation(filter->GetOutput());
  if (!SameImage(filter->GetOutput(), normalImage))
  {
    std::cout << "Results varied with spacing enabled!" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Test finished." << std::endl;
  return EXIT_SUCCESS;
}
