/*=========================================================================
 *
 *  Copyright UMC Utrecht and contributors
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef elxLinearResampleInterpolator_h
#define elxLinearResampleInterpolator_h

#include "elxIncludes.h" // include first to avoid MSVS warning
#include "itkLinearInterpolateImageFunction.h"

namespace elastix
{

/**
 * \class LinearResampleInterpolator
 * \brief A linear resample-interpolator.
 *
 * Compared to the BSplineResampleInterpolator and BSplineResampleInterpolatorFloat
 * with SplineOrder 1 this class uses less (in fact, no) memory. You can select
 * this resample interpolator if memory burden is an issue and linear interpolation
 * is sufficient.
 *
 * The parameters used in this class are:
 * \parameter ResampleInterpolator: Select this resample interpolator as follows:\n
 *   <tt>(ResampleInterpolator "FinalLinearInterpolator")</tt>
 *
 * \ingroup ResampleInterpolators
 */

template <class TElastix>
class ITK_TEMPLATE_EXPORT LinearResampleInterpolator
  : public itk::LinearInterpolateImageFunction<typename ResampleInterpolatorBase<TElastix>::InputImageType,
                                               typename ResampleInterpolatorBase<TElastix>::CoordRepType>
  , public ResampleInterpolatorBase<TElastix>
{
public:
  /** Standard ITK-stuff. */
  typedef LinearResampleInterpolator Self;
  typedef itk::LinearInterpolateImageFunction<typename ResampleInterpolatorBase<TElastix>::InputImageType,
                                              typename ResampleInterpolatorBase<TElastix>::CoordRepType>
                                             Superclass1;
  typedef ResampleInterpolatorBase<TElastix> Superclass2;
  typedef itk::SmartPointer<Self>            Pointer;
  typedef itk::SmartPointer<const Self>      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(LinearResampleInterpolator, itk::LinearInterpolateImageFunction);

  /** Name of this class.
   * Use this name in the parameter file to select this specific resample interpolator. \n
   * example: <tt>(ResampleInterpolator "FinalLinearInterpolator")</tt>\n
   */
  elxClassNameMacro("FinalLinearInterpolator");

  /** Dimension of the image. */
  itkStaticConstMacro(ImageDimension, unsigned int, Superclass1::ImageDimension);

  /** Typedef's inherited from the superclass. */
  using typename Superclass1::OutputType;
  using typename Superclass1::InputImageType;
  using typename Superclass1::IndexType;
  using typename Superclass1::ContinuousIndexType;

  /** Typedef's from ResampleInterpolatorBase. */
  using typename Superclass2::ElastixType;
  using typename Superclass2::ElastixPointer;
  using typename Superclass2::ConfigurationType;
  using typename Superclass2::ConfigurationPointer;
  using typename Superclass2::RegistrationType;
  using typename Superclass2::RegistrationPointer;
  typedef typename Superclass2::ITKBaseType ITKBaseType;

protected:
  /** The constructor. */
  LinearResampleInterpolator() = default;
  /** The destructor. */
  ~LinearResampleInterpolator() override = default;

private:
  elxOverrideGetSelfMacro;

  /** The deleted copy constructor. */
  LinearResampleInterpolator(const Self &) = delete;
  /** The deleted assignment operator. */
  void
  operator=(const Self &) = delete;
};

} // end namespace elastix

#ifndef ITK_MANUAL_INSTANTIATION
#  include "elxLinearResampleInterpolator.hxx"
#endif

#endif // end elxLinearResampleInterpolator_h
