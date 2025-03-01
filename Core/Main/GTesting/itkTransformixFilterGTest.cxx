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

#define _USE_MATH_DEFINES // For M_PI_4.

// First include the header file to be tested:
#include <itkTransformixFilter.h>
#include <itkParameterFileParser.h>

#include "elxCoreMainGTestUtilities.h"
#include "GTesting/elxGTestUtilities.h"

// ITK header files:
#include <itkAffineTransform.h>
#include <itkBSplineTransform.h>
#include <itkCompositeTransform.h>
#include <itkEuler2DTransform.h>
#include <itkEuler3DTransform.h>
#include <itkImage.h>
#include <itkImageBufferRange.h>
#include <itkNumberToString.h>
#include <itkResampleImageFilter.h>
#include <itkSimilarity2DTransform.h>
#include <itkSimilarity3DTransform.h>
#include <itkTranslationTransform.h>

// GoogleTest header file:
#include <gtest/gtest.h>

#include <algorithm> // For equal and transform.
#include <cmath>
#include <map>
#include <random>
#include <string>


using ParameterMapType = itk::ParameterFileParser::ParameterMapType;
using ParameterValuesType = itk::ParameterFileParser::ParameterValuesType;

template <typename TPixel, unsigned VImageDimension>
using ResampleImageFilterType =
  itk::ResampleImageFilter<itk::Image<TPixel, VImageDimension>, itk::Image<TPixel, VImageDimension>>;

// Using-declarations:
using elx::CoreMainGTestUtilities::CheckNew;
using elx::CoreMainGTestUtilities::CreateImageFilledWithSequenceOfNaturalNumbers;
using elx::CoreMainGTestUtilities::Deref;
using elx::CoreMainGTestUtilities::DerefSmartPointer;
using elx::CoreMainGTestUtilities::FillImageRegion;
using elx::CoreMainGTestUtilities::GetDataDirectoryPath;
using elx::GTestUtilities::GeneratePseudoRandomParameters;
using elx::GTestUtilities::MakePoint;
using elx::GTestUtilities::MakeSize;
using elx::GTestUtilities::MakeVector;


