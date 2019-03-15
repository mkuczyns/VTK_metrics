/****************************************************************************
*   vtkMetrics.cxx
*
*   Created by:     Michael Kuczynski
*   Created on:     10/03/2019
*   Version:        1.0
*   Description:    
****************************************************************************/

#include "interactorStyler.hxx"

vtkStandardNewMacro(myInteractorStyler);

int main(int argc, char* argv[])
{
    /***************************************************************
    *   Check input arguements
    ***************************************************************/
    if ( argc != 2 )
    {
        std::cout << "ERROR: Incorrect program usage. \n";
        std::cout << "Correct usage: \n";
        std::cout << argv[0] << " <DICOM_Folder_Directory> \n";
        std::cout << "OR: \n";
        std::cout << argv[0] << " <NIfTI_File_Directory> \n";
        return EXIT_FAILURE;
    }

    // Create a variable for the input arguement (if valid type).
    std::string inputFile = "";

    // Verify that the provided input arguements are valid
    int valid = checkInputs( argv[1] );

    if ( valid == 0 || valid == 1 )
    {
        inputFile = argv[1];
    }
    else
    {
        return EXIT_FAILURE;
    }

    vtkSmartPointer<vtkDICOMImageReader> dicomReader;
    vtkSmartPointer<vtkNIFTIImageReader> niftiReader;

    vtkSmartPointer<vtkImageViewer2> imageViewer = vtkSmartPointer<vtkImageViewer2>::New();

    vtkSmartPointer<vtkImageData> volume        = vtkSmartPointer<vtkImageData>::New();
    vtkSmartPointer<vtkImageData> gaussianImage = vtkSmartPointer<vtkImageData>::New();
    vtkSmartPointer<vtkImageData> medianImage   = vtkSmartPointer<vtkImageData>::New();

    vtkSmartPointer<vtkImageData> imageSeg      = vtkSmartPointer<vtkImageData>::New();

    /***************************************************************
    *   Read in the provided image
    ***************************************************************/
    switch( valid )
    {
        case 0:     // Directory with DICOM series
        {
            // Read all files from the DICOM series in the specified directory.
            dicomReader = vtkSmartPointer<vtkDICOMImageReader>::New();
            dicomReader->SetDirectoryName( inputFile.c_str() );
            dicomReader->Update();

            volume->DeepCopy( dicomReader->GetOutput() );

            break;
        }

        case 1:     // NIfTI
        {
            // Create a reader and check if the input file is readable.
            niftiReader = vtkSmartPointer<vtkNIFTIImageReader>::New();

            if ( !( niftiReader->CanReadFile( inputFile.c_str() ) ) )
            {
                std::cout << "ERROR: vtk NIFTI image reader cannot read the provided file: " << inputFile << std::endl;
                return EXIT_FAILURE;
            }

            niftiReader->SetFileName( inputFile.c_str() );
            niftiReader->Update();

            volume->DeepCopy( niftiReader->GetOutput() );

            break;
        }

        default:
        {
            return EXIT_FAILURE;
        }
    }

    /***************************************************************
    *   Apply a Gaussian and median filter to the image
    ***************************************************************/
    std::cout << "\n**Filtering the input image** \n";

    std::cout << "Applying a Gaussian filter with std = 1.0...";
    vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
    gaussianSmoothFilter->SetInputData( volume );
    gaussianSmoothFilter->SetStandardDeviation( 1.0 );
    gaussianSmoothFilter->SetRadiusFactors( 1.0, 1.0, 1.0 );
    gaussianSmoothFilter->SetDimensionality( 3 );
    gaussianSmoothFilter->Update();

    gaussianImage->DeepCopy( gaussianSmoothFilter->GetOutput() );
    std::cout << "Done! \n";

    std::cout << "Applying a median filter with a kernel size of 3x3x3...";
    vtkSmartPointer<vtkImageMedian3D> medianFilter = vtkSmartPointer<vtkImageMedian3D>::New();
    medianFilter->SetInputData( volume );
    medianFilter->SetKernelSize( 3, 3, 3 );
    medianFilter->Update();

    medianImage->DeepCopy( medianFilter->GetOutput() );
    std::cout << "Done! \n";

    /***************************************************************
    *   Calculate the SNR of the images
    ***************************************************************/
    int lowerThreshold = 0, upperThreshold = 0;

    // Get the threshold and isovalue parameters from the user
    std::cout << "\n**Performing SNR calculation** \n";
    std::cout << "Please enter upper and lower threshold values: \n";
    std::cout << "Lower Threshold = ";
    std::cin >> lowerThreshold;
    std::cout << "Upper Threshold = ";
    std::cin >> upperThreshold;

    // Set the image dimensionality
    int dimX = volume->GetDimensions()[0] - 1;
    int dimY = volume->GetDimensions()[1] - 1;
    int dimZ = volume->GetDimensions()[2] - 1;

    int countForeground[3]   = {0, 0, 0};
    int countBackground[3]   = {0, 0, 0};
    double meanForeground[3] = {0, 0, 0};
    double meanBackground[3] = {0, 0, 0};
    double std[3]            = {0, 0, 0};
    double var[3]            = {0, 0, 0};

    // Calculate the mean of the background
    for (int z = 0; z < dimZ; z++)
    {
        for (int y = 0; y < dimY; y++)
        {
            for (int x = 0; x < dimX; x++)
            {  
                double originalVoxel = volume->GetScalarComponentAsDouble( x, y, z, 0 );
                double gaussianVoxel = gaussianImage->GetScalarComponentAsDouble( x, y, z, 0 );
                double medianVoxel   = medianImage->GetScalarComponentAsDouble( x, y, z, 0 );

                // Original image
                if ( originalVoxel > upperThreshold || originalVoxel < lowerThreshold )
                {
                    meanBackground[0] += originalVoxel;
                    countBackground[0]++;
                }

                // Gaussian filtered image
                if ( gaussianVoxel > upperThreshold || gaussianVoxel < lowerThreshold )
                {
                    meanBackground[1] += gaussianVoxel;
                    countBackground[1]++;
                }

                // Median filtered image
                if ( medianVoxel > upperThreshold || medianVoxel < lowerThreshold )
                {
                    meanBackground[2] += medianVoxel;
                    countBackground[2]++;
                }
            }
        }
    }

    meanBackground[0] /= countBackground[0];
    meanBackground[1] /= countBackground[1];
    meanBackground[2] /= countBackground[2];

    std::cout << "\n" << std::fixed << std::setprecision(4);
    std::cout << "Mean of the background of the original image is:          " << meanBackground[0] << "\n";
    std::cout << "Mean of the background of the Gaussian filtered image is: " << meanBackground[1] << "\n";
    std::cout << "Mean of the background of the median filtered image is:   " << meanBackground[2] << "\n";

    // Calculate the mean of the foreground and the variance of the background
    for (int z = 0; z < dimZ; z++)
    {
        for (int y = 0; y < dimY; y++)
        {
            for (int x = 0; x < dimX; x++)
            {  
                double originalVoxel = volume->GetScalarComponentAsDouble( x, y, z, 0 );
                double gaussianVoxel = gaussianImage->GetScalarComponentAsDouble( x, y, z, 0 );
                double medianVoxel   = medianImage->GetScalarComponentAsDouble( x, y, z, 0 );

                // Original image
                if ( originalVoxel <= upperThreshold && originalVoxel >= lowerThreshold )
                {
                    meanForeground[0] += originalVoxel;
                    countForeground[0]++;
                }
                else
                {
                    var[0] += pow( ( originalVoxel - meanBackground[0] ), 2 );
                }

                // Gaussian filtered image
                if ( gaussianVoxel <= upperThreshold && gaussianVoxel >= lowerThreshold )
                {
                    meanForeground[1] += gaussianVoxel;
                    countForeground[1]++;
                }
                else
                {
                    var[1] += pow( ( gaussianVoxel - meanBackground[1] ), 2 );
                }

                // Median filtered image
                if ( medianVoxel <= upperThreshold && medianVoxel >= lowerThreshold )
                {
                    meanForeground[2] += medianVoxel;
                    countForeground[2]++;
                }
                else
                {
                    var[2] += pow( ( medianVoxel - meanBackground[2] ), 2 );
                }
            }
        }
    }

    meanForeground[0] /= countForeground[0];
    meanForeground[1] /= countForeground[1];
    meanForeground[2] /= countForeground[2];

    std::cout << "\n";
    std::cout << "Mean of the foreground of the original image is:          " << meanForeground[0] << "\n";
    std::cout << "Mean of the foreground of the Gaussian filtered image is: " << meanForeground[1] << "\n";
    std::cout << "Mean of the foreground of the median filtered image is:   " << meanForeground[2] << "\n";

    var[0] /= countBackground[0];
    var[1] /= countBackground[1];
    var[2] /= countBackground[2];

    std[0] = sqrt( var[0] );
    std[1] = sqrt( var[1] );
    std[2] = sqrt( var[2] );

    std::cout << "\n";
    std::cout << "Standard deviation of the background of the original image is:          " << std[0] << "\n";
    std::cout << "Standard deviation of the background of the Gaussian filtered image is: " << std[1] << "\n";
    std::cout << "Standard deviation of the background of the median filtered image is:   " << std[2] << "\n";

    std::cout << "\n";
    std::cout << "SNR of the image of the original image is:          " << meanForeground[0]/std[0] << "\n";
    std::cout << "SNR of the image of the Gaussian filtered image is: " << meanForeground[1]/std[1] << "\n";
    std::cout << "SNR of the image of the median filtered image is:   " << meanForeground[2]/std[2] << "\n";

    /***************************************************************
    *   Segment the input image
    ***************************************************************/
    std::cout << "\n**Performing image segmentation** \n";
    std::cout << "Applying global threshold...";

    // Apply the global threshold
    vtkSmartPointer<vtkImageThreshold> globalThresh = vtkSmartPointer<vtkImageThreshold>::New();
    globalThresh->SetInputData( volume );
    globalThresh->ThresholdBetween( lowerThreshold, upperThreshold );
    globalThresh->ReplaceInOn();
    globalThresh->SetInValue( 1 );
    globalThresh->ReplaceOutOn();
    globalThresh->SetOutValue( 0 );
    globalThresh->SetOutputScalarTypeToFloat();
    globalThresh->Update();

    imageSeg->DeepCopy( globalThresh->GetOutput() );

    std::cout << "Done! \n";

    /***************************************************************
    *   Add mappers, actors, renderer, and setup the scene
    ***************************************************************/
    std::cout << "\n**Rendering the scene** \n";

    // Create a message with current slice out of total slices and current window level.
    // Use Time New Roman as the font, size 18.
    vtkSmartPointer<vtkTextProperty> sliceTextProperty = vtkSmartPointer<vtkTextProperty>::New();
    sliceTextProperty->SetFontFamilyToTimes();
    sliceTextProperty->SetFontSize( 18 );

    vtkSmartPointer<vtkTextProperty> WindowLevelTextProperty = vtkSmartPointer<vtkTextProperty>::New();
    WindowLevelTextProperty->SetFontFamilyToTimes();
    WindowLevelTextProperty->SetFontSize(18);

    vtkSmartPointer<vtkTextProperty> WindowTextProperty = vtkSmartPointer<vtkTextProperty>::New();
    WindowTextProperty->SetFontFamilyToTimes();
    WindowTextProperty->SetFontSize( 18 );

    // Create the text mappers for both slice number, window level, and window. Use a helper class "ImageMessage" for this.
    vtkSmartPointer<vtkTextMapper> sliceTextMapper = vtkSmartPointer<vtkTextMapper>::New();
    std::string sliceMessage = ImageMessage::sliceNumberFormat( imageViewer->GetSliceMin(), imageViewer->GetSliceMax() );
    sliceTextMapper->SetInput( sliceMessage.c_str() );
    sliceTextMapper->SetTextProperty( sliceTextProperty );

    imageViewer->GetWindowLevel()->SetLevel( 200 );
    vtkSmartPointer<vtkTextMapper> WindowLevelTextMapper = vtkSmartPointer<vtkTextMapper>::New();
    std::string windowLevelMessage = ImageMessage::windowLevelFormat( int( imageViewer->GetWindowLevel()->GetLevel() ) );
    WindowLevelTextMapper->SetInput( windowLevelMessage.c_str() );
    WindowLevelTextMapper->SetTextProperty( WindowLevelTextProperty );

    imageViewer->GetWindowLevel()->SetWindow( 200 );
    vtkSmartPointer<vtkTextMapper> WindowTextMapper = vtkSmartPointer<vtkTextMapper>::New();
    std::string windowMessage = ImageMessage::windowFormat( int( imageViewer->GetWindowLevel()->GetWindow() ) );
    WindowTextMapper->SetInput( windowMessage.c_str() );
    WindowTextMapper->SetTextProperty( WindowTextProperty );

    // Create the actors for each message.
    vtkSmartPointer<vtkActor2D> sliceTextActor = vtkSmartPointer<vtkActor2D>::New();
    sliceTextActor->SetMapper( sliceTextMapper );

    vtkSmartPointer<vtkActor2D> windowLevelTextActor = vtkSmartPointer<vtkActor2D>::New();
    windowLevelTextActor->SetMapper( WindowLevelTextMapper );

    vtkSmartPointer<vtkActor2D> windowTextActor = vtkSmartPointer<vtkActor2D>::New();
    windowTextActor->SetMapper( WindowTextMapper );

    // Specify the position of each message to be in the top left corner of the display.
    sliceTextActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
    sliceTextActor->GetPositionCoordinate()->SetValue( 0.05, 0.95);

    windowLevelTextActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
    windowLevelTextActor->GetPositionCoordinate()->SetValue( 0.05, 0.90);

    windowTextActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
    windowTextActor->GetPositionCoordinate()->SetValue( 0.05, 0.85);
    
    // Create a mask for the overlaid segmentation
    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetNumberOfTableValues( 2 );
    lookupTable->SetTableRange( 0.0, 1.0 );
    lookupTable->SetTableValue( 0.0, 0.0, 0.0, 0.0, 0.0 );
    lookupTable->SetTableValue( 0.0, 1.0, 0.0, 0.0, 1.0 );
    lookupTable->Build();

    vtkSmartPointer<vtkImageMapToColors> mapTransparency = vtkSmartPointer<vtkImageMapToColors>::New();
    mapTransparency->SetLookupTable( lookupTable );
    mapTransparency->PassAlphaToOutputOn();
    mapTransparency->SetInputData( imageSeg );

    // Create mappers for the original and segmented images
    vtkSmartPointer<vtkImageMapper> originalMapper = vtkSmartPointer<vtkImageMapper>::New();
    originalMapper->SetInputData( volume );
    originalMapper->SetZSlice( 1 );
    originalMapper->SetColorWindow( 1000 );
    originalMapper->SetColorLevel( 500 );

    vtkSmartPointer<vtkImageMapper> segMapper = vtkSmartPointer<vtkImageMapper>::New();
    segMapper->SetInputConnection( mapTransparency->GetOutputPort() );
    segMapper->SetZSlice( 1 );
    segMapper->SetColorWindow( 1 );
    segMapper->SetColorLevel( 1 );

    // Create actors for the original and segmented images
    vtkSmartPointer<vtkActor2D> imageActor = vtkSmartPointer<vtkActor2D>::New();
    imageActor->SetMapper( originalMapper );

    vtkSmartPointer<vtkActor2D> maskActor = vtkSmartPointer<vtkActor2D>::New();
    maskActor->SetMapper( segMapper );

    // Create the renderer and render window
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer( renderer );

    vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow( renderWindow );

    vtkSmartPointer<myInteractorStyler> interactorStyle = vtkSmartPointer<myInteractorStyler>::New();

    // Set the image viewer and status mapper to enable message updates when interacting with the image.
    interactorStyle->setImageViewer( originalMapper, segMapper, renderWindow );
    interactorStyle->setSliceStatusMapper( sliceTextMapper );
    interactorStyle->setWindowLevelStatusMapper( WindowLevelTextMapper );
    interactorStyle->setWindowStatusMapper( WindowTextMapper );

    interactor->SetInteractorStyle( interactorStyle );

    renderer->AddActor( imageActor );
    renderer->AddActor( maskActor );
    renderer->AddActor2D( sliceTextActor );
    renderer->AddActor2D( windowLevelTextActor );
    renderer->AddActor2D( windowTextActor );

    renderer->SetBackground( 0.0, 0.0, 0.0 );

    renderWindow->SetSize( imageViewer->GetRenderWindow()->GetScreenSize() );
    renderWindow->Render();

    std::cout << "Done! \n";

    renderer->ResetCamera();

    interactor->Initialize();
    interactor->Start();

    return EXIT_SUCCESS;
}