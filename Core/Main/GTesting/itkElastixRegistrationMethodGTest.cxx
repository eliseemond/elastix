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


// First include the header file to be tested:
#include <itkElastixRegistrationMethod.h>

#include "elxCoreMainGTestUtilities.h"
#include "GTesting/elxGTestUtilities.h"

// ITK header file:
#include <itkImage.h>
#include <itkIndexRange.h>

// GoogleTest header file:
#include <gtest/gtest.h>

#include <algorithm> // For transform
#include <map>
#include <string>
#include <utility> // For pair


// Using-declarations:
using elx::CoreMainGTestUtilities::CheckNew;
using elx::CoreMainGTestUtilities::ConvertToOffset;
using elx::CoreMainGTestUtilities::CreateParameterObject;
using elx::CoreMainGTestUtilities::Deref;
using elx::CoreMainGTestUtilities::FillImageRegion;
using elx::CoreMainGTestUtilities::Front;
using elx::CoreMainGTestUtilities::GetDataDirectoryPath;
using elx::CoreMainGTestUtilities::GetTransformParametersFromFilter;
using elx::GTestUtilities::MakePoint;


// Tests registering two small (5x6) binary images, which are translated with respect to each other.
GTEST_TEST(itkElastixRegistrationMethod, Translation)
{
  constexpr auto ImageDimension = 2U;
  using ImageType = itk::Image<float, ImageDimension>;
  using SizeType = itk::Size<ImageDimension>;
  using IndexType = itk::Index<ImageDimension>;
  using OffsetType = itk::Offset<ImageDimension>;

  const OffsetType translationOffset{ { 1, -2 } };
  const auto       regionSize = SizeType::Filled(2);
  const SizeType   imageSize{ { 5, 6 } };
  const IndexType  fixedImageRegionIndex{ { 1, 3 } };

  const auto fixedImage = ImageType::New();
  fixedImage->SetRegions(imageSize);
  fixedImage->Allocate(true);
  FillImageRegion(*fixedImage, fixedImageRegionIndex, regionSize);

  const auto movingImage = ImageType::New();
  movingImage->SetRegions(imageSize);
  movingImage->Allocate(true);
  FillImageRegion(*movingImage, fixedImageRegionIndex + translationOffset, regionSize);

  const auto filter = CheckNew<itk::ElastixRegistrationMethod<ImageType, ImageType>>();

  filter->SetFixedImage(fixedImage);
  filter->SetMovingImage(movingImage);
  filter->SetParameterObject(CreateParameterObject({ // Parameters in alphabetic order:
                                                     { "ImageSampler", "Full" },
                                                     { "MaximumNumberOfIterations", "2" },
                                                     { "Metric", "AdvancedNormalizedCorrelation" },
                                                     { "Optimizer", "AdaptiveStochasticGradientDescent" },
                                                     { "Transform", "TranslationTransform" } }));
  filter->Update();

  const auto transformParameters = GetTransformParametersFromFilter(*filter);
  EXPECT_EQ(ConvertToOffset<ImageDimension>(transformParameters), translationOffset);
}