namespace
{
template <unsigned NDimension>
itk::Vector<double, NDimension>
ConvertToItkVector(const itk::Size<NDimension> & size)
{
  itk::Vector<double, NDimension> result;
  std::copy_n(size.begin(), NDimension, result.begin());
  return result;
}


elx::ParameterObject::Pointer
CreateParameterObject(const ParameterMapType & parameterMap)
{
  const auto parameterObject = elx::ParameterObject::New();
  parameterObject->SetParameterMap(parameterMap);
  return parameterObject;
}


template <unsigned ImageDimension>
ParameterValuesType
CreateDefaultDirectionParameterValues()
{
  constexpr auto      numberOfValues = ImageDimension * ImageDimension;
  ParameterValuesType values(numberOfValues, "0");

  for (std::size_t i{}; i < numberOfValues; i += (ImageDimension + 1))
  {
    values[i] = "1";
  }
  return values;
}


template <typename T>
ParameterValuesType
ConvertToParameterValues(const T & container)
{
  ParameterValuesType parameterValues(container.size());
  std::transform(std::begin(container),
                 std::end(container),
                 parameterValues.begin(),
                 [](decltype(*std::begin(container)) inputValue) { return itk::NumberToString<double>{}(inputValue); });
  return parameterValues;
}


// Translates an image by the specified offset, using itk::TransformixFilter,
// specifying "TranslationTransform" as Transform.
template <typename TImage>
itk::SmartPointer<TImage>
TranslateImage(TImage & image, const typename TImage::OffsetType & translationOffset)
{
  constexpr auto ImageDimension = TImage::ImageDimension;

  const auto filter = itk::TransformixFilter<TImage>::New();

  filter->SetMovingImage(&image);
  filter->SetTransformParameterObject(
    CreateParameterObject({ // Parameters in alphabetic order:
                            { "Direction", CreateDefaultDirectionParameterValues<ImageDimension>() },
                            { "Index", ParameterValuesType(ImageDimension, "0") },
                            { "NumberOfParameters", { std::to_string(ImageDimension) } },
                            { "Origin", ParameterValuesType(ImageDimension, "0") },
                            { "ResampleInterpolator", { "FinalLinearInterpolator" } },
                            { "Size", ConvertToParameterValues(image.GetRequestedRegion().GetSize()) },
                            { "Transform", ParameterValuesType{ "TranslationTransform" } },
                            { "TransformParameters", ConvertToParameterValues(translationOffset) },
                            { "Spacing", ParameterValuesType(ImageDimension, "1") } }));
  filter->Update();

  return &Deref(filter->GetOutput());
}


template <typename TPixel, unsigned int VImageDimension>
void
ExpectEqualImages(const itk::Image<TPixel, VImageDimension> & actualImage,
                  const itk::Image<TPixel, VImageDimension> & expectedImage)
{
  EXPECT_EQ(actualImage, expectedImage);
}


template <typename TImage>
bool
ImageBuffer_has_nonzero_pixel_values(const TImage & image)
{
  const itk::ImageBufferRange<const TImage> imageBufferRange(image);
  return std::any_of(imageBufferRange.cbegin(),
                     imageBufferRange.cend(),
                     [](const typename TImage::PixelType pixelValue) { return pixelValue != 0; });
}


template <typename TPixel, unsigned VImageDimension>
itk::SmartPointer<itk::TransformixFilter<itk::Image<TPixel, VImageDimension>>>
CreateTransformixFilter(itk::Image<TPixel, VImageDimension> &                            image,
                        const itk::Transform<double, VImageDimension, VImageDimension> & itkTransform,
                        const std::string & initialTransformParametersFileName = "NoInitialTransform",
                        const std::string & howToCombineTransforms = "Compose")
{
  const auto filter = CheckNew<itk::TransformixFilter<itk::Image<TPixel, VImageDimension>>>();
  filter->SetMovingImage(&image);

  std::string transformName = itkTransform.GetNameOfClass();

  const auto dimensionPosition = transformName.find(std::to_string(VImageDimension) + "DTransform");
  if (dimensionPosition != std::string::npos)
  {
    // Erase "2D" or "3D".
    transformName.erase(dimensionPosition, 2);
  }

  filter->SetTransformParameterObject(CreateParameterObject(
    { // Parameters in alphabetic order:
      { "Direction", CreateDefaultDirectionParameterValues<VImageDimension>() },
      { "HowToCombineTransforms", { howToCombineTransforms } },
      { "Index", ParameterValuesType(VImageDimension, "0") },
      { "InitialTransformParametersFileName", { initialTransformParametersFileName } },
      { "ITKTransformParameters", ConvertToParameterValues(itkTransform.GetParameters()) },
      { "ITKTransformFixedParameters", ConvertToParameterValues(itkTransform.GetFixedParameters()) },
      { "Origin", ParameterValuesType(VImageDimension, "0") },
      { "ResampleInterpolator", { "FinalLinearInterpolator" } },
      { "Size", ConvertToParameterValues(image.GetBufferedRegion().GetSize()) },
      { "Transform", { transformName } },
      { "Spacing", ParameterValuesType(VImageDimension, "1") } }));
  filter->Update();
  return filter;
}


template <typename TPixel, unsigned VImageDimension>
itk::SmartPointer<itk::Image<TPixel, VImageDimension>>
RetrieveOutputFromTransformixFilter(itk::Image<TPixel, VImageDimension> &                            image,
                                    const itk::Transform<double, VImageDimension, VImageDimension> & itkTransform,
                                    const std::string & initialTransformParametersFileName = "NoInitialTransform",
                                    const std::string & howToCombineTransforms = "Compose")
{
  const auto transformixFilter =
    CreateTransformixFilter(image, itkTransform, initialTransformParametersFileName, howToCombineTransforms);
  const auto output = transformixFilter->GetOutput();
  EXPECT_NE(output, nullptr);
  return output;
}


template <typename TPixel, unsigned VImageDimension>
itk::SmartPointer<ResampleImageFilterType<TPixel, VImageDimension>>
CreateResampleImageFilter(const itk::Image<TPixel, VImageDimension> &                      image,
                          const itk::Transform<double, VImageDimension, VImageDimension> & itkTransform)
{
  const auto filter = ResampleImageFilterType<TPixel, VImageDimension>::New();
  filter->SetInput(&image);
  filter->SetTransform(&itkTransform);
  filter->SetSize(image.GetBufferedRegion().GetSize());
  filter->Update();
  return filter;
}


template <typename TPixel, unsigned VImageDimension>
void
Expect_TransformixFilter_output_equals_ResampleImageFilter_output(
  itk::Image<TPixel, VImageDimension> &                            inputImage,
  const itk::Transform<double, VImageDimension, VImageDimension> & itkTransform)
{
  const auto resampleImageFilter = CreateResampleImageFilter(inputImage, itkTransform);
  const auto transformixFilter = CreateTransformixFilter(inputImage, itkTransform);

  const auto & resampleImageFilterOutput = Deref(DerefSmartPointer(resampleImageFilter).GetOutput());
  const auto & transformixFilterOutput = Deref(DerefSmartPointer(transformixFilter).GetOutput());

  // Check that the ResampleImageFilter output isn't equal to the input image,
  // otherwise the test itself would be less interesting.
  EXPECT_NE(resampleImageFilterOutput, inputImage);

  // Check that the output is not simply a black image, otherwise the test
  // itself would be less interesting.
  EXPECT_TRUE(ImageBuffer_has_nonzero_pixel_values(transformixFilterOutput));

  ExpectEqualImages(transformixFilterOutput, resampleImageFilterOutput);
}


// Creates a transform of the specified (typically derived) type, and implicitly converts to an itk::Transform pointer.
template <typename TTransform>
itk::SmartPointer<itk::Transform<typename TTransform::ParametersValueType,
                                 TTransform::InputSpaceDimension,
                                 TTransform::OutputSpaceDimension>>
CreateTransform()
{
  return TTransform::New();
}


// Creates a matrix-and-offset-transform of the specified (typically derived) type, and implicitly converts to
// an itk::MatrixOffsetTransformBase pointer.
template <typename TTransform>
itk::SmartPointer<itk::MatrixOffsetTransformBase<typename TTransform::ParametersValueType,
                                                 TTransform::InputSpaceDimension,
                                                 TTransform::OutputSpaceDimension>>
CreateMatrixOffsetTransform()
{
  return TTransform::New();
}

template <typename TImage>
void
ExpectAlmostEqualPixelValues(const TImage & actualImage, const TImage & expectedImage, const double tolerance)
{
  // Expect the specified tolerance value to be greater than zero, otherwise
  // `ExpectEqualImages` should have been called instead.
  EXPECT_GT(tolerance, 0.0);

  using ImageBufferRangeType = itk::ImageBufferRange<const TImage>;

  const ImageBufferRangeType actualImageBufferRange(actualImage);
  const ImageBufferRangeType expectedImageBufferRange(expectedImage);

  ASSERT_EQ(actualImageBufferRange.size(), expectedImageBufferRange.size());

  const auto beginOfExpectedImageBuffer = expectedImageBufferRange.cbegin();

  // First expect that _not_ all pixel values are not _exactly_ equal,
  // otherwise `ExpectEqualImages` should probably have been called instead!
  EXPECT_FALSE(std::equal(actualImageBufferRange.cbegin(), actualImageBufferRange.cend(), beginOfExpectedImageBuffer));

  auto expectedImageIterator = beginOfExpectedImageBuffer;

  const itk::ZeroBasedIndexRange<TImage::ImageDimension> indexRange(actualImage.GetBufferedRegion().GetSize());
  auto                                                   indexIterator = indexRange.cbegin();
  for (const typename TImage::PixelType actualPixelValue : actualImageBufferRange)
  {
    EXPECT_LE(std::abs(actualPixelValue - *expectedImageIterator), tolerance)
      << " actualPixelValue = " << actualPixelValue << "; expectedPixelValue = " << *expectedImageIterator
      << " index = " << *indexIterator;
    ++expectedImageIterator;
    ++indexIterator;
  }
}

} // namespace


