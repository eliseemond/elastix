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
#ifndef itkAdvancedBSplineDeformableTransformBase_h
#define itkAdvancedBSplineDeformableTransformBase_h

#include "itkAdvancedTransform.h"
#include "itkImage.h"
#include "itkImageRegion.h"

namespace itk
{

/** \class AdvancedBSplineDeformableTransformBase
 * \brief Base class for deformable transform using a B-spline representation
 *
 * This class is the base for the encapsulation of a deformable transform
 * of points from one N-dimensional one space to another N-dimensional space.
 *
 * This class is not templated over the spline order, which makes the use of
 * different spline orders more convenient in subsequent code.
 *
 */
template <class TScalarType = double, // Data type for scalars
          unsigned int NDimensions = 3>
// Number of dimensions
class ITK_TEMPLATE_EXPORT AdvancedBSplineDeformableTransformBase
  : public AdvancedTransform<TScalarType, NDimensions, NDimensions>
{
public:
  /** Standard class typedefs. */
  typedef AdvancedBSplineDeformableTransformBase                   Self;
  typedef AdvancedTransform<TScalarType, NDimensions, NDimensions> Superclass;
  typedef SmartPointer<Self>                                       Pointer;
  typedef SmartPointer<const Self>                                 ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(AdvancedBSplineDeformableTransformBase, AdvancedTransform);

  /** Dimension of the domain space. */
  itkStaticConstMacro(SpaceDimension, unsigned int, NDimensions);

  /** Typedefs from Superclass. */
  using typename Superclass::ParametersType;
  using typename Superclass::FixedParametersType;
  using typename Superclass::ParametersValueType;
  using typename Superclass::NumberOfParametersType;
  using typename Superclass::DerivativeType;
  using typename Superclass::JacobianType;
  using typename Superclass::ScalarType;
  using typename Superclass::InputPointType;
  using typename Superclass::OutputPointType;
  using typename Superclass::InputVectorType;
  using typename Superclass::OutputVectorType;
  using typename Superclass::InputVnlVectorType;
  using typename Superclass::OutputVnlVectorType;
  using typename Superclass::InputCovariantVectorType;
  using typename Superclass::OutputCovariantVectorType;
  using typename Superclass::TransformCategoryEnum;

  using typename Superclass::NonZeroJacobianIndicesType;
  using typename Superclass::SpatialJacobianType;
  using typename Superclass::JacobianOfSpatialJacobianType;
  using typename Superclass::SpatialHessianType;
  using typename Superclass::JacobianOfSpatialHessianType;
  using typename Superclass::InternalMatrixType;
  using typename Superclass::MovingImageGradientType;
  using typename Superclass::MovingImageGradientValueType;

  /** This method sets the parameters of the transform.
   * For a B-spline deformation transform, the parameters are the BSpline
   * coefficients on a sparse grid.
   *
   * The parameters are N number of N-D grid of coefficients. Each N-D grid
   * is represented as a flat array of doubles
   * (in the same configuration as an itk::Image).
   * The N arrays are then concatenated to form one parameter array.
   *
   * For efficiency, this transform does not make a copy of the parameters.
   * It only keeps a pointer to the input parameters. It assumes that the memory
   * is managed by the caller. Use SetParametersByValue to force the transform
   * to call copy the parameters.
   *
   * This method wraps each grid as itk::Image's using the user specified
   * grid region, spacing and origin.
   * NOTE: The grid region, spacing and origin must be set first.
   */
  void
  SetParameters(const ParametersType & parameters) override;

  /** This method sets the fixed parameters of the transform.
   * For a B-spline deformation transform, the parameters are the following:
   *    Grid Size, Grid Origin, and Grid Spacing
   *
   * The fixed parameters are the three times the size of the templated
   * dimensions.
   * This function has the effect of make the following calls:
   *       transform->SetGridSpacing( spacing );
   *       transform->SetGridOrigin( origin );
   *       transform->SetGridDirection( direction );
   *       transform->SetGridRegion( bsplineRegion );
   *
   * This function was added to allow the transform to work with the
   * itkTransformReader/Writer I/O filters.
   */
  void
  SetFixedParameters(const FixedParametersType & parameters) override;

  /** This method sets the parameters of the transform.
   * For a B-spline deformation transform, the parameters are the BSpline
   * coefficients on a sparse grid.
   *
   * The parameters are N number of N-D grid of coefficients. Each N-D grid
   * is represented as a flat array of doubles
   * (in the same configuration as an itk::Image).
   * The N arrays are then concatenated to form one parameter array.
   *
   * This methods makes a copy of the parameters while for
   * efficiency the SetParameters method does not.
   *
   * This method wraps each grid as itk::Image's using the user specified
   * grid region, spacing and origin.
   * NOTE: The grid region, spacing and origin must be set first.
   */
  void
  SetParametersByValue(const ParametersType & parameters) override;

  /** This method can ONLY be invoked AFTER calling SetParameters().
   *  This restriction is due to the fact that the AdvancedBSplineDeformableTransform
   *  does not copy the array of parameters internally, instead it keeps a
   *  pointer to the user-provided array of parameters. This method is also
   *  in violation of the const-correctness of the parameters since the
   *  parameter array has been passed to the transform on a 'const' basis but
   *  the values get modified when the user invokes SetIdentity().
   */
  void
  SetIdentity(void);

  /** Get the Transformation Parameters. */
  const ParametersType &
  GetParameters(void) const override;

  /** Get the Transformation Fixed Parameters. */
  const FixedParametersType &
  GetFixedParameters(void) const override;

  /** Parameters as SpaceDimension number of images. */
  typedef typename ParametersType::ValueType     PixelType;
  typedef Image<PixelType, Self::SpaceDimension> ImageType;
  typedef typename ImageType::Pointer            ImagePointer;

  /** Get the array of coefficient images. */
  virtual const ImagePointer *
  GetCoefficientImages(void) const
  {
    return this->m_CoefficientImages;
  }

  /** Set the array of coefficient images.
   *
   * This is an alternative API for setting the B-spline coefficients
   * as an array of SpaceDimension images. The grid region spacing
   * and origin is taken from the first image. It is assume that
   * the buffered region of all the subsequent images are the same
   * as the first image. Note that no error checking is done.
   *
   * Warning: use either the SetParameters() or SetCoefficientImages()
   * API. Mixing the two modes may results in unexpected results.
   */
  virtual void
  SetCoefficientImages(ImagePointer images[]);

  /** Typedefs for specifying the extend to the grid. */
  typedef ImageRegion<Self::SpaceDimension> RegionType;

  typedef typename RegionType::IndexType    IndexType;
  typedef typename RegionType::SizeType     SizeType;
  typedef typename ImageType::SpacingType   SpacingType;
  typedef typename ImageType::DirectionType DirectionType;
  typedef typename ImageType::PointType     OriginType;
  typedef IndexType                         GridOffsetType;

  /** This method specifies the region over which the grid resides. */
  virtual void
  SetGridRegion(const RegionType & region) = 0;

  itkGetConstMacro(GridRegion, RegionType);

  /** This method specifies the grid spacing or resolution. */
  virtual void
  SetGridSpacing(const SpacingType & spacing);

  itkGetConstMacro(GridSpacing, SpacingType);

  /** This method specifies the grid directions . */
  virtual void
  SetGridDirection(const DirectionType & direction);

  itkGetConstMacro(GridDirection, DirectionType);

  /** This method specifies the grid origin. */
  virtual void
  SetGridOrigin(const OriginType & origin);

  itkGetConstMacro(GridOrigin, OriginType);

  /** Parameter index array type. */
  typedef Array<unsigned long> ParameterIndexArrayType;

  /** Method to transform a vector -
   *  not applicable for this type of transform.
   */
  OutputVectorType
  TransformVector(const InputVectorType &) const override
  {
    itkExceptionMacro(<< "Method not applicable for deformable transform.");
  }


  /** Method to transform a vnl_vector -
   *  not applicable for this type of transform.
   */
  OutputVnlVectorType
  TransformVector(const InputVnlVectorType &) const override
  {
    itkExceptionMacro(<< "Method not applicable for deformable transform. ");
  }


  /** Method to transform a CovariantVector -
   *  not applicable for this type of transform.
   */
  OutputCovariantVectorType
  TransformCovariantVector(const InputCovariantVectorType &) const override
  {
    itkExceptionMacro(<< "Method not applicable for deformable transform. ");
  }


  /** Return the number of parameters that completely define the Transform. */
  NumberOfParametersType
  GetNumberOfParameters(void) const override;

  /** Return the number of parameters per dimension */
  virtual NumberOfParametersType
  GetNumberOfParametersPerDimension(void) const;

  /** Return the region of the grid wholly within the support region */
  itkGetConstReferenceMacro(ValidRegion, RegionType);

  /** Indicates that this transform is linear. That is, given two
   * points P and Q, and scalar coefficients a and b, then
   *
   *           T( a*P + b*Q ) = a * T(P) + b * T(Q)
   */
  bool
  IsLinear(void) const override
  {
    return false;
  }

  /** Indicates the category transform.
   *  e.g. an affine transform, or a local one, e.g. a deformation field.
   */
  TransformCategoryEnum
  GetTransformCategory(void) const override
  {
    return TransformCategoryEnum::BSpline;
  }


  virtual unsigned int
  GetNumberOfAffectedWeights(void) const = 0;

  NumberOfParametersType
  GetNumberOfNonZeroJacobianIndices(void) const override = 0;

  /** This typedef should be equal to the typedef used
   * in derived classes based on the weights function.
   */
  typedef ContinuousIndex<ScalarType, SpaceDimension> ContinuousIndexType;

protected:
  /** Print contents of an AdvancedBSplineDeformableTransformBase. */
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  AdvancedBSplineDeformableTransformBase();
  ~AdvancedBSplineDeformableTransformBase() override;

  /** Wrap flat array into images of coefficients. */
  void
  WrapAsImages(void);

  /** Convert an input point to a continuous index inside the B-spline grid. */
  void
  TransformPointToContinuousGridIndex(const InputPointType & point, ContinuousIndexType & index) const;

  void
  UpdatePointIndexConversions(void);

  virtual void
  ComputeNonZeroJacobianIndices(NonZeroJacobianIndicesType & nonZeroJacobianIndices,
                                const RegionType &           supportRegion) const = 0;

  /** Check if a continuous index is inside the valid region. */
  virtual bool
  InsideValidRegion(const ContinuousIndexType & index) const;

  /** Array of images representing the B-spline coefficients
   *  in each dimension.
   */
  ImagePointer m_CoefficientImages[NDimensions];

  /** Variables defining the coefficient grid extend. */
  RegionType     m_GridRegion;
  SpacingType    m_GridSpacing;
  DirectionType  m_GridDirection;
  OriginType     m_GridOrigin;
  GridOffsetType m_GridOffsetTable;

  DirectionType                                     m_PointToIndexMatrix;
  SpatialJacobianType                               m_PointToIndexMatrix2;
  DirectionType                                     m_PointToIndexMatrixTransposed;
  SpatialJacobianType                               m_PointToIndexMatrixTransposed2;
  FixedArray<ScalarType, NDimensions>               m_PointToIndexMatrixDiagonal;
  FixedArray<ScalarType, NDimensions * NDimensions> m_PointToIndexMatrixDiagonalProducts;
  DirectionType                                     m_IndexToPoint;
  bool                                              m_PointToIndexMatrixIsDiagonal;

  RegionType m_ValidRegion;

  /** Variables defining the interpolation support region. */
  unsigned long       m_Offset;
  SizeType            m_SupportSize;
  ContinuousIndexType m_ValidRegionBegin;
  ContinuousIndexType m_ValidRegionEnd;

  /** Odd or even order B-spline. */
  bool m_SplineOrderOdd;

  /** Keep a pointer to the input parameters. */
  const ParametersType * m_InputParametersPointer;

  /** Jacobian as SpaceDimension number of images. */
  typedef typename JacobianType::ValueType               JacobianPixelType;
  typedef Image<JacobianPixelType, Self::SpaceDimension> JacobianImageType;

  typename JacobianImageType::Pointer m_JacobianImage[NDimensions];

  /** Keep track of last support region used in computing the Jacobian
   * for fast resetting of Jacobian to zero.
   */
  mutable IndexType m_LastJacobianIndex;

  /** Array holding images wrapped from the flat parameters. */
  ImagePointer m_WrappedImage[NDimensions];

  /** Internal parameters buffer. */
  ParametersType m_InternalParametersBuffer;

  void
  UpdateGridOffsetTable(void);

private:
  AdvancedBSplineDeformableTransformBase(const Self &) = delete;
  void
  operator=(const Self &) = delete;
};

} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkAdvancedBSplineDeformableTransformBase.hxx"
#endif

#endif /* itkAdvancedBSplineDeformableTransformBase_h */