// Tests "MaximumNumberOfIterations" value "0"
GTEST_TEST(itkElastixRegistrationMethod, MaximumNumberOfIterationsZero)
{
  constexpr auto ImageDimension = 2U;
  using ImageType = itk::Image<float, ImageDimension>;
  using SizeType = itk::Size<ImageDimension>;
  using IndexType = itk::Index<ImageDimension>;
  using OffsetType = itk::Offset<ImageDimension>;

  const OffsetType translationOffset{ { 1, -2 } };
  const auto       regionSize = SizeType::Filled(2);
  const SizeType   imageSize{ { 5, 6 } };
  const IndexType  fixedImageRegionIndex{ { 1, 3 } };

  const auto fixedImage = ImageType::New();
  fixedImage->SetRegions(imageSize);
  fixedImage->Allocate(true);
  FillImageRegion(*fixedImage, fixedImageRegionIndex, regionSize);

  const auto movingImage = ImageType::New();
  movingImage->SetRegions(imageSize);
  movingImage->Allocate(true);
  FillImageRegion(*movingImage, fixedImageRegionIndex + translationOffset, regionSize);

  for (const auto optimizer :
       { "AdaptiveStochasticGradientDescent", "FiniteDifferenceGradientDescent", "StandardGradientDescent" })
  {
    const auto filter = CheckNew<itk::ElastixRegistrationMethod<ImageType, ImageType>>();

    filter->SetFixedImage(fixedImage);
    filter->SetMovingImage(movingImage);
    filter->SetParameterObject(CreateParameterObject({ // Parameters in alphabetic order:
                                                       { "ImageSampler", "Full" },
                                                       { "MaximumNumberOfIterations", "0" },
                                                       { "Metric", "AdvancedNormalizedCorrelation" },
                                                       { "Optimizer", optimizer },
                                                       { "Transform", "TranslationTransform" } }));
    filter->Update();

    const auto transformParameters = GetTransformParametersFromFilter(*filter);

    for (const auto & transformParameter : transformParameters)
    {
      EXPECT_EQ(transformParameter, 0.0);
    }
  }
}


// Tests "AutomaticTransformInitializationMethod" "CenterOfGravity".
GTEST_TEST(itkElastixRegistrationMethod, AutomaticTransformInitializationCenterOfGravity)
{
  constexpr auto ImageDimension = 2U;
  using ImageType = itk::Image<float, ImageDimension>;
  using SizeType = itk::Size<ImageDimension>;
  using IndexType = itk::Index<ImageDimension>;
  using OffsetType = itk::Offset<ImageDimension>;

  const OffsetType translationOffset{ { 1, -2 } };
  const auto       regionSize = SizeType::Filled(2);
  const SizeType   imageSize{ { 5, 6 } };
  const IndexType  fixedImageRegionIndex{ { 1, 3 } };

  const auto fixedImage = ImageType::New();
  fixedImage->SetRegions(imageSize);
  fixedImage->Allocate(true);
  FillImageRegion(*fixedImage, fixedImageRegionIndex, regionSize);

  const auto movingImage = ImageType::New();
  movingImage->SetRegions(imageSize);
  movingImage->Allocate(true);
  FillImageRegion(*movingImage, fixedImageRegionIndex + translationOffset, regionSize);

  for (const bool automaticTransformInitialization : { false, true })
  {
    const auto filter = CheckNew<itk::ElastixRegistrationMethod<ImageType, ImageType>>();

    filter->SetFixedImage(fixedImage);
    filter->SetMovingImage(movingImage);
    filter->SetParameterObject(CreateParameterObject(
      { // Parameters in alphabetic order:
        { "AutomaticTransformInitialization", automaticTransformInitialization ? "true" : "false" },
        { "AutomaticTransformInitializationMethod", "CenterOfGravity" },
        { "ImageSampler", "Full" },
        { "MaximumNumberOfIterations", "0" },
        { "Metric", "AdvancedNormalizedCorrelation" },
        { "Optimizer", "AdaptiveStochasticGradientDescent" },
        { "Transform", "TranslationTransform" } }));
    filter->Update();

    const auto transformParameters = GetTransformParametersFromFilter(*filter);
    const auto estimatedOffset = ConvertToOffset<ImageDimension>(transformParameters);
    EXPECT_EQ(estimatedOffset == translationOffset, automaticTransformInitialization);
  }
}


