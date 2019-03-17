# VTK_metrics

# Features
1. Reads and displays 3D renderings of DICOM or NIfTI images.
2. Applies a Gaussian and median filters for smoothing.
3. Calculates the signal to noise ratio (SNR) for the original image and both filtered images.
4. A global threshold can be set by the user. This segmentation is then overlaid on the original image.
5. Scroll through slices with the UP/DOWN arrow keys or the mouse wheel. 
6. Zoom in and out by clicking and dragging the right mouse buttom.

# How to Run
1. Create a folder for the build (e.g. bin, build, etc.)
2. Build with CMake and your favorite compiler.
3. Run the executable that is generated in the bin\Debug folder from the command line
   
    ```
    vtkMetrics.exe <PATH_TO_DICOM_FOLDER>
    ```
    OR:

    ```
    vtkMetrics.exe <NIfTI_IMAGE_FILE>.nii
    ```