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
#ifndef itkHessianToObjectnessMeasureImageFilter_hxx
#define itkHessianToObjectnessMeasureImageFilter_hxx

#include "itkImageRegionIterator.h"
#include "itkSymmetricEigenAnalysis.h"
#include "itkProgressReporter.h"
#include "itkTotalProgressReporter.h"

#include "itkMath.h"

#include <algorithm>

namespace itk
{

template <typename TInputImage, typename TOutputImage>
HessianToObjectnessMeasureImageFilter<TInputImage, TOutputImage>::HessianToObjectnessMeasureImageFilter()

{
  this->DynamicMultiThreadingOn();
  this->ThreaderUpdateProgressOff();
}

template <typename TInputImage, typename TOutputImage>
void
HessianToObjectnessMeasureImageFilter<TInputImage, TOutputImage>::VerifyPreconditions() const
{
  Superclass::VerifyPreconditions();
  if (m_ObjectDimension >= ImageDimension)
  {
    itkExceptionMacro("ObjectDimension must be lower than ImageDimension.");
  }
}

template <typename TInputImage, typename TOutputImage>
void
HessianToObjectnessMeasureImageFilter<TInputImage, TOutputImage>::DynamicThreadedGenerateData(
  const OutputImageRegionType & outputRegionForThread)
{
  OutputImageType *      output = this->GetOutput();
  const InputImageType * input = this->GetInput();

  TotalProgressReporter progress(this, output->GetRequestedRegion().GetNumberOfPixels(), 1000);

  // Calculator for computation of the eigen values
  using CalculatorType = SymmetricEigenAnalysisFixedDimension<ImageDimension, InputPixelType, EigenValueArrayType>;
  const CalculatorType eigenCalculator;

  // Walk the region of eigen values and get the objectness measure
  ImageRegionConstIterator<InputImageType> it(input, outputRegionForThread);
  ImageRegionIterator<OutputImageType>     oit(output, outputRegionForThread);

  while (!it.IsAtEnd())
  {
    // Compute eigen values
    EigenValueArrayType eigenValues;
    eigenCalculator.ComputeEigenValues(it.Get(), eigenValues);

    // Sort the eigenvalues by magnitude but retain their sign.
    // The eigenvalues are to be sorted |e1|<=|e2|<=...<=|eN|
    EigenValueArrayType sortedEigenValues = eigenValues;
    std::sort(sortedEigenValues.Begin(), sortedEigenValues.End(), AbsLessCompare());

    // Check whether eigenvalues have the right sign
    bool signConstraintsSatisfied = true;
    for (unsigned int i = m_ObjectDimension; i < ImageDimension; ++i)
    {
      if ((m_BrightObject && sortedEigenValues[i] > 0.0) || (!m_BrightObject && sortedEigenValues[i] < 0.0))
      {
        signConstraintsSatisfied = false;
        break;
      }
    }

    if (!signConstraintsSatisfied)
    {
      oit.Set(OutputPixelType{});
      ++it;
      ++oit;
      progress.CompletedPixel();
      continue;
    }

    EigenValueArrayType sortedAbsEigenValues;
    for (unsigned int i = 0; i < ImageDimension; ++i)
    {
      sortedAbsEigenValues[i] = itk::Math::abs(sortedEigenValues[i]);
    }

    // Initialize the objectness measure
    double objectnessMeasure = 1.0;

    // Compute objectness from eigenvalue ratios and second-order structureness
    if (m_ObjectDimension < ImageDimension - 1)
    {
      double rA = sortedAbsEigenValues[m_ObjectDimension];
      double rADenominatorBase = 1.0;
      for (unsigned int j = m_ObjectDimension + 1; j < ImageDimension; ++j)
      {
        rADenominatorBase *= sortedAbsEigenValues[j];
      }
      if (itk::Math::abs(rADenominatorBase) > 0.0)
      {
        if (itk::Math::abs(m_Alpha) > 0.0)
        {
          rA /= std::pow(rADenominatorBase, 1.0 / (ImageDimension - m_ObjectDimension - 1));
          objectnessMeasure *= 1.0 - std::exp(-0.5 * itk::Math::sqr(rA) / itk::Math::sqr(m_Alpha));
        }
      }
      else
      {
        objectnessMeasure = 0.0;
      }
    }

    if (m_ObjectDimension > 0)
    {
      double rB = sortedAbsEigenValues[m_ObjectDimension - 1];
      double rBDenominatorBase = 1.0;
      for (unsigned int j = m_ObjectDimension; j < ImageDimension; ++j)
      {
        rBDenominatorBase *= sortedAbsEigenValues[j];
      }
      if (itk::Math::abs(rBDenominatorBase) > 0.0 && itk::Math::abs(m_Beta) > 0.0)
      {
        rB /= std::pow(rBDenominatorBase, 1.0 / (ImageDimension - m_ObjectDimension));

        objectnessMeasure *= std::exp(-0.5 * itk::Math::sqr(rB) / itk::Math::sqr(m_Beta));
      }
      else
      {
        objectnessMeasure = 0.0;
      }
    }

    if (itk::Math::abs(m_Gamma) > 0.0)
    {
      double frobeniusNormSquared = 0.0;
      for (unsigned int i = 0; i < ImageDimension; ++i)
      {
        frobeniusNormSquared += itk::Math::sqr(sortedAbsEigenValues[i]);
      }
      objectnessMeasure *= 1.0 - std::exp(-0.5 * frobeniusNormSquared / itk::Math::sqr(m_Gamma));
    }

    // Just in case, scale by largest absolute eigenvalue
    if (m_ScaleObjectnessMeasure)
    {
      objectnessMeasure *= sortedAbsEigenValues[ImageDimension - 1];
    }

    oit.Set(static_cast<OutputPixelType>(objectnessMeasure));

    ++it;
    ++oit;
    progress.CompletedPixel();
  }
}

template <typename TInputImage, typename TOutputImage>
void
HessianToObjectnessMeasureImageFilter<TInputImage, TOutputImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Alpha: " << m_Alpha << std::endl;
  os << indent << "Beta: " << m_Beta << std::endl;
  os << indent << "Gamma: " << m_Gamma << std::endl;
  os << indent << "ScaleObjectnessMeasure: " << m_ScaleObjectnessMeasure << std::endl;
  os << indent << "ObjectDimension: " << m_ObjectDimension << std::endl;
  os << indent << "BrightObject: " << m_BrightObject << std::endl;
}
} // end namespace itk

#endif
