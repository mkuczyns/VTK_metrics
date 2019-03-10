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

    vtkSmartPointer<vtkImageData> volume = vtkSmartPointer<vtkImageData>::New();

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
    *   Apply a Gaussian filter to the image
    ***************************************************************/
    vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
    gaussianSmoothFilter->SetInputData( volume );
    gaussianSmoothFilter->SetStandardDeviation( 1.0 );
    gaussianSmoothFilter->SetRadiusFactors( 1.0, 1.0, 1.0 );
    gaussianSmoothFilter->SetDimensionality( 3 );
    gaussianSmoothFilter->Update();

    /***************************************************************
    *   Segment the input image
    ***************************************************************/
    int lowerThresh = 0, upperThresh = 0;
    double isoValue = 0;

    // Get the threshold and isovalue parameters from the user
    std::cout << "\n**Performing image segmentation** \n";
    std::cout << "Please enter upper and lower threshold values: \n";
    std::cout << "Lower Threshold = ";
    std::cin >> lowerThresh;
    std::cout << "Upper Threshold = ";
    std::cin >> upperThresh;

    std::cout << "Applying global threshold...";

    // Apply the global threshold
    vtkSmartPointer<vtkImageThreshold> globalThresh = vtkSmartPointer<vtkImageThreshold>::New();
    globalThresh->SetInputData( volume );
    globalThresh->ThresholdBetween( lowerThresh, upperThresh );
    globalThresh->ReplaceInOn();
    globalThresh->SetInValue( 1 );
    globalThresh->ReplaceOutOn();
    globalThresh->SetOutValue( 0 );
    globalThresh->SetOutputScalarTypeToFloat();
    globalThresh->Update();

    std::cout << "Done! \n";

    /***************************************************************
    *   Use the Marching cubes algorithm to generate the surface
    ***************************************************************/
    // std::cout << "\n**Generating surface using Marching cubes** \n";

    // std::cout << "Please enter the desired isovalue for the Marching Cubes algortihm (between 0-1): ";
    // std::cin >> isoValue;

    // std::cout << "Starting surface rendering...";

    // vtkSmartPointer<vtkMarchingCubes> surface = vtkSmartPointer<vtkMarchingCubes>::New();
    // surface->SetInputData( globalThresh->GetOutput() );
    // surface->ComputeNormalsOn();
    // surface->SetValue( 0, isoValue );

    // // Reduce the number of triangles by half to speed up computation
    // vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
    // decimate->SetInputConnection( surface->GetOutputPort() );
    // decimate->SetTargetReduction( 0.5 );
    // decimate->Update();

    // std::cout << "Done! \n";

    vtkSmartPointer<vtkImageMask> maskFilter = vtkSmartPointer<vtkImageMask>::New();
    maskFilter->SetInputData( 0, volume );
    maskFilter->SetInputConnection( 1, globalThresh->GetOutputPort() );
    maskFilter->SetMaskedOutputValue( 0, 1, 0 );
    maskFilter->Update();

    /***************************************************************
    *   Add mappers, actors, renderer, and setup the scene
    ***************************************************************/
    std::cout << "\n**Rendering the scene** \n";
    
    // vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    // mapper->SetInputData( decimate->GetOutput() );

    vtkSmartPointer<vtkImageMapper> maskedMapper = vtkSmartPointer<vtkImageMapper>::New();
    maskedMapper->SetInputData( maskFilter->GetOutput() );

    // vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    // actor->SetMapper( mapper );
    // actor->GetProperty()->SetColor( 0, 1, 0 );
    // actor->GetProperty()->SetOpacity( 0.5 );
    // actor->GetProperty()->SetPointSize( 4 );
    // actor->GetMapper()->ScalarVisibilityOff();

    vtkSmartPointer<vtkImageActor> maskedActor = vtkSmartPointer<vtkImageActor>::New();
    maskedActor->GetMapper()->SetInputConnection( maskFilter->GetOutputPort() );
    // maskedActor->SetMapper( maskedMapper );

    // Create the renderer and render window
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer( renderer );

    vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow( renderWindow );

    renderer->AddActor( maskedActor );

    renderer->SetBackground( 0.0, 0.0, 0.0 );

    renderWindow->SetSize( 800, 800 );
    renderWindow->Render();

    std::cout << "Done! \n";

    renderer->ResetCamera();

    interactor->Initialize();
    interactor->Start();

    return EXIT_SUCCESS;
}