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

#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"
#include "itkTimeProbesCollectorBase.h"
#include "itkTestingMacros.h"


namespace
{

template <typename TImageType>
int
ActualTest(std::string filename, typename TImageType::SizeType size)
{
  using ImageType = TImageType;
  using PixelType = typename ImageType::PixelType;


  using WriterType = itk::ImageFileWriter<ImageType>;
  using ReaderType = itk::ImageFileReader<ImageType>;

  using IteratorType = itk::ImageRegionIterator<ImageType>;
  using ConstIteratorType = itk::ImageRegionConstIterator<ImageType>;

  const typename ImageType::IndexType  index{};
  const typename ImageType::RegionType region{ index, size };

  { // begin write block
    auto image = ImageType::New();

    image->SetRegions(region);

    size_t numberOfPixels = 1;
    for (unsigned int i = 0; i < ImageType::ImageDimension; ++i)
    {
      numberOfPixels *= region.GetSize(i);
    }

    const size_t sizeInMebiBytes = sizeof(PixelType) * numberOfPixels / (1024 * 1024);

    std::cout << "Trying to allocate an image of size " << sizeInMebiBytes << " MiB " << std::endl;

    itk::TimeProbesCollectorBase chronometer;
    chronometer.Start("Allocate");
    image->Allocate();
    chronometer.Stop("Allocate");

    std::cout << "Initializing pixel values " << std::endl;

    IteratorType itr(image, region);
    itr.GoToBegin();

    PixelType pixelValue{};

    chronometer.Start("Initializing");
    while (!itr.IsAtEnd())
    {
      itr.Set(pixelValue);
      ++pixelValue;
      ++itr;
    }
    chronometer.Stop("Initializing");

    std::cout << "Trying to write the image to disk" << std::endl;
    try
    {
      auto writer = WriterType::New();
      writer->SetInput(image);
      writer->SetFileName(filename);
      chronometer.Start("Write");
      writer->Update();
      chronometer.Stop("Write");
    }
    catch (const itk::ExceptionObject & ex)
    {
      std::cout << ex << std::endl;
      return EXIT_FAILURE;
    }
  } // end write block to free the memory

  std::cout << "Trying to read the image back from disk" << std::endl;
  auto reader = ReaderType::New();
  reader->SetFileName(filename);

  try
  {

    itk::TimeProbesCollectorBase chronometer;
    chronometer.Start("Read");
    reader->Update();
    chronometer.Stop("Read");
  }
  catch (const itk::ExceptionObject & ex)
  {
    std::cout << ex << std::endl;
    return EXIT_FAILURE;
  }

  const typename ImageType::ConstPointer readImage = reader->GetOutput();

  ConstIteratorType ritr(readImage, region);
  // IteratorType oitr( image, region );

  ritr.GoToBegin();
  // oitr.GoToBegin();

  std::cout << "Comparing the pixel values..." << std::endl;

  PixelType pixelValue{};


  itk::TimeProbesCollectorBase chronometer;
  chronometer.Start("Compare");
  while (!ritr.IsAtEnd())
  {
    // if( ( oitr.Get() != ritr.Get() ) || ( oitr.Get() != pixelValue ) )
    if (ritr.Get() != pixelValue)
    {
      std::cerr << "Pixel comparison failed at index = " << ritr.GetIndex() << std::endl;
      std::cerr << "Expected pixel value " << pixelValue << std::endl;
      // std::cerr << "Original Image pixel value " << oitr.Get() << std::endl;
      std::cerr << "Read Image pixel value " << ritr.Get() << std::endl;
      return EXIT_FAILURE;
    }

    ++pixelValue;
    // ++oitr;
    ++ritr;
  }
  chronometer.Stop("Compare");

  chronometer.Report(std::cout);

  std::cout << std::endl;
  std::cout << "Test PASSED !" << std::endl;

  return EXIT_SUCCESS;
}

} // namespace

int
itkLargeImageWriteReadTest(int argc, char * argv[])
{

  if (argc < 3)
  {
    std::cout << "Usage: " << itkNameOfTestExecutableMacro(argv)
              << " outputFileName numberOfPixelsInOneDimension [numberOfZslices]" << std::endl;
    return EXIT_FAILURE;
  }

  const std::string filename = argv[1];

  if (argc == 3)
  {
    constexpr unsigned int Dimension = 2;

    using PixelType = unsigned short;
    using ImageType = itk::Image<PixelType, Dimension>;

    auto size = ImageType::SizeType::Filled(atol(argv[2]));

    return ActualTest<ImageType>(filename, size);
  }

  constexpr unsigned int Dimension = 3;

  using PixelType = unsigned short;
  using ImageType = itk::Image<PixelType, Dimension>;

  auto size = ImageType::SizeType::Filled(atol(argv[2]));
  size[2] = atol(argv[3]);

  return ActualTest<ImageType>(filename, size);
}
