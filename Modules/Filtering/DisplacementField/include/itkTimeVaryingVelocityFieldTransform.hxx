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
#ifndef itkTimeVaryingVelocityFieldTransform_hxx
#define itkTimeVaryingVelocityFieldTransform_hxx


#include "itkTimeVaryingVelocityFieldIntegrationImageFilter.h"

namespace itk
{

template <typename TParametersValueType, unsigned int VDimension>
void
TimeVaryingVelocityFieldTransform<TParametersValueType, VDimension>::IntegrateVelocityField()
{
  if (this->GetVelocityField())
  {
    using IntegratorType = TimeVaryingVelocityFieldIntegrationImageFilter<VelocityFieldType, DisplacementFieldType>;

    auto integrator = IntegratorType::New();
    integrator->SetInput(this->GetVelocityField());
    integrator->SetLowerTimeBound(this->GetLowerTimeBound());
    integrator->SetUpperTimeBound(this->GetUpperTimeBound());

    if (this->GetVelocityFieldInterpolator())
    {
      integrator->SetVelocityFieldInterpolator(this->GetModifiableVelocityFieldInterpolator());
    }

    integrator->SetNumberOfIntegrationSteps(this->GetNumberOfIntegrationSteps());
    integrator->Update();

    const typename DisplacementFieldType::Pointer displacementField = integrator->GetOutput();
    displacementField->DisconnectPipeline();

    this->SetDisplacementField(displacementField);
    this->GetModifiableInterpolator()->SetInputImage(displacementField);

    auto inverseIntegrator = IntegratorType::New();
    inverseIntegrator->SetInput(this->GetVelocityField());
    inverseIntegrator->SetLowerTimeBound(this->GetUpperTimeBound());
    inverseIntegrator->SetUpperTimeBound(this->GetLowerTimeBound());
    if (!this->GetVelocityFieldInterpolator())
    {
      inverseIntegrator->SetVelocityFieldInterpolator(this->GetModifiableVelocityFieldInterpolator());
    }

    inverseIntegrator->SetNumberOfIntegrationSteps(this->GetNumberOfIntegrationSteps());
    inverseIntegrator->Update();

    const typename DisplacementFieldType::Pointer inverseDisplacementField = inverseIntegrator->GetOutput();
    inverseDisplacementField->DisconnectPipeline();

    this->SetInverseDisplacementField(inverseDisplacementField);
  }
  else
  {
    itkExceptionMacro("The velocity field does not exist.");
  }
}

} // namespace itk

#endif