// Tests translating a small (5x6) binary image, having a 2x2 white square.
GTEST_TEST(itkTransformixFilter, Translation2D)
{
  constexpr auto ImageDimension = 2U;
  using ImageType = itk::Image<float, ImageDimension>;
  using SizeType = itk::Size<ImageDimension>;

  const itk::Offset<ImageDimension> translationOffset{ { 1, -2 } };
  const auto                        regionSize = SizeType::Filled(2);
  const SizeType                    imageSize{ { 5, 6 } };
  const itk::Index<ImageDimension>  fixedImageRegionIndex{ { 1, 3 } };

  const auto fixedImage = ImageType::New();
  fixedImage->SetRegions(imageSize);
  fixedImage->Allocate(true);
  FillImageRegion(*fixedImage, fixedImageRegionIndex, regionSize);

  const auto movingImage = ImageType::New();
  movingImage->SetRegions(imageSize);
  movingImage->Allocate(true);
  FillImageRegion(*movingImage, fixedImageRegionIndex + translationOffset, regionSize);

  const auto transformedImage = TranslateImage(*movingImage, translationOffset);

  ExpectEqualImages(*transformedImage, *fixedImage);
}


// Tests translating a small (5x7x9) binary 3D image, having a 2x2x2 white cube.
GTEST_TEST(itkTransformixFilter, Translation3D)
{
  constexpr auto ImageDimension = 3U;
  using ImageType = itk::Image<float, ImageDimension>;
  using SizeType = itk::Size<ImageDimension>;

  const itk::Offset<ImageDimension> translationOffset{ { 1, 2, 3 } };
  const auto                        regionSize = SizeType::Filled(2);
  const SizeType                    imageSize{ { 5, 7, 9 } };
  const itk::Index<ImageDimension>  fixedImageRegionIndex{ { 1, 2, 3 } };

  const auto fixedImage = ImageType::New();
  fixedImage->SetRegions(imageSize);
  fixedImage->Allocate(true);
  FillImageRegion(*fixedImage, fixedImageRegionIndex, regionSize);

  const auto movingImage = ImageType::New();
  movingImage->SetRegions(imageSize);
  movingImage->Allocate(true);
  FillImageRegion(*movingImage, fixedImageRegionIndex + translationOffset, regionSize);

  const auto transformedImage = TranslateImage(*movingImage, translationOffset);

  ExpectEqualImages(*transformedImage, *fixedImage);
}


