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
#ifndef itkSumOfPairwiseCorrelationCoefficientsMetric_h
#define itkSumOfPairwiseCorrelationCoefficientsMetric_h

#include "itkAdvancedImageToImageMetric.h"

#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkImageRandomCoordinateSampler.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkExtractImageFilter.h"

namespace itk
{
template <class TFixedImage, class TMovingImage>
class ITK_TEMPLATE_EXPORT SumOfPairwiseCorrelationCoefficientsMetric
  : public AdvancedImageToImageMetric<TFixedImage, TMovingImage>
{
public:
  /** Standard class typedefs. */
  typedef SumOfPairwiseCorrelationCoefficientsMetric            Self;
  typedef AdvancedImageToImageMetric<TFixedImage, TMovingImage> Superclass;
  typedef SmartPointer<Self>                                    Pointer;
  typedef SmartPointer<const Self>                              ConstPointer;

  using typename Superclass::FixedImageRegionType;
  typedef typename FixedImageRegionType::SizeType FixedImageSizeType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(SumOfPairwiseCorrelationCoefficientsMetric, AdvancedImageToImageMetric);

  /** Set functions. */
  itkSetMacro(NumAdditionalSamplesFixed, unsigned int);
  itkSetMacro(ReducedDimensionIndex, unsigned int);
  itkSetMacro(SubtractMean, bool);
  itkSetMacro(GridSize, FixedImageSizeType);
  itkSetMacro(TransformIsStackTransform, bool);

  /** Typedefs from the superclass. */
  using typename Superclass::CoordinateRepresentationType;
  using typename Superclass::MovingImageType;
  using typename Superclass::MovingImagePixelType;
  using typename Superclass::MovingImageConstPointer;
  using typename Superclass::FixedImageType;
  using typename Superclass::FixedImageConstPointer;
  using typename Superclass::TransformType;
  using typename Superclass::TransformPointer;
  using typename Superclass::InputPointType;
  using typename Superclass::OutputPointType;
  using typename Superclass::TransformParametersType;
  using typename Superclass::TransformJacobianType;
  using typename Superclass::InterpolatorType;
  using typename Superclass::InterpolatorPointer;
  using typename Superclass::RealType;
  using typename Superclass::GradientPixelType;
  using typename Superclass::GradientImageType;
  using typename Superclass::GradientImagePointer;
  using typename Superclass::GradientImageFilterType;
  using typename Superclass::GradientImageFilterPointer;
  using typename Superclass::FixedImageMaskType;
  using typename Superclass::FixedImageMaskPointer;
  using typename Superclass::MovingImageMaskType;
  using typename Superclass::MovingImageMaskPointer;
  using typename Superclass::MeasureType;
  using typename Superclass::DerivativeType;
  using typename Superclass::ParametersType;
  using typename Superclass::FixedImagePixelType;
  using typename Superclass::MovingImageRegionType;
  using typename Superclass::ImageSamplerType;
  using typename Superclass::ImageSamplerPointer;
  using typename Superclass::ImageSampleContainerType;
  using typename Superclass::ImageSampleContainerPointer;
  using typename Superclass::FixedImageLimiterType;
  using typename Superclass::MovingImageLimiterType;
  using typename Superclass::FixedImageLimiterOutputType;
  using typename Superclass::MovingImageLimiterOutputType;
  using typename Superclass::MovingImageDerivativeScalesType;

  /** The fixed image dimension. */
  itkStaticConstMacro(FixedImageDimension, unsigned int, FixedImageType::ImageDimension);

  /** The moving image dimension. */
  itkStaticConstMacro(MovingImageDimension, unsigned int, MovingImageType::ImageDimension);

  /** Get the value for single valued optimizers. */
  MeasureType
  GetValue(const TransformParametersType & parameters) const override;

  /** Get the derivatives of the match measure. */
  void
  GetDerivative(const TransformParametersType & parameters, DerivativeType & derivative) const override;

  /** Get value and derivatives for multiple valued optimizers. */
  void
  GetValueAndDerivative(const TransformParametersType & parameters,
                        MeasureType &                   Value,
                        DerivativeType &                Derivative) const override;

  /** Initialize the Metric by making sure that all the components
   *  are present and plugged together correctly.
   * \li Call the superclass' implementation.   */

  void
  Initialize(void) override;

protected:
  SumOfPairwiseCorrelationCoefficientsMetric();
  ~SumOfPairwiseCorrelationCoefficientsMetric() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  /** Protected Typedefs ******************/

  /** Typedefs inherited from superclass */
  using typename Superclass::FixedImageIndexType;
  using typename Superclass::FixedImageIndexValueType;
  using typename Superclass::MovingImageIndexType;
  using typename Superclass::FixedImagePointType;
  typedef typename itk::ContinuousIndex<CoordinateRepresentationType, FixedImageDimension>
    FixedImageContinuousIndexType;
  using typename Superclass::MovingImagePointType;
  using typename Superclass::MovingImageContinuousIndexType;
  using typename Superclass::BSplineInterpolatorType;
  using typename Superclass::CentralDifferenceGradientFilterType;
  using typename Superclass::MovingImageDerivativeType;
  using typename Superclass::NonZeroJacobianIndicesType;

  /** Computes the innerproduct of transform Jacobian with moving image gradient.
   * The results are stored in imageJacobian, which is supposed
   * to have the right size (same length as Jacobian's number of columns). */
  void
  EvaluateTransformJacobianInnerProduct(const TransformJacobianType &     jacobian,
                                        const MovingImageDerivativeType & movingImageDerivative,
                                        DerivativeType &                  imageJacobian) const override;

private:
  SumOfPairwiseCorrelationCoefficientsMetric(const Self &) = delete;
  void
  operator=(const Self &) = delete;

  /** Sample n random numbers from 0..m and add them to the vector. */
  void
  SampleRandom(const int n, const int m, std::vector<int> & numbers) const;

  /** Variables to control random sampling in last dimension. */
  unsigned int m_NumAdditionalSamplesFixed;
  unsigned int m_ReducedDimensionIndex;

  /** Bool to determine if we want to subtract the mean derivate from the derivative elements. */
  bool m_SubtractMean{ true };

  /** GridSize of B-spline transform. */
  FixedImageSizeType m_GridSize;

  /** Bool to indicate if the transform used is a stacktransform. Set by elx files. */
  bool m_TransformIsStackTransform{ true };
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkSumOfPairwiseCorrelationCoefficientsMetric.hxx"
#endif

#endif // end #ifndef itkSumOfPairwiseCorrelationCoefficientsMetric_h
