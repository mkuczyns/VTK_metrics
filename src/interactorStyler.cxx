/****************************************************************************
*   interactorStyler.cxx
*
*   Created by:     Michael Kuczynski
*   Created on:     19/01/2019
*   Description:    Custom interactor styling implementation.
****************************************************************************/

#include "interactorStyler.hxx"

void myInteractorStyler::setImageViewer( vtkImageMapper* originalMapper, vtkImageMapper* segMapper, vtkRenderWindow* renderWindow )
{
    _RenderWindow = renderWindow;
    _ImageMapper = originalMapper;
    _SegMapper = segMapper;
    minSlice = originalMapper->GetWholeZMin();
    maxSlice = originalMapper->GetWholeZMax();
    windowLevelOG = originalMapper->GetColorLevel();
    windowLevelSEG = segMapper->GetColorLevel();
    windowOG = originalMapper->GetColorWindow();
    windowSEG = segMapper->GetColorWindow();
    
    // Start current slice at 0
    slice = minSlice;
}

void myInteractorStyler::setSliceStatusMapper( vtkTextMapper* statusMapper ) 
{
    _SliceStatusMapper = statusMapper;
}

void myInteractorStyler::setWindowLevelStatusMapper( vtkTextMapper* statusMapper ) 
{
    _WindowLevelStatusMapper = statusMapper;
}

void myInteractorStyler::setWindowStatusMapper( vtkTextMapper* statusMapper ) 
{
    _WindowStatusMapper = statusMapper;
}

void myInteractorStyler::moveSliceForward() 
{
    if ( slice < maxSlice ) 
    {
        slice += 1;

        _ImageMapper->SetZSlice( slice );
        _SegMapper->SetZSlice( slice );

        // Create the message to be displayed.
        std::string msg = ImageMessage::sliceNumberFormat( slice, maxSlice );

        // Update the mapper and render.
        _SliceStatusMapper->SetInput( msg.c_str() );
        _RenderWindow->Render();
    }
}

void myInteractorStyler::moveSliceBackward() 
{
    if ( slice > minSlice ) 
    {
        slice -= 1;

        _ImageMapper->SetZSlice( slice );
        _SegMapper->SetZSlice( slice );

        // Create the message to be displayed.
        std::string msg = ImageMessage::sliceNumberFormat( slice, maxSlice );

        // Update the mapper and render.
        _SliceStatusMapper->SetInput( msg.c_str() );
        _RenderWindow->Render();
    }
}

void myInteractorStyler::moveWindowLevelForward() 
{
    windowLevelOG += 10;
    windowLevelSEG += 10;

    _ImageMapper->SetColorLevel( windowLevelOG );
    _SegMapper->SetColorLevel( windowLevelSEG );

    // Create the message to be displayed.
    std::string msg = ImageMessage::windowLevelFormat( int( windowLevelOG ) );

    // Update the mapper and render.
    _WindowLevelStatusMapper->SetInput( msg.c_str() );
    _RenderWindow->Render();
}

void myInteractorStyler::moveWindowLevelBackward() 
{
    windowLevelOG -= 10;
    windowLevelSEG -= 10;

    _ImageMapper->SetColorLevel( windowLevelOG );
    _SegMapper->SetColorLevel( windowLevelSEG );

    // Create the message to be displayed.
    std::string msg = ImageMessage::windowLevelFormat( int( windowLevelOG ) );

    // Update the mapper and render.
    _WindowLevelStatusMapper->SetInput( msg.c_str() );
    _RenderWindow->Render();
}

void myInteractorStyler::moveWindowForward()
{
    windowOG += 10;
    windowSEG += 10;

    _ImageMapper->SetColorWindow( windowOG );
    _SegMapper->SetColorWindow( windowSEG );

    // Create the message to be displayed.
    std::string msg = ImageMessage::windowFormat( int( windowOG ) );

    // Update the mapper and render.
    _WindowStatusMapper->SetInput( msg.c_str() );
    _RenderWindow->Render();
}

void myInteractorStyler::moveWindowBackward()
{
    windowOG -= 10;
    windowSEG -= 10;

    _ImageMapper->SetColorWindow( windowOG );
    _SegMapper->SetColorWindow( windowSEG );

    // Create the message to be displayed.
    std::string msg = ImageMessage::windowFormat( int( windowOG ) );

    // Update the mapper and render.
    _WindowStatusMapper->SetInput( msg.c_str() );
    _RenderWindow->Render();  
}