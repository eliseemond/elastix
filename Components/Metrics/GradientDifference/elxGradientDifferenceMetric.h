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
#ifndef elxGradientDifferenceMetric_h
#define elxGradientDifferenceMetric_h

#include "elxIncludes.h" // include first to avoid MSVS warning
#include "itkGradientDifferenceImageToImageMetric2.h"

namespace elastix
{

/**
 * \class GradientDifferenceMetric
 * \brief An metric based on the itk::GradientDifferenceImageToImageMetric.
 *
 *
 * \ingroup Metrics
 *
 */

template <class TElastix>
class ITK_TEMPLATE_EXPORT GradientDifferenceMetric
  : public itk::GradientDifferenceImageToImageMetric<typename MetricBase<TElastix>::FixedImageType,
                                                     typename MetricBase<TElastix>::MovingImageType>
  , public MetricBase<TElastix>
{
public:
  /** Standard ITK-stuff. */
  typedef GradientDifferenceMetric Self;
  typedef itk::GradientDifferenceImageToImageMetric<typename MetricBase<TElastix>::FixedImageType,
                                                    typename MetricBase<TElastix>::MovingImageType>
                                        Superclass1;
  typedef MetricBase<TElastix>          Superclass2;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(GradientDifferenceMetric, itk::GradientDifferenceImageToImageMetric);

  /** Name of this class.
   * Use this name in the parameter file to select this specific metric. \n
   * example: <tt>(Metric "GradientDifference")</tt>\n
   */
  elxClassNameMacro("GradientDifference");

  /** Typedefs from the superclass. */
  using typename Superclass1::CoordinateRepresentationType;
  using typename Superclass1::MovingImageType;
  using typename Superclass1::MovingImagePixelType;
  using typename Superclass1::MovingImageConstPointer;
  using typename Superclass1::FixedImageType;
  using typename Superclass1::FixedImageConstPointer;
  using typename Superclass1::FixedImageRegionType;
  using typename Superclass1::TransformType;
  using typename Superclass1::TransformPointer;
  using typename Superclass1::InputPointType;
  using typename Superclass1::OutputPointType;
  using typename Superclass1::TransformParametersType;
  using typename Superclass1::TransformJacobianType;
  using typename Superclass1::InterpolatorType;
  using typename Superclass1::InterpolatorPointer;
  using typename Superclass1::RealType;
  using typename Superclass1::GradientPixelType;
  using typename Superclass1::GradientImageType;
  using typename Superclass1::GradientImagePointer;
  using typename Superclass1::GradientImageFilterType;
  using typename Superclass1::GradientImageFilterPointer;
  using typename Superclass1::FixedImageMaskType;
  using typename Superclass1::FixedImageMaskPointer;
  using typename Superclass1::MovingImageMaskType;
  using typename Superclass1::MovingImageMaskPointer;
  using typename Superclass1::MeasureType;
  using typename Superclass1::DerivativeType;
  using typename Superclass1::ParametersType;
  using typename Superclass1::FixedImagePixelType;
  using typename Superclass1::MovingImageRegionType;
  using typename Superclass1::ImageSamplerType;
  using typename Superclass1::ImageSamplerPointer;
  using typename Superclass1::ImageSampleContainerType;
  using typename Superclass1::ImageSampleContainerPointer;
  using typename Superclass1::FixedImageLimiterType;
  using typename Superclass1::MovingImageLimiterType;
  using typename Superclass1::FixedImageLimiterOutputType;
  using typename Superclass1::MovingImageLimiterOutputType;
  using typename Superclass1::MovingImageDerivativeScalesType;

  /** The fixed image dimension. */
  itkStaticConstMacro(FixedImageDimension, unsigned int, FixedImageType::ImageDimension);

  /** The moving image dimension. */
  itkStaticConstMacro(MovingImageDimension, unsigned int, MovingImageType::ImageDimension);

  /** Typedef's inherited from Elastix. */
  using typename Superclass2::ElastixType;
  using typename Superclass2::ElastixPointer;
  using typename Superclass2::ConfigurationType;
  using typename Superclass2::ConfigurationPointer;
  using typename Superclass2::RegistrationType;
  using typename Superclass2::RegistrationPointer;
  typedef typename Superclass2::ITKBaseType ITKBaseType;

  /** Sets up a timer to measure the initialization time and
   * calls the Superclass' implementation.
   */
  void
  Initialize(void) override;

  /**
   * Do some things before each resolution:
   * \li Set CheckNumberOfSamples setting
   * \li Set UseNormalization setting
   */

  void
  BeforeRegistration(void) override;

  void
  BeforeEachResolution(void) override;

protected:
  /** The constructor. */
  GradientDifferenceMetric() = default;

  /** The destructor. */
  ~GradientDifferenceMetric() override = default;

private:
  elxOverrideGetSelfMacro;

  /** The deleted copy constructor. */
  GradientDifferenceMetric(const Self &) = delete;
  /** The deleted assignment operator. */
  void
  operator=(const Self &) = delete;
};

} // end namespace elastix

#ifndef ITK_MANUAL_INSTANTIATION
#  include "elxGradientDifferenceMetric.hxx"
#endif

#endif // end #ifndef elxGradientDifferenceMetric_h