GTEST_TEST(itkTransformixFilter, TranslationViaExternalTransformFile)
{
  constexpr auto ImageDimension = 2U;
  using ImageType = itk::Image<float, ImageDimension>;
  using SizeType = itk::Size<ImageDimension>;

  const itk::Offset<ImageDimension> translationOffset{ { 1, -2 } };
  const auto                        regionSize = SizeType::Filled(2);
  const SizeType                    imageSize{ { 5, 6 } };
  const itk::Index<ImageDimension>  fixedImageRegionIndex{ { 1, 3 } };

  const auto movingImage = ImageType::New();
  movingImage->SetRegions(imageSize);
  movingImage->Allocate(true);
  FillImageRegion(*movingImage, fixedImageRegionIndex + translationOffset, regionSize);

  const auto expectedOutputImage = TranslateImage(*movingImage, translationOffset);

  for (const std::string transformFileName :
       { "ITK-Transform.tfm", "ITK-HDF5-Transform.h5", "Special characters [(0-9,;!@#$%&)]/ITK-Transform.tfm" })
  {
    const auto transformFilePathName = GetDataDirectoryPath() + "/Translation(1,-2)/" + transformFileName;

    const auto filter = itk::TransformixFilter<ImageType>::New();
    ASSERT_NE(filter, nullptr);

    filter->SetMovingImage(movingImage);
    filter->SetTransformParameterObject(
      CreateParameterObject({ // Parameters in alphabetic order:
                              { "Direction", CreateDefaultDirectionParameterValues<ImageDimension>() },
                              { "Index", ParameterValuesType(ImageDimension, "0") },
                              { "Origin", ParameterValuesType(ImageDimension, "0") },
                              { "ResampleInterpolator", { "FinalLinearInterpolator" } },
                              { "Size", ConvertToParameterValues(imageSize) },
                              { "Transform", ParameterValuesType{ "File" } },
                              { "TransformFileName", { transformFilePathName } },
                              { "Spacing", ParameterValuesType(ImageDimension, "1") } }));
    filter->Update();
    const auto * const outputImage = filter->GetOutput();
    ExpectEqualImages(Deref(outputImage), *expectedOutputImage);
  }
}


GTEST_TEST(itkTransformixFilter, ITKTranslationTransform2D)
{
  constexpr auto ImageDimension = 2U;

  const auto itkTransform = itk::TranslationTransform<double, ImageDimension>::New();
  itkTransform->SetOffset(MakeVector(1.0, -2.0));

  Expect_TransformixFilter_output_equals_ResampleImageFilter_output(
    *CreateImageFilledWithSequenceOfNaturalNumbers<float>(MakeSize(5, 6)), *itkTransform);
}