// Tests registering two images, having "WriteResultImage" set.
GTEST_TEST(itkElastixRegistrationMethod, WriteResultImage)
{
  constexpr auto ImageDimension = 2U;
  using ImageType = itk::Image<float, ImageDimension>;
  using SizeType = itk::Size<ImageDimension>;
  using IndexType = itk::Index<ImageDimension>;
  using OffsetType = itk::Offset<ImageDimension>;

  const OffsetType translationOffset{ { 1, -2 } };
  const auto       regionSize = SizeType::Filled(2);
  const SizeType   imageSize{ { 5, 6 } };
  const IndexType  fixedImageRegionIndex{ { 1, 3 } };

  const auto fixedImage = ImageType::New();
  fixedImage->SetRegions(imageSize);
  fixedImage->Allocate(true);
  FillImageRegion(*fixedImage, fixedImageRegionIndex, regionSize);

  const auto movingImage = ImageType::New();
  movingImage->SetRegions(imageSize);
  movingImage->Allocate(true);
  FillImageRegion(*movingImage, fixedImageRegionIndex + translationOffset, regionSize);

  for (const bool writeResultImage : { true, false })
  {
    const auto filter = CheckNew<itk::ElastixRegistrationMethod<ImageType, ImageType>>();

    filter->SetFixedImage(fixedImage);
    filter->SetMovingImage(movingImage);
    filter->SetParameterObject(
      CreateParameterObject({ // Parameters in alphabetic order:
                              { "ImageSampler", "Full" },
                              { "MaximumNumberOfIterations", "2" },
                              { "Metric", "AdvancedNormalizedCorrelation" },
                              { "Optimizer", "AdaptiveStochasticGradientDescent" },
                              { "Transform", "TranslationTransform" },
                              { "WriteResultImage", (writeResultImage ? "true" : "false") } }));
    filter->Update();

    const auto &       output = Deref(filter->GetOutput());
    const auto &       outputImageSize = output.GetBufferedRegion().GetSize();
    const auto * const outputBufferPointer = output.GetBufferPointer();

    if (writeResultImage)
    {
      EXPECT_EQ(outputImageSize, imageSize);
      ASSERT_NE(outputBufferPointer, nullptr);

      // When "WriteResultImage" is true, expect an output image that is very much like the fixed image.
      for (const auto index : itk::ZeroBasedIndexRange<ImageDimension>(imageSize))
      {
        EXPECT_EQ(std::round(output.GetPixel(index)), std::round(fixedImage->GetPixel(index)));
      }
    }
    else
    {
      // When "WriteResultImage" is false, expect an empty output image.
      EXPECT_EQ(outputImageSize, ImageType::SizeType());
      EXPECT_EQ(outputBufferPointer, nullptr);
    }

    const auto transformParameters = GetTransformParametersFromFilter(*filter);
    EXPECT_EQ(ConvertToOffset<ImageDimension>(transformParameters), translationOffset);
  }
}


