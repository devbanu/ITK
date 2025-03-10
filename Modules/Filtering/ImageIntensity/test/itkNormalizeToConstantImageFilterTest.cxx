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

#include <iostream>

#include "itkNormalizeToConstantImageFilter.h"
#include "itkRandomImageSource.h"
#include "itkSimpleFilterWatcher.h"
#include "itkMath.h"
#include "itkTestingMacros.h"

int
itkNormalizeToConstantImageFilterTest(int, char *[])
{
  constexpr unsigned int Dimension = 3;
  using IntPixelType = int;
  using DoublePixelType = double;

  using IntImage = itk::Image<IntPixelType, Dimension>;
  using DoubleImage = itk::Image<DoublePixelType, Dimension>;

  // Generate a random image
  using SourceType = itk::RandomImageSource<IntImage>;
  auto source = SourceType::New();

  IntImage::SizeValueType randomSize[3] = { 18, 17, 67 };

  source->SetSize(randomSize);

  constexpr IntImage::PixelType minValue = 0;
  constexpr IntImage::PixelType maxValue = 1000;
  source->SetMin(minValue);
  source->SetMax(maxValue);

  using NormalizeType = itk::NormalizeToConstantImageFilter<IntImage, DoubleImage>;
  auto normalize = NormalizeType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(normalize, NormalizeToConstantImageFilter, ImageToImageFilter);

  DoubleImage::PixelType constant = 1.0;
  normalize->SetConstant(constant);
  ITK_TEST_SET_GET_VALUE(constant, normalize->GetConstant());

  const itk::SimpleFilterWatcher watch(normalize, "NormalizeToConstant");

  normalize->SetInput(source->GetOutput());
  normalize->Update();

  using IteratorType = itk::ImageRegionConstIterator<DoubleImage>;
  IteratorType it(normalize->GetOutput(), normalize->GetOutput()->GetLargestPossibleRegion());

  DoubleImage::PixelType sum = 0.0;
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    sum += it.Value();
  }

  double epsilon = 1e-5;
  if (!itk::Math::FloatAlmostEqual(constant, sum, 10, epsilon))
  {
    std::cout.precision(static_cast<int>(itk::Math::abs(std::log10(epsilon))));
    std::cout << "First sum (" << sum << ") does not equal constant (" << constant << ')' << std::endl;
    return EXIT_FAILURE;
  }

  constant = 134.2;
  normalize->SetConstant(constant);
  ITK_TEST_SET_GET_VALUE(constant, normalize->GetConstant());

  normalize->Update();

  sum = 0.0;
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    sum += it.Value();
  }

  epsilon = 1e-3;
  if (!itk::Math::FloatAlmostEqual(constant, sum, 10, epsilon))
  {
    std::cout.precision(static_cast<int>(itk::Math::abs(std::log10(epsilon))));
    std::cout << "Second sum (" << sum << ") does not equal constant (" << constant << ')' << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