GTEST_TEST(itkTransformixFilter, ITKTranslationTransform3D)
{
  constexpr auto ImageDimension = 3U;

  const auto itkTransform = itk::TranslationTransform<double, ImageDimension>::New();
  itkTransform->SetOffset(MakeVector(1.0, -2.0, 3.0));

  Expect_TransformixFilter_output_equals_ResampleImageFilter_output(
    *CreateImageFilledWithSequenceOfNaturalNumbers<float>(MakeSize(5, 6, 7)), *itkTransform);
}


GTEST_TEST(itkTransformixFilter, ITKAffineTransform2D)
{
  constexpr auto ImageDimension = 2U;

  const auto itkTransform = itk::AffineTransform<double, ImageDimension>::New();
  itkTransform->SetTranslation(MakeVector(1.0, -2.0));
  itkTransform->SetCenter(MakePoint(2.5, 3.0));
  itkTransform->Rotate2D(M_PI_4);

  Expect_TransformixFilter_output_equals_ResampleImageFilter_output(
    *CreateImageFilledWithSequenceOfNaturalNumbers<float>(MakeSize(5, 6)), *itkTransform);
}


GTEST_TEST(itkTransformixFilter, ITKAffineTransform3D)
{
  constexpr auto ImageDimension = 3U;

  const auto itkTransform = itk::AffineTransform<double, ImageDimension>::New();
  itkTransform->SetTranslation(MakeVector(1.0, 2.0, 3.0));
  itkTransform->SetCenter(MakePoint(3.0, 2.0, 1.0));
  itkTransform->Rotate3D(itk::Vector<double, ImageDimension>(1.0), M_PI_4);

  Expect_TransformixFilter_output_equals_ResampleImageFilter_output(
    *CreateImageFilledWithSequenceOfNaturalNumbers<float>(MakeSize(5, 6, 7)), *itkTransform);
}


GTEST_TEST(itkTransformixFilter, ITKEulerTransform2D)
{
  const auto itkTransform = itk::Euler2DTransform<double>::New();
  itkTransform->SetTranslation(MakeVector(1.0, -2.0));
  itkTransform->SetCenter(MakePoint(2.5, 3.0));
  itkTransform->SetAngle(M_PI_4);

  Expect_TransformixFilter_output_equals_ResampleImageFilter_output(
    *CreateImageFilledWithSequenceOfNaturalNumbers<float>(MakeSize(5, 6)), *itkTransform);
}


GTEST_TEST(itkTransformixFilter, ITKEulerTransform3D)
{
  const auto itkTransform = itk::Euler3DTransform<double>::New();
  itkTransform->SetTranslation(MakeVector(1.0, -2.0, 3.0));
  itkTransform->SetCenter(MakePoint(3.0, 2.0, 1.0));
  itkTransform->SetRotation(M_PI_2, M_PI_4, M_PI_4 / 2.0);

  Expect_TransformixFilter_output_equals_ResampleImageFilter_output(
    *CreateImageFilledWithSequenceOfNaturalNumbers<float>(MakeSize(5, 6, 7)), *itkTransform);
}


GTEST_TEST(itkTransformixFilter, ITKSimilarityTransform2D)
{
  const auto itkTransform = itk::Similarity2DTransform<double>::New();
  itkTransform->SetScale(0.75);
  itkTransform->SetTranslation(MakeVector(1.0, -2.0));
  itkTransform->SetCenter(MakePoint(2.5, 3.0));
  itkTransform->SetAngle(M_PI_4);

  Expect_TransformixFilter_output_equals_ResampleImageFilter_output(
    *CreateImageFilledWithSequenceOfNaturalNumbers<float>(MakeSize(5, 6)), *itkTransform);
}


GTEST_TEST(itkTransformixFilter, ITKSimilarityTransform3D)
{
  const auto itkTransform = itk::Similarity3DTransform<double>::New();
  itkTransform->SetScale(0.75);
  itkTransform->SetTranslation(MakeVector(1.0, -2.0, 3.0));
  itkTransform->SetCenter(MakePoint(3.0, 2.0, 1.0));
  itkTransform->SetRotation(itk::Vector<double, 3>(1.0), M_PI_4);

  Expect_TransformixFilter_output_equals_ResampleImageFilter_output(
    *CreateImageFilledWithSequenceOfNaturalNumbers<float>(MakeSize(5, 6, 7)), *itkTransform);
}