// Tests that the origin of the output image is equal to the origin of the fixed image (by default).
GTEST_TEST(itkElastixRegistrationMethod, OutputHasSameOriginAsFixedImage)
{
  constexpr auto ImageDimension = 2U;
  using ImageType = itk::Image<float, ImageDimension>;
  using SizeType = itk::Size<ImageDimension>;
  using IndexType = itk::Index<ImageDimension>;
  using OffsetType = itk::Offset<ImageDimension>;

  const OffsetType translationOffset{ { 1, -2 } };
  const auto       regionSize = SizeType::Filled(2);
  const SizeType   imageSize{ { 12, 16 } };
  const IndexType  fixedImageRegionIndex{ { 3, 9 } };

  const auto fixedImage = ImageType::New();
  fixedImage->SetRegions(imageSize);
  fixedImage->Allocate(true);
  FillImageRegion(*fixedImage, fixedImageRegionIndex, regionSize);

  const auto movingImage = ImageType::New();
  movingImage->SetRegions(imageSize);
  movingImage->Allocate(true);
  FillImageRegion(*movingImage, fixedImageRegionIndex + translationOffset, regionSize);

  for (const auto fixedImageOrigin : { MakePoint(-1.0, -2.0), ImageType::PointType(), MakePoint(0.25, 0.75) })
  {
    fixedImage->SetOrigin(fixedImageOrigin);

    for (const auto movingImageOrigin : { MakePoint(-1.0, -2.0), ImageType::PointType(), MakePoint(0.25, 0.75) })
    {
      movingImage->SetOrigin(movingImageOrigin);

      const auto filter = CheckNew<itk::ElastixRegistrationMethod<ImageType, ImageType>>();

      filter->SetFixedImage(fixedImage);
      filter->SetMovingImage(movingImage);
      filter->SetParameterObject(CreateParameterObject({ // Parameters in alphabetic order:
                                                         { "ImageSampler", "Full" },
                                                         { "MaximumNumberOfIterations", "2" },
                                                         { "Metric", "AdvancedNormalizedCorrelation" },
                                                         { "Optimizer", "AdaptiveStochasticGradientDescent" },
                                                         { "Transform", "TranslationTransform" } }));
      filter->Update();

      const auto & output = Deref(filter->GetOutput());

      // The most essential check of this test.
      EXPECT_EQ(output.GetOrigin(), fixedImageOrigin);

      ASSERT_EQ(output.GetBufferedRegion().GetSize(), imageSize);
      ASSERT_NE(output.GetBufferPointer(), nullptr);

      // Expect an output image that is very much like the fixed image.
      for (const auto & index : itk::ZeroBasedIndexRange<ImageDimension>(imageSize))
      {
        EXPECT_EQ(std::round(output.GetPixel(index)), std::round(fixedImage->GetPixel(index)));
      }

      const auto transformParameters = GetTransformParametersFromFilter(*filter);

      ASSERT_EQ(transformParameters.size(), ImageDimension);

      for (std::size_t i{}; i < ImageDimension; ++i)
      {
        EXPECT_EQ(std::round(transformParameters[i] + fixedImageOrigin[i] - movingImageOrigin[i]),
                  translationOffset[i]);
      }
    }
  }
}


GTEST_TEST(itkElastixRegistrationMethod, InitialTransformParameterFile)
{
  constexpr auto ImageDimension = 2U;
  using ImageType = itk::Image<float, ImageDimension>;
  using SizeType = itk::Size<ImageDimension>;
  using IndexType = itk::Index<ImageDimension>;
  using OffsetType = itk::Offset<ImageDimension>;

  const OffsetType initialTranslation{ { 1, -2 } };
  const auto       regionSize = SizeType::Filled(2);
  const SizeType   imageSize{ { 5, 6 } };
  const IndexType  fixedImageRegionIndex{ { 1, 3 } };

  const auto fixedImage = ImageType::New();
  fixedImage->SetRegions(imageSize);
  fixedImage->Allocate(true);
  FillImageRegion(*fixedImage, fixedImageRegionIndex, regionSize);

  const auto movingImage = ImageType::New();
  movingImage->SetRegions(imageSize);
  movingImage->Allocate();

  const auto filter = CheckNew<itk::ElastixRegistrationMethod<ImageType, ImageType>>();

  filter->SetFixedImage(fixedImage);
  filter->SetInitialTransformParameterFileName(GetDataDirectoryPath() + "/Translation(1,-2)/TransformParameters.txt");

  filter->SetParameterObject(CreateParameterObject({ // Parameters in alphabetic order:
                                                     { "ImageSampler", "Full" },
                                                     { "MaximumNumberOfIterations", "2" },
                                                     { "Metric", "AdvancedNormalizedCorrelation" },
                                                     { "Optimizer", "AdaptiveStochasticGradientDescent" },
                                                     { "Transform", "TranslationTransform" } }));

  const auto toOffset = [](const IndexType & index) { return index - IndexType(); };

  for (const auto index :
       itk::ImageRegionIndexRange<ImageDimension>(itk::ImageRegion<ImageDimension>({ 0, -2 }, { 2, 3 })))
  {
    movingImage->FillBuffer(0);
    FillImageRegion(*movingImage, fixedImageRegionIndex + toOffset(index), regionSize);
    filter->SetMovingImage(movingImage);
    filter->Update();

    const auto transformParameters = GetTransformParametersFromFilter(*filter);
    ASSERT_EQ(transformParameters.size(), ImageDimension);

    for (unsigned i{}; i < ImageDimension; ++i)
    {
      EXPECT_EQ(std::round(transformParameters[i]), index[i] - initialTranslation[i]);
    }
  }
}


