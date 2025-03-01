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
#ifndef itkANNPriorityTreeSearch_h
#define itkANNPriorityTreeSearch_h

#include "itkBinaryANNTreeSearchBase.h"

namespace itk
{

/**
 * \class ANNPriorityTreeSearch
 *
 * \brief
 *
 *
 * \ingroup ANNwrap
 */

template <class TListSample>
class ITK_TEMPLATE_EXPORT ANNPriorityTreeSearch : public BinaryANNTreeSearchBase<TListSample>
{
public:
  /** Standard itk. */
  typedef ANNPriorityTreeSearch                Self;
  typedef BinaryANNTreeSearchBase<TListSample> Superclass;
  typedef SmartPointer<Self>                   Pointer;
  typedef SmartPointer<const Self>             ConstPointer;

  /** New method for creating an object using a factory. */
  itkNewMacro(Self);

  /** ITK type info. */
  itkTypeMacro(ANNPriorityTreeSearch, BinaryANNTreeSearchBase);

  /** Typedef's from Superclass. */
  using typename Superclass::ListSampleType;
  using typename Superclass::BinaryTreeType;
  using typename Superclass::MeasurementVectorType;
  using typename Superclass::IndexArrayType;
  using typename Superclass::DistanceArrayType;

  using typename Superclass::ANNPointType;         // double *
  using typename Superclass::ANNIndexType;         // int
  using typename Superclass::ANNIndexArrayType;    // int *
  using typename Superclass::ANNDistanceType;      // double
  using typename Superclass::ANNDistanceArrayType; // double *

  using typename Superclass::BinaryANNTreeType;

  /** Typedefs for casting to kd tree. */
  typedef ANNkd_tree  ANNkDTreeType;
  typedef ANNpointSet ANNPointSetType;

  /** Set and get the error bound eps. */
  itkSetClampMacro(ErrorBound, double, 0.0, 1e14);
  itkGetConstMacro(ErrorBound, double);

  /** Search the nearest neighbours of a query point qp. */
  void
  Search(const MeasurementVectorType & qp, IndexArrayType & ind, DistanceArrayType & dists) override;

  void
  SetBinaryTree(BinaryTreeType * tree) override;

protected:
  ANNPriorityTreeSearch();
  ~ANNPriorityTreeSearch() override;

  /** Member variables. */
  double          m_ErrorBound;
  ANNkDTreeType * m_BinaryTreeAskDTree;

private:
  ANNPriorityTreeSearch(const Self &) = delete;
  void
  operator=(const Self &) = delete;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkANNPriorityTreeSearch.hxx"
#endif

#endif // end #ifndef itkANNPriorityTreeSearch_h