GTEST_TEST(itkTransformixFilter, ITKBSplineTransform2D)
{
  const auto itkTransform = itk::BSplineTransform<double, 2>::New();

  const auto imageSize = MakeSize(5, 6);

  // Note that this unit test would fail if TransformDomainPhysicalDimensions would not be set.
  itkTransform->SetTransformDomainPhysicalDimensions(ConvertToItkVector(imageSize));
  itkTransform->SetParameters(GeneratePseudoRandomParameters(itkTransform->GetParameters().size(), -1.0));

  Expect_TransformixFilter_output_equals_ResampleImageFilter_output(
    *CreateImageFilledWithSequenceOfNaturalNumbers<float>(imageSize), *itkTransform);
}


GTEST_TEST(itkTransformixFilter, ITKBSplineTransform3D)
{
  const auto itkTransform = itk::BSplineTransform<double, 3>::New();

  const auto imageSize = MakeSize(5, 6, 7);

  // Note that this unit test would fail if TransformDomainPhysicalDimensions would not be set.
  itkTransform->SetTransformDomainPhysicalDimensions(ConvertToItkVector(imageSize));
  itkTransform->SetParameters(GeneratePseudoRandomParameters(itkTransform->GetParameters().size(), -1.0));

  Expect_TransformixFilter_output_equals_ResampleImageFilter_output(
    *CreateImageFilledWithSequenceOfNaturalNumbers<float>(imageSize), *itkTransform);
}


GTEST_TEST(itkTransformixFilter, CombineTranslationAndDefaultTransform)
{
  const auto     imageSize = MakeSize(5, 6);
  const auto     inputImage = CreateImageFilledWithSequenceOfNaturalNumbers<float>(imageSize);
  constexpr auto dimension = decltype(imageSize)::Dimension;

  using ParametersValueType = double;

  // Create a translated image, which is the expected output image.
  const auto translationTransform = itk::TranslationTransform<ParametersValueType, dimension>::New();
  translationTransform->SetOffset(MakeVector(1, -2));
  const auto   resampleImageFilter = CreateResampleImageFilter(*inputImage, *translationTransform);
  const auto & expectedOutputImage = Deref(resampleImageFilter->GetOutput());

  const std::string initialTransformParametersFileName =
    GetDataDirectoryPath() + "/Translation(1,-2)/TransformParameters.txt";

  for (const auto defaultTransform : { CreateTransform<itk::AffineTransform<ParametersValueType, dimension>>(),
                                       CreateTransform<itk::BSplineTransform<ParametersValueType, dimension>>(),
                                       CreateTransform<itk::Euler2DTransform<ParametersValueType>>(),
                                       CreateTransform<itk::Similarity2DTransform<ParametersValueType>>(),
                                       CreateTransform<itk::TranslationTransform<ParametersValueType, dimension>>() })
  {
    const auto actualOutputImage =
      RetrieveOutputFromTransformixFilter(*inputImage, *defaultTransform, initialTransformParametersFileName);
    EXPECT_EQ(*actualOutputImage, expectedOutputImage);
  }

  const auto defaultTransform = itk::TranslationTransform<ParametersValueType, dimension>::New();

  for (const std::string transformParameterFileName :
       { "TransformParameters-link-to-ITK-tfm-file.txt",
         "TransformParameters-link-to-ITK-HDF5-file.txt",
         "TransformParameters-link-to-file-with-special-chars-in-path-name.txt" })
  {
    const auto actualOutputImage = RetrieveOutputFromTransformixFilter(
      *inputImage, *defaultTransform, GetDataDirectoryPath() + "/Translation(1,-2)/" + transformParameterFileName);
    EXPECT_EQ(*actualOutputImage, expectedOutputImage);
  }
}


