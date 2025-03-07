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
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTestingMacros.h"

int
itkJPEGImageIOTest2(int argc, char * argv[])
{

  if (argc < 2)
  {
    std::cerr << "Missing parameters." << std::endl;
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv);
    std::cerr << " outputFilename " << std::endl;
    return EXIT_FAILURE;
  }

  constexpr unsigned int Dimension = 2;
  using PixelType = unsigned char;

  using ImageType = itk::Image<PixelType, Dimension>;

  auto image = ImageType::New();

  ImageType::RegionType region;
  ImageType::IndexType  start;
  ImageType::SizeType   size;

  size[0] = 157;
  size[1] = 129;

  start[0] = 0;
  start[1] = 0;

  region.SetSize(size);
  region.SetIndex(start);

  image->SetRegions(region);
  image->AllocateInitialized();

  ImageType::SpacingType spacing;

  spacing[0] = 3.1415;
  spacing[1] = 6.2830;

  image->SetSpacing(spacing);

  using WriterType = itk::ImageFileWriter<ImageType>;
  auto writer = WriterType::New();
  writer->SetFileName(argv[1]);

  writer->SetInput(image);

  ITK_TRY_EXPECT_NO_EXCEPTION(writer->Update());


  using ReaderType = itk::ImageFileReader<ImageType>;
  auto reader = ReaderType::New();
  reader->SetFileName(argv[1]);

  ITK_TRY_EXPECT_NO_EXCEPTION(reader->Update());


  const ImageType * readImage = reader->GetOutput();

  ImageType::SpacingType readSpacing = readImage->GetSpacing();

  constexpr double tolerance = 1e-1;

  if (itk::Math::abs(readSpacing[0] - spacing[0]) > tolerance)
  {
    std::cerr << "Spacing read/write failed !" << std::endl;
    std::cerr << "Expected spacing = " << spacing << std::endl;
    std::cerr << "Found    spacing = " << readSpacing << std::endl;
    return EXIT_FAILURE;
  }

  if (itk::Math::abs(readSpacing[1] - spacing[1]) > tolerance)
  {
    std::cerr << "Spacing read/write failed !" << std::endl;
    std::cerr << "Expected spacing = " << spacing << std::endl;
    std::cerr << "Found    spacing = " << readSpacing << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Expected spacing = " << spacing << std::endl;
  std::cout << "Found    spacing = " << readSpacing << std::endl;
  std::cout << "Test PASSED !" << std::endl;

  return EXIT_SUCCESS;
}
