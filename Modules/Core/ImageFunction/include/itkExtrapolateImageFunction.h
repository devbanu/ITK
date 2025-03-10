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
#ifndef itkExtrapolateImageFunction_h
#define itkExtrapolateImageFunction_h

#include "itkImageFunction.h"

namespace itk
{
/**
 * \class ExtrapolateImageFunction
 * \brief Base class for all image extrapolaters.
 *
 * ExtrapolateImageFunction is the base for all ImageFunctions that
 * extrapolates image intensity at a non-integer pixel position
 * outside the image buffer.
 * This class is templated over the input image type and the
 * coordinate representation type (e.g. float or double ).
 *
 * \warning This hierarchy of functions work only for images
 * with scalar pixel types.
 *
 * \ingroup ImageFunctions
 *
 *
 * \ingroup ITKImageFunction
 */
template <typename TInputImage, typename TCoordinate = float>
class ExtrapolateImageFunction
  : public ImageFunction<TInputImage, typename NumericTraits<typename TInputImage::PixelType>::RealType, TCoordinate>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(ExtrapolateImageFunction);

  /** Standard class type aliases. */
  using Self = ExtrapolateImageFunction;
  using Superclass =
    ImageFunction<TInputImage, typename NumericTraits<typename TInputImage::PixelType>::RealType, TCoordinate>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(ExtrapolateImageFunction);

  /** OutputType type alias support */
  using typename Superclass::OutputType;

  /** InputImageType type alias support */
  using typename Superclass::InputImageType;

  /** Dimension underlying input image. */
  static constexpr unsigned int ImageDimension = Superclass::ImageDimension;

  /** Point type alias support */
  using typename Superclass::PointType;

  /** Index type alias support */
  using typename Superclass::IndexType;

  /** ContinuousIndex type alias support */
  using typename Superclass::ContinuousIndexType;

  /** RealType type alias support */
  using RealType = typename NumericTraits<typename TInputImage::PixelType>::RealType;

  /** Extrapolate the image at a point position
   *
   * Returns the extrapolated image intensity at a
   * specified point position.
   */
  OutputType
  Evaluate(const PointType & point) const override
  {
    const ContinuousIndexType index =
      this->GetInputImage()->template TransformPhysicalPointToContinuousIndex<TCoordinate>(point);
    return (this->EvaluateAtContinuousIndex(index));
  }

  /** Extrapolate the image at a continuous index position
   *
   * Returns the extrapolated image intensity at a
   * specified point position.
   */
  OutputType
  EvaluateAtContinuousIndex(const ContinuousIndexType & index) const override = 0;

  /** Extrapolate the image at an index position.
   *
   * Returns the extrapolated image intensity at a
   * specified point position.
   */
  OutputType
  EvaluateAtIndex(const IndexType & index) const override = 0;

protected:
  ExtrapolateImageFunction() = default;
  ~ExtrapolateImageFunction() override = default;
};
} // end namespace itk

#endif