GTEST_TEST(itkTransformixFilter, CombineTranslationAndInverseTranslation)
{
  const auto imageSize = MakeSize(5, 6);
  enum
  {
    dimension = decltype(imageSize)::Dimension
  };

  const auto inputImage = itk::Image<float, dimension>::New();
  inputImage->SetRegions(imageSize);
  inputImage->Allocate(true);
  FillImageRegion(*inputImage, { 2, 1 }, itk::Size<dimension>::Filled(2));

  using ParametersValueType = double;

  const std::string initialTransformParametersFileName =
    GetDataDirectoryPath() + "/Translation(1,-2)/TransformParameters.txt";

  const auto offset = MakeVector(1.0, -2.0);
  const auto inverseOffset = -offset;

  // Sanity check: when only an identity transform is applied, the transform from the TransformParameters.txt file
  // makes the output image unequal to the input image.
  const auto identityTransform = itk::TranslationTransform<ParametersValueType, dimension>::New();

  EXPECT_NE(*(RetrieveOutputFromTransformixFilter(*inputImage, *identityTransform, initialTransformParametersFileName)),
            *inputImage);

  // The inverse of the transform from the TransformParameters.txt file.
  const auto inverseTranslationTransform = [inverseOffset] {
    const auto transform = itk::TranslationTransform<ParametersValueType, dimension>::New();
    transform->SetOffset(inverseOffset);
    return transform;
  }();

  EXPECT_EQ(*(RetrieveOutputFromTransformixFilter(
              *inputImage, *inverseTranslationTransform, initialTransformParametersFileName)),
            *inputImage);

  for (const auto matrixOffsetTransform :
       { CreateMatrixOffsetTransform<itk::AffineTransform<ParametersValueType, dimension>>(),
         CreateMatrixOffsetTransform<itk::Euler2DTransform<ParametersValueType>>(),
         CreateMatrixOffsetTransform<itk::Similarity2DTransform<ParametersValueType>>() })
  {
    matrixOffsetTransform->SetOffset(inverseOffset);
    EXPECT_EQ(
      *(RetrieveOutputFromTransformixFilter(*inputImage, *matrixOffsetTransform, initialTransformParametersFileName)),
      *inputImage);
  }

  const auto inverseBSplineTransform = [imageSize, inverseOffset] {
    const auto transform = itk::BSplineTransform<ParametersValueType, dimension>::New();
    transform->SetTransformDomainPhysicalDimensions(ConvertToItkVector(imageSize));

    const auto                       numberOfParameters = transform->GetParameters().size();
    itk::OptimizerParameters<double> parameters(numberOfParameters, inverseOffset[1]);
    std::fill_n(parameters.begin(), numberOfParameters / 2, inverseOffset[0]);
    transform->SetParameters(parameters);
    return transform;
  }();

  const auto inverseBSplineOutputImage = RetrieveOutputFromTransformixFilter(
    *inputImage, *inverseBSplineTransform, initialTransformParametersFileName, "Add");
  ExpectAlmostEqualPixelValues(*inverseBSplineOutputImage, *inputImage, 1e-15);
}


GTEST_TEST(itkTransformixFilter, CombineTranslationAndScale)
{
  const auto imageSize = MakeSize(5, 6);
  enum
  {
    dimension = decltype(imageSize)::Dimension
  };
  const auto inputImage = CreateImageFilledWithSequenceOfNaturalNumbers<float>(imageSize);

  using ParametersValueType = double;

  const std::string initialTransformParametersFileName =
    GetDataDirectoryPath() + "/Translation(1,-2)/TransformParameters.txt";

  const auto scaleTransform = itk::AffineTransform<ParametersValueType, dimension>::New();
  scaleTransform->Scale(2.0);

  const auto translationTransform = itk::TranslationTransform<ParametersValueType, dimension>::New();
  translationTransform->SetOffset(MakeVector(1.0, -2.0));

  const auto transformixOutput =
    RetrieveOutputFromTransformixFilter(*inputImage, *scaleTransform, initialTransformParametersFileName);

  const auto translationAndScaleTransform = itk::CompositeTransform<double, 2>::New();
  translationAndScaleTransform->AddTransform(translationTransform);
  translationAndScaleTransform->AddTransform(scaleTransform);

  const auto scaleAndTranslationTransform = itk::CompositeTransform<double, 2>::New();
  scaleAndTranslationTransform->AddTransform(scaleTransform);
  scaleAndTranslationTransform->AddTransform(translationTransform);

  // Expect that Transformix output is unequal (!) to the output of the corresponding ITK translation + scale composite
  // transform.
  EXPECT_NE(DerefSmartPointer(transformixOutput),
            *(CreateResampleImageFilter(*inputImage, *translationAndScaleTransform)->GetOutput()));

  // Expect that Transformix output is equal to the output of the corresponding ITK scale + translation composite
  // transform (in that order). Note that itk::CompositeTransform processed the transforms in reverse order (compared to
  // elastix).
  EXPECT_EQ(DerefSmartPointer(transformixOutput),
            *(CreateResampleImageFilter(*inputImage, *scaleAndTranslationTransform)->GetOutput()));
}