GTEST_TEST(itkElastixRegistrationMethod, InitialTransformParameterFileLinkToTransformFile)
{
  constexpr auto ImageDimension = 2U;
  using ImageType = itk::Image<float, ImageDimension>;
  using SizeType = itk::Size<ImageDimension>;
  using IndexType = itk::Index<ImageDimension>;
  using OffsetType = itk::Offset<ImageDimension>;

  using RegistrationMethodType = itk::ElastixRegistrationMethod<ImageType, ImageType>;

  const OffsetType initialTranslation{ { 1, -2 } };
  const auto       regionSize = SizeType::Filled(2);
  const SizeType   imageSize{ { 5, 6 } };
  const IndexType  fixedImageRegionIndex{ { 1, 3 } };

  const auto fixedImage = ImageType::New();
  fixedImage->SetRegions(imageSize);
  fixedImage->Allocate(true);
  FillImageRegion(*fixedImage, fixedImageRegionIndex, regionSize);

  const auto movingImage = ImageType::New();
  movingImage->SetRegions(imageSize);
  movingImage->Allocate();

  const auto toOffset = [](const IndexType & index) { return index - IndexType(); };

  const auto createFilter = [fixedImage](const std::string & initialTransformParameterFileName) {
    const auto filter = CheckNew<RegistrationMethodType>();
    filter->SetFixedImage(fixedImage);
    filter->SetInitialTransformParameterFileName(GetDataDirectoryPath() + "/Translation(1,-2)/" +
                                                 initialTransformParameterFileName);
    filter->SetParameterObject(CreateParameterObject({ // Parameters in alphabetic order:
                                                       { "ImageSampler", "Full" },
                                                       { "MaximumNumberOfIterations", "2" },
                                                       { "Metric", "AdvancedNormalizedCorrelation" },
                                                       { "Optimizer", "AdaptiveStochasticGradientDescent" },
                                                       { "Transform", "TranslationTransform" } }));
    return filter;
  };

  const auto filter1 = createFilter("TransformParameters.txt");

  for (const auto transformParameterFileName :
       { "TransformParameters-link-to-ITK-tfm-file.txt",
         "TransformParameters-link-to-ITK-HDF5-file.txt",
         "TransformParameters-link-to-file-with-special-chars-in-path-name.txt" })
  {
    const auto filter2 = createFilter(transformParameterFileName);

    for (const auto index :
         itk::ImageRegionIndexRange<ImageDimension>(itk::ImageRegion<ImageDimension>({ 0, -2 }, { 2, 3 })))
    {
      movingImage->FillBuffer(0);
      FillImageRegion(*movingImage, fixedImageRegionIndex + toOffset(index), regionSize);

      const auto updateAndRetrieveTransformParameterMap = [movingImage](RegistrationMethodType & filter) {
        filter.SetMovingImage(movingImage);
        filter.Update();
        const elx::ParameterObject & transformParameterObject = Deref(filter.GetTransformParameterObject());
        const auto &                 transformParameterMaps = transformParameterObject.GetParameterMap();
        EXPECT_EQ(transformParameterMaps.size(), 1);
        return Front(transformParameterMaps);
      };

      const auto transformParameterMap1 = updateAndRetrieveTransformParameterMap(*filter1);
      const auto transformParameterMap2 = updateAndRetrieveTransformParameterMap(*filter2);

      ASSERT_EQ(transformParameterMap1.size(), transformParameterMap2.size());
      for (const auto & transformParameter : transformParameterMap1)
      {
        const auto found = transformParameterMap2.find(transformParameter.first);
        ASSERT_NE(found, transformParameterMap2.end());

        if (transformParameter.first == "InitialTransformParametersFileName")
        {
          ASSERT_NE(*found, transformParameter);
        }
        else
        {
          ASSERT_EQ(*found, transformParameter);
        }
      }
    }
  }
}
